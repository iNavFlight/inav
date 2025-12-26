import numpy as np
import torch

class BatchedWind(object):
    """
    Constructs a batch of wind profiles for multiple vehicles. Each vehicle has its own wind profile. 
    This is a brute force way to interface with RotorPy's batched simulation environment. 
    It's likely to be inefficient, but it's a start. 
    If you want to speed things up, you might want to consider implementing a batched/vectorized version of your wind.
    (see default_wind.py for examples of Batched Wind objects)
    """

    def __init__(self, wind_profiles, device='cpu'):
        """
        Inputs: 
            wind_profiles, a list of wind profile objects, one for each vehicle.
        """

        self.wind_profiles = wind_profiles
        self.num_vehicles = len(wind_profiles)

        self.wind_out = torch.zeros(self.num_vehicles, 3, device=device)

    def update(self, t, position):
        """
        Given the present time and position of the vehicles in the world frame, return the
        current wind speed on all three axes for each vehicle.
        """

        for i in range(self.num_vehicles):
            vehicle_wind = self.wind_profiles[i].update(t, position[i])
            self.wind_out[i] = torch.from_numpy(vehicle_wind)

        return self.wind_out