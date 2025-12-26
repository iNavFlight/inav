import numpy as np
import torch

class BatchedTrajectory(object):
    """
    Constructs a batch of trajectories for multiple uav. Each uav has its own trajectory. 
    This is a brute force way to interface with RotorPy's batched simulation environment. 
    It's likely to be inefficient, but it's a start. 
    If you want to speed things up, you might want to consider implementing a batched/vectorized version of your trajectories.

    """
    def __init__(self, trajectories, device='cpu'):
        """
        Inputs:
            trajectories, a list of trajectory objects, one for each uav. 
        """

        self.trajectories = trajectories
        self.num_uavs = len(trajectories)

        self.x_out = torch.zeros(self.num_uavs, 3, device=device)
        self.x_dot_out = torch.zeros(self.num_uavs, 3, device=device)
        self.x_ddot_out = torch.zeros(self.num_uavs, 3, device=device)
        self.x_dddot_out = torch.zeros(self.num_uavs, 3, device=device)
        self.x_ddddot_out = torch.zeros(self.num_uavs, 3, device=device)
        self.yaw_out = torch.zeros(self.num_uavs, device=device)
        self.yaw_dot_out = torch.zeros(self.num_uavs, device=device)
        self.yaw_ddot_out = torch.zeros(self.num_uavs, device=device)

    def update(self, t):
        """
        Given the present time, return the desired flat output and derivatives for each uav.

        Inputs
            t, time, s
        Outputs
            flat_output, a dict describing the present desired flat outputs with keys
                x,        position, m
                x_dot,    velocity, m/s
                x_ddot,   acceleration, m/s**2
                x_dddot,  jerk, m/s**3
                x_ddddot, snap, m/s**4
                yaw,      yaw angle, rad
                yaw_dot,  yaw rate, rad/s
        """

        for i in range(self.num_uavs):
            uav_flat_output = self.trajectories[i].update(t)
            self.x_out[i] = torch.from_numpy(uav_flat_output['x'])
            self.x_dot_out[i] = torch.from_numpy(uav_flat_output['x_dot'])
            self.x_ddot_out[i] = torch.from_numpy(uav_flat_output['x_ddot'])
            self.x_dddot_out[i] = torch.from_numpy(uav_flat_output['x_dddot'])
            self.x_ddddot_out[i] = torch.from_numpy(uav_flat_output['x_ddddot'])
            self.yaw_out[i] = torch.tensor(uav_flat_output['yaw'])
            self.yaw_dot_out[i] = torch.tensor(uav_flat_output['yaw_dot'])
            self.yaw_ddot_out[i] = torch.tensor(uav_flat_output['yaw_ddot'])

        flat_outputs = { 'x':self.x_out, 'x_dot':self.x_dot_out, 'x_ddot':self.x_ddot_out, 'x_dddot':self.x_dddot_out, 'x_ddddot':self.x_ddddot_out,
                        'yaw':self.yaw_out, 'yaw_dot':self.yaw_dot_out, 'yaw_ddot':self.yaw_ddot_out}

        return flat_outputs