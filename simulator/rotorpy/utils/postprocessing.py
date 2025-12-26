import pandas as pd
import numpy as np
from scipy.spatial.transform import Rotation
import os


def unpack_sim_data(result):
    """
    Unpacks the simulated result dictionary and converts it to a Pandas DataFrame

    """
    
    headers = [ 'time',                                                                                                                                                         # Time
            'x', 'y', 'z', 'xdot', 'ydot', 'zdot', 'qx', 'qy', 'qz', 'qw', 'wx', 'wy', 'wz', 'windx', 'windy', 'windz', 'r1', 'r2', 'r3', 'r4',                             # Vehicle state                                                                                                                                               # GT body velocity
            'xdes', 'ydes', 'zdes', 'xdotdes', 'ydotdes', 'zdotdes', 'xddotdes', 'yddotdes', 'zddotdes', 'xdddotdes', 'ydddotdes', 'zdddotdes',                             # Flat outputs
            'xddddotdes', 'yddddotdes', 'zddddotdes', 'yawdes', 'yawdotdes', 
            'ax', 'ay', 'az', 'ax_gt', 'ay_gt', 'az_gt', 'gx', 'gy', 'gz',                                                                                                  # IMU measurements
            'mocap_x', 'mocap_y', 'mocap_z', 'mocap_xdot', 'mocap_ydot', 'mocap_zdot', 'mocap_qx', 'mocap_qy', 'mocap_qz', 'mocap_qw', 'mocap_wx', 'mocap_wy', 'mocap_wz',  # Mocap measurements
            'r1des', 'r2des', 'r3des', 'r4des', 'thrustdes', 'qxdes', 'qydes', 'qzdes', 'qwdes', 'mxdes', 'mydes', 'mzdes',                                                 # Controller
    ]

    # Extract data into numpy arrays
    time                = result['time'].reshape(-1,1)
    state               = result['state']
    x               = state['x']
    v               = state['v']
    q               = state['q']
    w               = state['w']
    wind            = state['wind']
    rotor_speeds    = state['rotor_speeds']
    control             = result['control']
    cmd_rotor   = control['cmd_motor_speeds']
    cmd_thrust  = control['cmd_thrust'].reshape(-1,1)
    cmd_q       = control['cmd_q']
    cmd_moment  = control['cmd_moment']

    flat                = result['flat']
    x_des       = flat['x']
    v_des       = flat['x_dot']
    a_des       = flat['x_ddot']
    j_des       = flat['x_dddot']
    s_des       = flat['x_ddddot']
    yaw_des     = flat['yaw'].reshape(-1,1)
    yawdot_des  = flat['yaw_dot'].reshape(-1,1)

    imu_measurements    = result['imu_measurements']
    a_measured = imu_measurements['accel']
    w_measured = imu_measurements['gyro']

    mocap_measurements = result['mocap_measurements']
    x_mc = mocap_measurements['x']
    v_mc = mocap_measurements['v']
    q_mc = mocap_measurements['q']
    w_mc = mocap_measurements['w']

    imu_actual          = result['imu_gt']
    a_actual = imu_actual['accel']

    state_estimate      = result['state_estimate']
    filter_state = state_estimate['filter_state']
    covariance = state_estimate['covariance']

    if filter_state.shape[1] > 0:
        # Computes the standard deviation of the filter covariance diagonal elements
        sd = 3*np.sqrt(np.diagonal(covariance, axis1=1, axis2=2))
        headers.extend(['xhat_'+str(i) for i in range(filter_state.shape[1])])
        headers.extend(['sigma_'+str(i) for i in range(filter_state.shape[1])])

        dataset = np.hstack((time,
                             x, v, q, w, wind, rotor_speeds,
                             x_des, v_des, a_des, j_des, s_des, yaw_des, yawdot_des, 
                             a_measured, a_actual, w_measured, 
                             x_mc, v_mc, q_mc, w_mc,
                             cmd_rotor, cmd_thrust, cmd_q, cmd_moment, 
                             filter_state, sd))
    else:
        sd = []

        dataset = np.hstack((time,
                            x, v, q, w, wind, rotor_speeds,
                            x_des, v_des, a_des, j_des, s_des, yaw_des, yawdot_des, 
                            a_measured, a_actual, w_measured, 
                            x_mc, v_mc, q_mc, w_mc,
                            cmd_rotor, cmd_thrust, cmd_q, cmd_moment))
    df = pd.DataFrame(data=dataset,
                        columns=headers)
    
    return df

def save_sim_data(result, filename="output.csv"):
    """
    Saves the data in result as a csv file for post processing.
    """

    if not (".csv" in filename):
        # Check whether or not filename contains .csv
        filename = filename + ".csv"

    # Get dataframe from simulation data. 
    df = unpack_sim_data(result)

    # Send it to csv file and save it in /rotorpy/data_out/ file. 
    path = os.path.join(os.path.dirname(__file__), '..', 'data_out/'+filename)
    df.to_csv(path)

    return 