import numpy as np
import torch

class HoverTraj(object):
    """
    This trajectory simply has the quadrotor hover at the origin indefinitely.
    By modifying the initial condition, you can create step response
    experiments.
    """
    def __init__(self, x0=np.array([0, 0, 0])):
        """
        This is the constructor for the Trajectory object. A fresh trajectory
        object will be constructed before each mission.
        """

        self.x0 = x0

    def update(self, t):
        """
        Given the present time, return the desired flat output and derivatives.

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
        x = self.x0
        x_dot = np.zeros((3,))
        x_ddot = np.zeros((3,))
        x_dddot = np.zeros((3,))
        x_ddddot = np.zeros((3,))
        yaw    = 0
        yaw_dot = 0
        yaw_ddot = 0

        flat_output = { 'x':x, 'x_dot':x_dot, 'x_ddot':x_ddot, 'x_dddot':x_dddot, 'x_ddddot':x_ddddot,
                        'yaw':yaw, 'yaw_dot':yaw_dot, 'yaw_ddot':yaw_ddot}
        return flat_output

class BatchedHoverTraj(object):
    """ 
    This is a batched version of the HoverTraj trajectory above. 
    """

    def __init__(self, num_uavs, x0=None, device='cpu'):
        """
        Set the initial conditions for the batched hover trajectory.
        Inputs:
            num_uavs, the number of UAVs in the batch.
            x0, a numpy array of shape (num_uavs, 3) containing the initial conditions. 
                if x0 is None, the initial conditions will be set to the origin for all UAVs.
        """

        if x0 is not None:
            assert x0.shape[0] == num_uavs, "x0 must have shape (num_uavs, 3)"
            x0 = torch.tensor(x0)
        else:
            x0 = torch.zeros(num_uavs, 3)

        self.num_uavs = num_uavs
        self.x0 = x0

        self.flat_outputs = { 'x':self.x0, 'x_dot':torch.zeros(num_uavs, 3), 'x_ddot':torch.zeros(num_uavs, 3),
                              'x_dddot':torch.zeros(num_uavs, 3), 'x_ddddot':torch.zeros(num_uavs, 3),
                              'yaw':torch.zeros(num_uavs), 'yaw_dot':torch.zeros(num_uavs), 'yaw_ddot':torch.zeros(num_uavs)}

    def update(self, t):
        """ 
        Given the present time, return the desired flat output and derivatives for each UAV.
        """

        return self.flat_outputs