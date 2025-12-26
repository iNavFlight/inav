import numpy as np

d = 0.17 # distance from CoM to rotor (m)

# 10040_sihsim_quadx preset (aligned with PX4 SIH parameters)
quad_params = {
    'mass':             1.0,     # kg      (PX4 param SIH_MASS)
    'Ixx':              0.025,   # kg·m²   (PX4 param SIH_IXX)
    'Iyy':              0.025,   # kg·m²   (PX4 param SIH_IYY)
    'Izz':              0.030,   # kg·m²   (PX4 param SIH_IZZ)
    'Ixy':              0.0,
    'Ixz':              0.0,
    'Iyz':              0.0,

    'num_rotors':       4,
    'rotor_pos': {
        'r1':           d*np.array([ 1.0,  -1.0, 0.0]),
        'r2':           d*np.array([-1.0,   1.0, 0.0]),
        'r3':           d*np.array([ 1.0,   1.0, 0.0]),
        'r4':           d*np.array([-1.0,  -1.0, 0.0]),
    },

    # Sign for each motor’s yaw moment (+ CW, - CCW)
    'rotor_directions': np.array([ -1, -1, 1, 1 ]),
    'rI':               np.array([0.0, 0.0, 0.0]),

    'c_Dx':             1e-2,
    'c_Dy':             1e-2,
    'c_Dz':             1e-2,

    'k_eta':            5e-6,    # This is computed as SIH_T_MAX/(rotor_speed_max^2)
    'k_m':              1e-7,    # max yaw moment per rotor (Nm) to SIH_Q_MAX/(rotor_speed_max^2)
    'k_d':              0.0,    # rotor drag
    'k_z':              0.0,    # induced inflow
    'k_h':              0.0,    # translational lift
    'k_flap':           0.0,    # blade flapping moment

    'tau_m':            0.05,   # Motor response time constant (s) from SIH_T_TAU
    'rotor_speed_min':  0.0,    # [rad/s] zero throttle
    'rotor_speed_max':  1000.0,  # [rad/s] full throttle
    'motor_noise_std':  0.0,
}

