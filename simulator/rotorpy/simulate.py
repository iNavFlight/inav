import time
from enum import Enum
import copy
import numpy as np
import roma
import torch
from numpy.linalg import norm
from scipy.spatial.transform import Rotation
from time import perf_counter
 
import rotorpy.wind
from rotorpy.controllers.quadrotor_control import BatchedSE3Control
from rotorpy.vehicles.multirotor import BatchedMultirotor


class ExitStatus(Enum):
    """ Exit status values indicate the reason for simulation termination. """
    COMPLETE     = 'Success: End reached.'
    TIMEOUT      = 'Timeout: Simulation end time reached.'
    INF_VALUE    = 'Failure: Your controller returned inf motor speeds.'
    NAN_VALUE    = 'Failure: Your controller returned nan motor speeds.'
    OVER_SPEED   = 'Failure: Your quadrotor is out of control; it is going faster than 100 m/s. The Guinness World Speed Record is 73 m/s.'
    OVER_SPIN    = 'Failure: Your quadrotor is out of control; it is spinning faster than 100 rad/s. The onboard IMU can only measure up to 52 rad/s (3000 deg/s).'
    FLY_AWAY     = 'Failure: Your quadrotor is out of control; it flew away with a position error greater than 20 meters.'
    COLLISION    = 'Failure: Your quadrotor collided with an object.'

