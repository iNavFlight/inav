import numpy as np
from scipy.spatial.transform import Rotation
import copy

"""
This is the null estimator, which outputs nothing. This code serves two purposes: 
1) When the user does not supply an estimator, the null estimator is the default "estimator" for the simulator. 
2) If a user wants to write their own estimator, the null estimator is good stub code because it specifies the input/output structure used by the simulator. 
"""
class NullEstimator:
    """
    NullEstimator
        Description of your filter goes here. 

        State space: 
            Describe your filter state space here, being as specific as necessary and ideally specifying types, too. 
                e.g. X = [x,y,z,xdot,ydot,zdot]
        Measurement space: 
            Describe your filter measurement space here, being as specific as necessary and ideally specifying types, too. 
                e.g. Y = [imu_x, imu_y, imu_z]
    """

    def __init__(self):
        """
        Here you would specify important design parameters for your estimator, such as noise matrices, initial state, or other tuning parameters. 
        For the null estimator, the initial state and covariance are specified (but are default values anyways)
        """
        
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
        self.propagate(ground_truth_state, controller_command)
        self.update(ground_truth_state, controller_command, imu_measurement, mocap_measurement)

        return {'filter_state': [], 'covariance': []}

    """ OPTIONAL """
    def propagate(self, ground_truth_state, controller_command):
        """
        Propagate occurs whenever an action u is taken.
        Inputs:
            ground_truth_state, the ground truth state is mainly there if it's necessary to compute certain portions of the state, e.g., actual thrust produced by the rotors. 
            controller_command, the controller command taken, this has to be converted to the appropriate control vector u depending on the filter model. 
        """

        return {'filter_state': [], 'covariance': []}


    """ OPTIONAL """
    def update(self, ground_truth_state, controller_command, imu_measurement, mocap_measurement):
        """
        Update the estimate based on new sensor measurments. 
        Inputs:
            ground_truth_state, the ground truth state is mainly there if it's necessary to compute certain portions of the state, e.g., actual thrust produced by the rotors. 
            controller_command, the controller command taken, this has to be converted to the appropriate control vector u depending on the filter model. 
            imu_measurement, contains measurements from an inertial measurement unit. These measurements are noisy, potentially biased, and potentially off-axis. The measurement
                        is specific acceleration, i.e., total force minus gravity. 
            mocap_measurement, provides noisy measurements of pose and twist. 
        """

        return {'filter_state': [], 'covariance': []}
