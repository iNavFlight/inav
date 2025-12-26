import numpy as np
from scipy.spatial.transform import Rotation
import copy

from filterpy.kalman import UnscentedKalmanFilter
from filterpy.kalman import MerweScaledSigmaPoints

"""
The Wind UKF uses the same model as the EKF found in wind_ekf.py, but instead applies the Unscented Kalman Filter. The benefit
of this approach is the accuracy of the UKF is 3rd order (compared to EKF's 1st order), and Jacobians do not need to be computed. 
"""
class WindUKF:
    """
    WindUKF
        Given approximate dynamics near level flight, the wind EKF will produce an estimate of the local wind vector acting on the body. 
        It requires knowledge of the effective drag coefficient on each axis, which would be determined either from real flight or computed in simulation, and the mass of the vehicle. 
        The inputs to the filter are the mass normalized collective thrust and the body rates on each axis. 
        Measurements of body velocity, Euler angles, and acceleration are provided to the vehicle. 
        The filter estimates the Euler angles, body velocities, and wind velocities.

        State space: 
            xhat = [psi, theta, phi, u, v, w, windx, windy, windz]
        Measurement space: 
            u = [T/m, p, q, r]
    """

    def __init__(self, quad_params,
                       Q=np.diag(np.concatenate([0.05*np.ones(3),0.07*np.ones(3),0.01*np.ones(3)])),
                       R=np.diag(np.concatenate([0.0005*np.ones(3),0.0010*np.ones(3),np.sqrt(100/2)*(0.38**2)*np.ones(3)])), 
                       xhat0=np.array([0,0,0, 0.1,0.05,0.02, 1.5,1.5,1.5]), 
                       P0=1*np.eye(9),
                       dt=1/100,
                       alpha=0.1,
                       beta=2.0,
                       kappa=-1):
        """
        Inputs:
            quad_params, dict with keys specified in quadrotor_params.py
            Q, the process noise covariance
            R, the measurement noise covariance
            x0, the initial filter state
            P0, the initial state covariance
            dt, the time between predictions
        """

        self.mass            = quad_params['mass'] # kg

        # Frame parameters
        self.c_Dx            = quad_params['c_Dx']  # drag coeff, N/(m/s)**2
        self.c_Dy            = quad_params['c_Dy']  # drag coeff, N/(m/s)**2
        self.c_Dz            = quad_params['c_Dz']  # drag coeff, N/(m/s)**2

        self.g = 9.81 # m/s^2

        # Filter parameters
        self.xhat = xhat0
        self.P = P0

        self.dt = dt

        self.N = self.xhat.shape[0]

        self.points = MerweScaledSigmaPoints(self.N, alpha=alpha, beta=beta, kappa=kappa)

        self.uk = np.array([self.g, 0, 0, 0])

        self.filter = UnscentedKalmanFilter(dim_x=self.N, dim_z=self.N, dt=dt, fx=self.f, hx=self.h, points=self.points)
        self.filter.x = xhat0
        self.filter.P = P0
        self.filter.R = R
        self.filter.Q = Q

        
    def step(self, ground_truth_state, controller_command, imu_measurement, mocap_measurement):
        """
        The step command will update the filter based on the following. 
        Inputs:
            ground_truth_state, the ground truth state is mainly there if it's necessary to compute certain portions of the state, e.g., actual thrust produced by the rotors. 
            controller_command, the controller command taken, this has to be converted to the appropriate control vector u depending on the filter model. 
            imu_measurement, contains measurements from an inertial measurement unit. These measurements are noisy, potentially biased, and potentially off-axis. The measurement
                        is specific acceleration, i.e., total force minus gravity. 
            mocap_measurement, provides noisy measurements of pose and twist. 

        Outputs:
            A dictionary with the following keys: 
                filter_state, containing the current filter estimate.
                covariance, containing the current covariance matrix.

        The ground truth state is supplied in case you would like to have "knowns" in your filter, or otherwise manipulate the state to create a custom measurement of your own desires. 
        IMU measurements are commonly used in filter setups, so we already supply these measurements as an input into the system. 
        Motion capture measurements are useful if you want noisy measurements of the pose and twist of the vehicle. 
        """

        # Extract the appropriate u vector based on the controller commands.
        self.uk = self.construct_control_vector(ground_truth_state, controller_command)

        # Construct the measurement vector yk
        orientation = Rotation.from_quat(copy.deepcopy(mocap_measurement['q']))
        euler_angles = orientation.as_euler('zyx', degrees=False)  # Get Euler angles from current orientation
        inertial_speed = mocap_measurement['v']
        body_speed = (orientation.as_matrix()).T@inertial_speed
        zk = np.array([euler_angles[0],                 # phi
                       euler_angles[1],                 # theta
                       euler_angles[2],                 # psi
                       body_speed[0],                   # vx
                       body_speed[1],                   # vy
                       body_speed[2],                   # vz
                       imu_measurement['accel'][0],     # body x acceleration
                       imu_measurement['accel'][1],     # body y acceleration
                       imu_measurement['accel'][2]      # body z acceleration
                       ])

        self.filter.predict()
        self.filter.update(zk)

        return {'filter_state': self.filter.x, 'covariance': self.filter.P}

    def f(self, xk, dt):
        """
        Process model
        """

        va = np.sqrt((xk[3]-xk[6])**2 + (xk[4]-xk[7])**2 + (xk[5]-xk[8])**2)  # Compute the norm of the airspeed vector

        # The process model is integrated using forward Euler. Below assumes Euler angles are given in order of [phi, theta, psi] (XYZ)
        # xdot = np.array([self.uk[1] + xk[0]*xk[1]*self.uk[2] + xk[2]*self.uk[3], 
        #                 self.uk[2] - xk[0]*self.uk[3], 
        #                 xk[0]*self.uk[2] + self.uk[3], 
        #                 -self.c_Dx/self.mass*(xk[3]-xk[6])*va + self.g*xk[1] + xk[4]*self.uk[3] - xk[5]*self.uk[1],
        #                 -self.c_Dy/self.mass*(xk[4]-xk[7])*va - self.g*xk[0] + xk[5]*self.uk[1] - xk[3]*self.uk[3], 
        #                 self.uk[0] - self.c_Dz/self.mass*(xk[5]-xk[8])*va - self.g + xk[3]*self.uk[2] - xk[4]*self.uk[1], 
        #                 0,
        #                 0,
        #                 0])

        # The process model, below assumes Euler angles are given in the order of [psi, theta, phi] (ZYX)
        xdot = np.array([xk[2]*self.uk[2] + self.uk[3], 
                        self.uk[2] - xk[2]*self.uk[3], 
                        self.uk[1] + xk[2]*xk[1]*self.uk[2] + xk[0]*self.uk[3], 
                        -self.c_Dx/self.mass*(xk[3]-xk[6])*va + self.g*xk[1] + xk[4]*self.uk[3] - xk[5]*self.uk[1],
                        -self.c_Dy/self.mass*(xk[4]-xk[7])*va - self.g*xk[2] + xk[5]*self.uk[1] - xk[3]*self.uk[3], 
                        self.uk[0] - self.c_Dz/self.mass*(xk[5]-xk[8])*va - self.g + xk[3]*self.uk[2] - xk[4]*self.uk[1], 
                        0,
                        0,
                        0])

        xkp1 = xk + xdot*dt

        return xkp1

    def h(self, xk):
        """
        Measurement model
        """

        h = np.zeros(xk.shape)

        va = np.sqrt((xk[3]-xk[6])**2 + (xk[4]-xk[7])**2 + (xk[5]-xk[8])**2)  # Compute the norm of the airspeed vector

        h[0:3] = np.hstack((np.eye(3), np.zeros((3,6))))@(xk)
        h[3:6] = np.hstack((np.zeros((3,3)), np.eye(3), np.zeros((3,3))))@(xk)

        h[6:] = np.array([-self.c_Dx/self.mass*(xk[3]-xk[6])*va, 
                          -self.c_Dy/self.mass*(xk[4]-xk[7])*va,
                          self.uk[0]-self.c_Dz/self.mass*(xk[5]-xk[8])*va])

        return h

    def construct_control_vector(self, ground_truth_state, controller_command):
        """
        Constructs control vector
        """
        uk = np.array([controller_command['cmd_thrust']/self.mass,    # Compute mass normalized thrust from the command thrust, note that this is not the actual thrust...
                       ground_truth_state['w'][0],                      # Body rate in x axis
                       ground_truth_state['w'][1],                      # Body rate in y axis
                       ground_truth_state['w'][2]]                      # Body rate in z axis
                       )

        return uk