def simulate(world, initial_state, vehicle, controller, trajectory, wind_profile, imu, mocap, estimator, t_final, t_step, safety_margin, use_mocap, terminate=None, print_fps=False):
    """
    Perform a vehicle simulation and return the numerical results.

    Inputs:
        world, a class representing the world it is flying in, including objects and world bounds. 
        initial_state, a dict defining the vehicle initial conditions with appropriate keys
        vehicle, Vehicle object containing the dynamics
        controller, Controller object containing the controller
        trajectory, Trajectory object containing the trajectory to follow
        wind_profile, Wind Profile object containing the wind generator. 
        t_final, maximum duration of simulation, s
        t_step, the time between each step in the simulator, s
        safety_margin, the radius of the ball surrounding the vehicle position to determine if a collision occurs
        imu, IMU object that generates accelerometer and gyroscope readings from the vehicle state
        terminate, None, False, or a function of time and state that returns
            ExitStatus. If None (default), terminate when hover is reached at
            the location of trajectory with t=inf. If False, never terminate
            before timeout or error. If a function, terminate when returns not
            None.
        mocap, a MotionCapture object that provides noisy measurements of pose and twist with artifacts. 
        use_mocap, a boolean to determine in noisy measurements from mocap should be used for quadrotor control
        estimator, an estimator object that provides estimates of a portion or all of the vehicle state.

    Outputs:
        time, seconds, shape=(N,)
        state, a dict describing the state history with keys
            x, position, m, shape=(N,3)
            v, linear velocity, m/s, shape=(N,3)
            q, quaternion [i,j,k,w], shape=(N,4)
            w, angular velocity, rad/s, shape=(N,3)
            rotor_speeds, motor speeds, rad/s, shape=(N,n) where n is the number of rotors
            wind, wind velocity, m/s, shape=(N,3)
        control, a dict describing the command input history with keys
            cmd_motor_speeds, motor speeds, rad/s, shape=(N,4)
            cmd_q, commanded orientation (not used by simulator), quaternion [i,j,k,w], shape=(N,4)
            cmd_w, commanded angular velocity (not used by simulator), rad/s, shape=(N,3)
        flat, a dict describing the desired flat outputs from the trajectory with keys
            x,        position, m
            x_dot,    velocity, m/s
            x_ddot,   acceleration, m/s**2
            x_dddot,  jerk, m/s**3
            x_ddddot, snap, m/s**4
            yaw,      yaw angle, rad
            yaw_dot,  yaw rate, rad/s
        imu_measurements, a dict containing the biased and noisy measurements from an accelerometer and gyroscope
            accel,  accelerometer, m/s**2
            gyro,   gyroscope, rad/s
        imu_gt, a dict containing the ground truth (no noise, no bias) measurements from an accelerometer and gyroscope
            accel,  accelerometer, m/s**2
            gyro,   gyroscope, rad/s
        mocap_measurements, a dict containing noisy measurements of pose and twist for the vehicle. 
            x, position (inertial)
            v, velocity (inertial)
            q, orientation of body w.r.t. inertial frame.
            w, body rates in the body frame. 
        exit_status, an ExitStatus enum indicating the reason for termination.
    """

    # Coerce entries of initial state into numpy arrays, if they are not already.
    initial_state = {k: np.array(v) for k, v in initial_state.items()}

    if terminate is None:    # Default exit. Terminate at final position of trajectory.
        normal_exit = traj_end_exit(initial_state, trajectory, using_vio = False)
    elif terminate is False: # Never exit before timeout.
        normal_exit = lambda t, s: None
    else:                    # Custom exit.
        normal_exit = terminate

    time    = [0]
    state   = [copy.deepcopy(initial_state)]
    state[0]['wind'] = wind_profile.update(0, state[0]['x'])   # TODO: move this line elsewhere so that other objects that don't have wind as a state can work here. 
    imu_measurements = []
    mocap_measurements = []
    imu_gt = []
    state_estimate = []
    flat    = [trajectory.update(time[-1])]
    mocap_measurements.append(mocap.measurement(state[-1], with_noise=True, with_artifacts=False))
    if use_mocap:
        # In this case the controller will use the motion capture estimate of the pose and twist for control. 
        control = [controller.update(time[-1], mocap_measurements[-1], flat[-1])]
    else:
        control = [controller.update(time[-1], state[-1], flat[-1])]
    state_dot =  vehicle.statedot(state[0], control[0], t_step)
    imu_measurements.append(imu.measurement(state[-1], state_dot, with_noise=True))
    imu_gt.append(imu.measurement(state[-1], state_dot, with_noise=False))
    state_estimate.append(estimator.step(state[0], control[0], imu_measurements[0], mocap_measurements[0]))

    exit_status = None

    while True:
        step_start_time = perf_counter()
        exit_status = exit_status or safety_exit(world, safety_margin, state[-1], flat[-1], control[-1])
        exit_status = exit_status or normal_exit(time[-1], state[-1])
        exit_status = exit_status or time_exit(time[-1], t_final)
        if exit_status:
            break
        time.append(time[-1] + t_step)
        state[-1]['wind'] = wind_profile.update(time[-1], state[-1]['x'])
        state.append(vehicle.step(state[-1], control[-1], t_step))
        flat.append(trajectory.update(time[-1]))
        mocap_measurements.append(mocap.measurement(state[-1], with_noise=True, with_artifacts=mocap.with_artifacts))
        state_estimate.append(estimator.step(state[-1], control[-1], imu_measurements[-1], mocap_measurements[-1]))
        if use_mocap:
            control.append(controller.update(time[-1], mocap_measurements[-1], flat[-1]))
        else:
            control.append(controller.update(time[-1], state[-1], flat[-1]))
        state_dot = vehicle.statedot(state[-1], control[-1], t_step)
        imu_measurements.append(imu.measurement(state[-1], state_dot, with_noise=True))
        imu_gt.append(imu.measurement(state[-1], state_dot, with_noise=False))

        wall_dt = max(perf_counter() - step_start_time, 1e-6)
        fps = 1/wall_dt
        if print_fps:
            print(f"FPS is {fps}")

    time    = np.array(time, dtype=float)    
    state   = merge_dicts(state)
    imu_measurements = merge_dicts(imu_measurements)
    imu_gt = merge_dicts(imu_gt)
    mocap_measurements = merge_dicts(mocap_measurements)
    control         = merge_dicts(control)
    flat            = merge_dicts(flat)
    state_estimate  = merge_dicts(state_estimate)

    return (time, state, control, flat, imu_measurements, imu_gt, mocap_measurements, state_estimate, exit_status)

def merge_dicts(dicts_in):
    """
    Concatenates contents of a list of N state dicts into a single dict by
    prepending a new dimension of size N. This is more convenient for plotting
    and analysis. Requires dicts to have consistent keys and have values that
    are numpy arrays.
    """
    dict_out = {}
    for k in dicts_in[0].keys():
        dict_out[k] = []
        for d in dicts_in:
            dict_out[k].append(d[k])
        dict_out[k] = np.array(dict_out[k])
    return dict_out


