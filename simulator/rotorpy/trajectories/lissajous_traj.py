import numpy as np
import torch

"""
Lissajous curves are defined by trigonometric functions parameterized in time. 
See https://en.wikipedia.org/wiki/Lissajous_curve

"""
class TwoDLissajous(object):
    """
    The standard Lissajous on the XY curve as defined by https://en.wikipedia.org/wiki/Lissajous_curve
    This is planar in the XY plane at a fixed height. 
    """
    def __init__(self, A=1, B=1, a=1, b=1, delta=0, x_offset=0, y_offset=0, height=0, yaw_bool=False):
        """
        This is the constructor for the Trajectory object. A fresh trajectory
        object will be constructed before each mission.

        Inputs:
            A := amplitude on the X axis
            B := amplitude on the Y axis
            a := frequency on the X axis
            b := frequency on the Y axis
            delta := phase offset between the x and y parameterization
            x_offset := the offset of the trajectory in the x axis
            y_offset := the offset of the trajectory in the y axis
            height := the z height that the lissajous occurs at
            yaw_bool := determines whether the vehicle should yaw
        """

        self.A, self.B = A, B
        self.a, self.b = a, b 
        self.delta = delta
        self.height = height
        self.x_offset = x_offset
        self.y_offset = y_offset

        self.yaw_bool = yaw_bool

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
        x        = np.array([self.x_offset + self.A*np.sin(self.a*t + self.delta),
                             self.y_offset + self.B*np.sin(self.b*t),
                             self.height])
        x_dot    = np.array([self.a*self.A*np.cos(self.a*t + self.delta),
                             self.b*self.B*np.cos(self.b*t),
                             0])
        x_ddot   = np.array([-(self.a)**2*self.A*np.sin(self.a*t + self.delta),
                             -(self.b)**2*self.B*np.sin(self.b*t),
                             0])
        x_dddot  = np.array([-(self.a)**3*self.A*np.cos(self.a*t + self.delta),
                             -(self.b)**3*self.B*np.cos(self.b*t),
                             0])
        x_ddddot = np.array([(self.a)**4*self.A*np.sin(self.a*t + self.delta),
                             (self.b)**4*self.B*np.sin(self.b*t),
                             0])

        if self.yaw_bool:
            yaw = np.pi/4*np.sin(np.pi*t)
            yaw_dot = np.pi*np.pi/4*np.cos(np.pi*t)
            yaw_ddot = np.pi*np.pi*np.pi/4*np.cos(np.pi*t)
        else:
            yaw = 0
            yaw_dot = 0
            yaw_ddot = 0

        flat_output = { 'x':x, 'x_dot':x_dot, 'x_ddot':x_ddot, 'x_dddot':x_dddot, 'x_ddddot':x_ddddot,
                        'yaw':yaw, 'yaw_dot':yaw_dot, 'yaw_ddot':yaw_ddot}
        return flat_output

class BatchedTwoDLissajous(object):
    """ 
    This is a batched version of the TwoDLissajous trajectory.
    """ 

    def __init__(self, A_s, B_s, a_s, b_s, delta_s, x_offset_s, y_offset_s, height_s, yaw_bool_s, device='cpu'):
        """
        Inputs:
            A_s := array of amplitudes on the X axis, with shape (num_uavs,)
            B_s := array of amplitudes on the Y axis, with shape (num_uavs,)
            a_s := array of frequencies on the X axis, with shape (num_uavs,)
            b_s := array of frequencies on the Y axis, with shape (num_uavs,)
            delta_s := array of phase offsets between the x and y parameterization: (num_uavs,)
            x_offset_s := array of the offset of the trajectory in the x axis: (num_uavs,)
            y_offset_s := array of the offset of the trajectory in the y axis: (num_uavs,)
            height_s := array of the z height that the lissajous occurs at: (num_uavs,)
            yaw_bool_s := array of booleans determining whether the vehicle should yaw: (num_uavs,)
            device := the device to run the simulation on
        """

        assert len(A_s) == len(B_s) == len(a_s) == len(b_s) == len(delta_s) == len(x_offset_s) == len(y_offset_s) == len(height_s) == len(yaw_bool_s), "All inputs must have the same length"

        self.A_s, self.B_s = torch.tensor(A_s, device=device), torch.tensor(B_s, device=device)
        self.a_s, self.b_s = torch.tensor(a_s, device=device), torch.tensor(b_s, device=device)
        self.delta_s = torch.tensor(delta_s, device=device)
        self.height_s = torch.tensor(height_s, device=device)
        self.x_offset_s = torch.tensor(x_offset_s, device=device)
        self.y_offset_s = torch.tensor(y_offset_s, device=device)
        self.yaw_bool_s = torch.tensor(yaw_bool_s, device=device)

        self.num_uavs = len(A_s)

    def update(self, t):
        """ 
        Given the present time, return the desired flat output and derivatives for each uav.
        """

        x = torch.stack([self.x_offset_s + self.A_s*torch.sin(self.a_s*t + self.delta_s),
                         self.y_offset_s + self.B_s*torch.sin(self.b_s*t),
                         self.height_s], dim=1)
        x_dot = torch.stack([self.a_s*self.A_s*torch.cos(self.a_s*t + self.delta_s),
                             self.b_s*self.B_s*torch.cos(self.b_s*t),
                             torch.zeros(self.num_uavs)], dim=1)
        x_ddot = torch.stack([-(self.a_s)**2*self.A_s*torch.sin(self.a_s*t + self.delta_s),
                             -(self.b_s)**2*self.B_s*torch.sin(self.b_s*t),
                             torch.zeros(self.num_uavs)], dim=1)
        x_dddot = torch.stack([-(self.a_s)**3*self.A_s*torch.cos(self.a_s*t + self.delta_s),
                             -(self.b_s)**3*self.B_s*torch.cos(self.b_s*t),
                             torch.zeros(self.num_uavs)], dim=1)
        x_ddddot = torch.stack([(self.a_s)**4*self.A_s*torch.sin(self.a_s*t + self.delta_s),
                             (self.b_s)**4*self.B_s*torch.sin(self.b_s*t),
                             torch.zeros(self.num_uavs)], dim=1)

        yaw = np.pi/4*np.sin(np.pi*t)*self.yaw_bool_s
        yaw_dot = np.pi*np.pi/4*np.cos(np.pi*t)*self.yaw_bool_s
        yaw_ddot = np.pi*np.pi*np.pi/4*np.cos(np.pi*t)*self.yaw_bool_s

        flat_output = { 'x':x, 'x_dot':x_dot, 'x_ddot':x_ddot, 'x_dddot':x_dddot, 'x_ddddot':x_ddddot,
                        'yaw':yaw, 'yaw_dot':yaw_dot, 'yaw_ddot':yaw_ddot}
        return flat_output