from rotorpy.vehicles.multirotor import BatchedMultirotorParams
from rotorpy.vehicles.crazyflie_params import quad_params as cf_params
import numpy as np

crazyflie_randomizations = {
    "mass": [0.027, 0.033],
    "k_eta": [2.1e-8, 2.5e-8],
}

def generate_random_vehicle_params(num_drones,
                                    device,
                                    nominal_params = cf_params,
                                    randomization_ranges = crazyflie_randomizations):
    """ 
    Generate random vehicle params. 
    Inputs:
        num_drones: the number of drones to generate params for. 
        nominal_params: the nominal parameters for the drone, i.e. the center of each sampling distribution.
        randomization_ranges: the range with which parameters are selected. 
    """

    batch_params = BatchedMultirotorParams([nominal_params for _ in range(num_drones)], num_drones, device)
    for idx in range(num_drones):
        update_vehicle_params(idx,
                              randomization_ranges,
                              batch_params)
    return batch_params

def update_vehicle_params(idx,
                          ranges,
                          params_obj):
    """
    Update vehicle parameters. 
    Inputs:
        idx: the particular idx of the drone. 
        ranges: the range of values to sample from. 
        prams_obj: the object (type BatchedMultirotorParams) to modify.

    """
    min_k_eta = 0
    if "mass" in ranges:
        mass_val = np.random.uniform(ranges["mass"][0], ranges["mass"][1])
        params_obj.update_mass(idx, mass_val)

        # divide rotor speed max by 1.8 to give some margin to allow flight 
        min_k_eta = (mass_val * (params_obj.g / 4) / ((params_obj.rotor_speed_max[idx]/1.2) ** 2)).item()
    if "k_eta" in ranges or "k_m" in ranges or "rotor_pos" in ranges:
        k_eta = np.random.uniform(max(ranges["k_eta"][0], min_k_eta),
                                  ranges["k_eta"][1]) if "k_eta" in ranges else None
        k_m = np.random.uniform(ranges["k_m"][0],
                                ranges["k_m"][1]) if "k_m" in ranges else None
        rotor_pos=None  # not implemented yet
        params_obj.update_thrust_and_rotor_params(idx, k_eta, k_m, rotor_pos)
    if "Ixx" in ranges or "Iyy" in ranges or "Izz" in ranges:
        Ixx = np.random.uniform(ranges["Ixx"][0],
                                ranges["Ixx"][1]) if "Ixx" in ranges else None
        Iyy  = np.random.uniform(ranges["Iyy"][0],
                                 ranges["Iyy"][1]) if "Iyy" in ranges else None
        Izz  = np.random.uniform(ranges["Izz"][0],
                                 ranges["Izz"][1]) if "Izz" in ranges else None
        params_obj.update_inertia(idx, Ixx, Iyy, Izz)
    if "tau_m" in ranges:
        tau_m = np.random.uniform(ranges["tau_m"][0],ranges["tau_m"][1])
        params_obj.tau_m[idx] = tau_m
    if "motor_noise" in ranges:
        motor_noise = np.random.uniform(ranges["motor_noise"][0], ranges["motor_noise"][1])
        params_obj.motor_noise[idx] = motor_noise

    return 