def traj_end_exit(initial_state, trajectory, using_vio = False):
    """
    Returns a exit function. The exit function returns an exit status message if
    the quadrotor is near hover at the end of the provided trajectory. If the
    initial state is already at the end of the trajectory, the simulation will
    run for at least one second before testing again.
    """

    xf = trajectory.update(np.inf)['x']
    yawf = trajectory.update(np.inf)['yaw']
    rotf = Rotation.from_rotvec(yawf * np.array([0, 0, 1])) # create rotation object that describes yaw
    if np.array_equal(initial_state['x'], xf):
        min_time = 1.0
    else:
        min_time = 0

    def exit_fn(time, state):
        cur_attitude = Rotation.from_quat(state['q'])
        err_attitude = rotf * cur_attitude.inv() # Rotation between current and final
        angle = norm(err_attitude.as_rotvec()) # angle in radians from vertical
        # Success is reaching near-zero speed with near-zero position error.
        if using_vio:
            # set larger threshold for VIO due to noisy measurements
            if time >= min_time and norm(state['x'] - xf) < 1 and norm(state['v']) <= 1 and angle <= 1:
                return ExitStatus.COMPLETE
        else:
            if time >= min_time and norm(state['x'] - xf) < 0.02 and norm(state['v']) <= 0.03 and angle <= 0.02:
                return ExitStatus.COMPLETE
        return None
    return exit_fn

def time_exit(time, t_final):
    """
    Return exit status if the time exceeds t_final, otherwise None.
    """
    if time >= t_final:
        return ExitStatus.TIMEOUT
    return None

def safety_exit(world, margin, state, flat, control):
    """
    Return exit status if any safety condition is violated, otherwise None.
    """
    if np.any(np.abs(state['v']) > 20):
        return ExitStatus.OVER_SPEED
    if np.any(np.abs(state['w']) > 100):
        return ExitStatus.OVER_SPIN

    if len(world.world.get('blocks', [])) > 0:
        # If a world has objects in it we need to check for collisions.  
        collision_pts = world.path_collisions(state['x'], margin)
        no_collision = collision_pts.size == 0
        if not no_collision:
            return ExitStatus.COLLISION
    return None


