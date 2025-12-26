"""
Physical parameters for the AscTec Hummingbird. Values parameterize the inertia, motor dynamics, 
rotor aerodynamics, parasitic drag, and rotor placement. 
Additional sources:
    https://digitalrepository.unm.edu/cgi/viewcontent.cgi?article=1189&context=ece_etds
    https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=7991501
Notes:
    The k_d and k_z terms are an order of magnitude smaller because 10^-3 was too large. 
"""
import numpy as np

d = 0.17 # Arm length

quad_params = {

    # Inertial properties
    'mass': 0.500,       # kg
    'Ixx':  3.65e-3,    # kg*m^2
    'Iyy':  3.68e-3,    # kg*m^2
    'Izz':  7.03e-3,    # kg*m^2
    'Ixy':  0.0,        # kg*m^2
    'Iyz':  0.0,        # kg*m^2 
    'Ixz':  0.0,        # kg*m^2

    # Geometric properties, all vectors are relative to the center of mass.
    'num_rotors': 4,                        # for looping over each actuator
    'rotor_radius': 0.10,                   # rotor radius, in meters
    'rotor_pos': {  
                    'r1': d*np.array([ 0.70710678118, 0.70710678118, 0]),    # Location of Rotor 1, meters
                    'r2': d*np.array([ 0.70710678118,-0.70710678118, 0]),    # Location of Rotor 2, meters
                    'r3': d*np.array([-0.70710678118,-0.70710678118, 0]),    # Location of Rotor 3, meters
                    'r4': d*np.array([-0.70710678118, 0.70710678118, 0]),    # Location of Rotor 4, meters
                 },

    'rotor_directions': np.array([1,-1,1,-1]),  # This dictates the direction of the torque for each motor. 

    'rI': np.array([0,0,0]), # location of the IMU sensor, meters

    # Frame aerodynamic properties
    'c_Dx': 0.5e-2,  # parasitic drag in body x axis, N/(m/s)**2
    'c_Dy': 0.5e-2,  # parasitic drag in body y axis, N/(m/s)**2
    'c_Dz': 1e-2,  # parasitic drag in body z axis, N/(m/s)**2

    # Rotor properties
    'k_eta': 5.57e-06,          # thrust coefficient N/(rad/s)**2
    'k_m':   1.36e-07,          # yaw moment coefficient Nm/(rad/s)**2
    'k_d':   1.19e-04,          # rotor drag coefficient N/(rad*m/s**2) = kg/rad
    'k_z':   2.32e-04,          # induced inflow coefficient N/(rad*m/s**2) = kg/rad
    'k_h':   3.39e-3,           # translational lift coefficient (N/(m/s)**2) = kg/m
    'k_flap': 0.0,              # Flapping moment coefficient Nm/(rad*m/s**2) = kg*m/rad

    # Motor properties
    'tau_m': 0.005,             # motor response time, seconds
    'rotor_speed_min': 0,       # rad/s
    'rotor_speed_max': 1500,    # rad/s
    'motor_noise_std': 0.0,     # rad/s

    # Lower level controller properties (for higher level control abstractions)
    'k_w': 1,               # The body rate P gain (for cmd_ctbr)
    'k_v': 10,              # The *world* velocity P gain (for cmd_vel)
    'kp_att': 544,          # The attitude P gain (for cmd_vel, cmd_acc, and cmd_ctatt)
    'kd_att': 46.64,        # The attitude D gain (for cmd_vel, cmd_acc, and cmd_ctatt)

}