def simulate_batch(world,
                   initial_states,
                   vehicles: BatchedMultirotor,
                   controller: BatchedSE3Control,
                   trajectories,
                   wind_profile,
                   imu,
                   t_final,
                   t_step,
                   safety_margin,
                   terminate=None,
                   start_times=None,
                   print_fps=False):
    """
    Simultaneously performs many vehicle simulations and returns the numerical results.
    Note that, currently, compared to the normal simulate() function, simulate_batch() does not support
    mocap, or the state estimator. See examples/batched_simulation.py for usage.

    Inputs:
        world, a class representing the world it is flying in, including objects and world bounds.
        initial_states, a dict of torch tensors defining the vehicle initial conditions with appropriate keys. See `BatchedMultirotor` for details.
        vehicles, Vehicle object containing the dynamics
        controller, BatchedController object containing the controller
        trajectories, BatchedTrajectory object containing the trajectories to follow
        wind_profile, Batched Wind Profile object containing the wind generator.
        imu: BatchedIMU object
        t_final, array of maximum simulation durations for each vehicle in the batch, s
        t_step, float, the time between each step in the simulator, s (shared across drones)
        safety_margin, the radius of the ball surrounding the vehicle position to determine if a collision occurs
        terminate, None, False, or a function of time and state that returns
            ExitStatus. If None (default), terminate when hover is reached at
            the location of trajectory with t=inf. If False, never terminate
            before timeout or error. If a function, terminate when returns not
            0.
        start_times: array (B,) indicating the times at which each drone in the batch should start. Useful if the
            different drones start at different points along their reference trajectories. Defaults to 0 for all if None.
        print_fps: bool, whether or not to print the FPS achieved by the simulation at each step.

    Outputs:
        time, seconds, numpy array of shape=(num_drones, N,) where N is the maximum number of timesteps by any drone in the batch
        state, a dict describing the state history with keys
            x, position, m, numpy array of shape=(N,B,3) where B is the number of drones in the batch
            v, linear velocity, m/s, numpy array of shape=(N,B,3)
            q, quaternion [i,j,k,w], numpy array of shape=(N,B,4)
            w, angular velocity, rad/s, numpy array of shape=(N,B,3)
            rotor_speeds, motor speeds, rad/s, numpy array of shape=(N,B,n) where n is the number of rotors
            wind, wind velocity, m/s, numpy array of shape=(N,B,3)

        control, a dict describing the command input history with keys
            cmd_motor_speeds, motor speeds, rad/s, numpy array of shape=(N,B,4)
            cmd_q, commanded orientation (not used by simulator), quaternion [i,j,k,w], shape=(N,B,4)
            cmd_w, commanded angular velocity (not used by simulator), rad/s, shape=(N,B,3)
        flat, a dict describing the desired flat outputs from the trajectory with keys
            x,        position, m
            x_dot,    velocity, m/s
            x_ddot,   acceleration, m/s**2
            x_dddot,  jerk, m/s**3
            x_ddddot, snap, m/s**4
            yaw,      yaw angle, rad
            yaw_dot,  yaw rate, rad/s
        exit_status, an array of ExitStatus enums indicating the reason for termination for each drone.
        exit_timesteps, an array indicating at which timestep each vehicle in the batch terminated, shape = (B).
            For efficiency, if drone i terminates at timestep n < N, the outputs in 'state', 'control', 'imu', 'imu_gt', and 'flat'
            for that drone for all subsequent timesteps will be NaN. 'exit_timesteps' contains the value n for each drone in the batch.
    """

    assert(torch.is_tensor(initial_states[k]) for k in initial_states.keys())

    if wind_profile is None:
        wind_profile = rotorpy.wind.default_winds.BatchedNoWind(vehicles.num_drones)
    if len(world.world['blocks']) > 0:
        raise Warning("Batched simulation does not check for collisions.")
    t_final = np.array(t_final)

    if terminate is None:    # Default exit. Terminate at final position of trajectory.
        normal_exit = traj_end_exit_batch(initial_states, trajectories, using_vio = False)
    elif terminate is False: # Never exit before timeout.
        normal_exit = lambda t, s: None
    else:                    # Custom exit.
        normal_exit = terminate

    if start_times is None:
        time_array = [np.zeros(vehicles.num_drones)]
    else:
        time_array = [start_times]
    exit_status = np.array([None] * vehicles.num_drones)
    done = np.zeros(vehicles.num_drones, dtype=bool)
    running_idxs = np.arange(vehicles.num_drones)
    exit_timesteps = np.zeros(vehicles.num_drones, dtype=int)
    state   = [copy.deepcopy(initial_states)]
    flat    = [trajectories.update(time_array[-1])]
    control = [controller.update(time_array[-1], state[-1], flat[-1], idxs=None)]
    statedot = vehicles.statedot(state[-1], control[-1], t_step, running_idxs.flatten())
    imu_measurements = [imu.measurement(state[-1], statedot, with_noise=True)]
    imu_gt = [imu.measurement(state[-1], statedot, with_noise=False)]
    step = 0
    total_num_frames = 0
    total_time = 0

    while True:
        step_start_time = time.time()
        prev_status = np.array(done, dtype=bool)
        se = safety_exit_batch(world, safety_margin, state[-1], flat[-1], control[-1])
        ne = normal_exit(time_array[-1], state[-1])
        te = time_exit_batch(time_array[-1], t_final)
        exit_status[running_idxs] = np.where(se[running_idxs], ExitStatus.OVER_SPEED, None)  # Not exactly correct.
        exit_status[running_idxs] = np.where(ne[running_idxs], ExitStatus.COMPLETE, None)
        exit_status[running_idxs] = np.where(te[running_idxs], ExitStatus.TIMEOUT, None)

        done = np.logical_or(done, se)
        done = np.logical_or(done, ne)
        done = np.logical_or(done, te)
        done_this_iter = np.logical_xor(prev_status, done)
        exit_timesteps[done_this_iter] = step+1
        if np.all(done):
            break
        running_idxs = np.nonzero(np.logical_not(done))[0]

        time_array.append(time_array[-1] + t_step)
        state[-1]['wind'] = wind_profile.update(time_array[-1], state[-1]['x'])
        state.append(vehicles.step(state[-1], control[-1], t_step, idxs=running_idxs.flatten()))
        flat.append(trajectories.update(time_array[-1]))
        control.append(controller.update(time_array[-1], state[-1], flat[-1], idxs=running_idxs.flatten()))

        statedot = vehicles.statedot(state[-1], control[-1], t_step, running_idxs.flatten())
        imu_measurements.append(imu.measurement(state[-1], statedot, running_idxs.flatten(), with_noise=True))
        imu_gt.append(imu.measurement(state[-1], statedot, running_idxs.flatten(), with_noise=False))
        step += 1
        fps = len(running_idxs) / (time.time() - step_start_time)
        total_time += time.time() - step_start_time
        total_num_frames += len(running_idxs)
        if print_fps:
            print(f"FPS at step {step} = {fps}")
    if print_fps:
        print(f"Average FPS of batched simulation was {total_num_frames/total_time}")
    time_array    = np.array(time_array, dtype=float)
    state   = merge_dicts_batch(state)
    control         = merge_dicts_batch(control)
    flat            = merge_dicts_batch(flat)
    return (time_array, state, control, flat, imu_measurements, imu_gt, exit_status, exit_timesteps)


def merge_dicts_batch(dicts_in):
    """
    Concatenates contents of a list of N state dicts into a single dict by
    prepending a new dimension of size N. This is more convenient for plotting
    and analysis. Requires dicts to have consistent keys and have values that
    are numpy arrays.
    """
    dict_out = {}
    for k in dicts_in[0].keys():
        dict_out[k] = []
        for d in dicts_in:
            dict_out[k].append(d[k].cpu().numpy())
        dict_out[k] = np.array(dict_out[k])
    return dict_out


def traj_end_exit_batch(initial_state, trajectory, using_vio = False):
    """
    Returns a exit function. The exit function returns True if
    the quadrotor is near hover at the end of the provided trajectory. If the
    initial state is already at the end of the trajectory, the simulation will
    run for at least one second before testing again.
    """

    xf = trajectory.update(np.inf)['x']
    yawf = trajectory.update(np.inf)['yaw'].unsqueeze(-1)  # (num_drones, 1)
    rotf = roma.rotvec_to_rotmat(yawf*torch.tensor([0,0,1], device=initial_state['x'].device).unsqueeze(0))
    min_times = torch.all(torch.eq(initial_state['x'], xf), dim=-1).float()

    def exit_fn(time, state):
        cur_attitudes = roma.unitquat_to_rotmat(state['q'])
        err_attitudes = rotf * torch.linalg.inv(cur_attitudes)
        angle = torch.linalg.norm(roma.rotmat_to_rotvec(err_attitudes))
        device = state['x'].device
        # Success is reaching near-zero speed with near-zero position error.
        if using_vio:
            # set larger threshold for VIO due to noisy measurements
            cond1 = torch.logical_and(torch.from_numpy(time).to(device) >= min_times, torch.linalg.norm(state['x'] - xf, dim=-1) < 1)
            cond2 = torch.logical_and(torch.linalg.norm(state['v'], dim=-1) <= 1, angle <= 1)
            cond = torch.logical_and(cond1, cond2).cpu().numpy()
        else:
            cond1 = torch.logical_and(torch.from_numpy(time).to(device) >= min_times, torch.linalg.norm(state['x'] - xf, dim=-1) < 0.02)
            cond2 = torch.logical_and(torch.linalg.norm(state['v'], dim=-1) <= 0.02, angle <= 0.02)
            cond = torch.logical_and(cond1, cond2).cpu().numpy()
        return np.where(cond, True, False)
    return exit_fn


def time_exit_batch(times: np.ndarray, t_finals: np.ndarray):
    """
    Return True if the time exceeds t_final, otherwise None.
    """
    return np.where(times >= t_finals, True, False)


def safety_exit_batch(world, margin, state, flat, control):
    """
    Return True per drone if their safety conditions is violated, otherwise 0.
    """
    status = np.zeros(state['x'].shape[0], dtype=bool)
    status = np.where(np.any(np.abs(state['v'].cpu().numpy()) > 20, axis=-1),
                      True,
                      status)
    status = np.where(np.any(np.abs(state['w'].cpu().numpy()) > 100, axis=-1),
                      True,
                      status)
    return status
