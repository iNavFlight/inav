import numpy as np
import torch
import sys

class ThreeDCircularTraj(object):
    """

    """
    def __init__(self, center=np.array([0,0,0]), radius=np.array([1,1,1]), freq=np.array([0.2,0.2,0.2]), yaw_bool=False):
        """
        This is the constructor for the Trajectory object. A fresh trajectory
        object will be constructed before each mission.

        Inputs:
            center, the center of the circle (m)
            radius, the radius of the circle (m)
            freq, the frequency with which a circle is completed (Hz)
        """

        self.center = center
        self.cx, self.cy, self.cz = center[0], center[1], center[2]
        self.radius = radius
        self.freq = freq

        self.omega = 2*np.pi*self.freq

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
        x        = np.array([self.cx + self.radius[0]*np.cos(self.omega[0]*t),
                             self.cy + self.radius[1]*np.sin(self.omega[1]*t),
                             self.cz + self.radius[2]*np.sin(self.omega[2]*t)])
        x_dot    = np.array([-self.radius[0]*self.omega[0]*np.sin(self.omega[0]*t),
                              self.radius[1]*self.omega[1]*np.cos(self.omega[1]*t),
                              self.radius[2]*self.omega[2]*np.cos(self.omega[2]*t)])
        x_ddot   = np.array([-self.radius[0]*(self.omega[0]**2)*np.cos(self.omega[0]*t),
                             -self.radius[1]*(self.omega[1]**2)*np.sin(self.omega[1]*t),
                             -self.radius[2]*(self.omega[2]**2)*np.sin(self.omega[2]*t)])
        x_dddot  = np.array([ self.radius[0]*(self.omega[0]**3)*np.sin(self.omega[0]*t),
                             -self.radius[1]*(self.omega[1]**3)*np.cos(self.omega[1]*t),
                              self.radius[2]*(self.omega[2]**3)*np.cos(self.omega[2]*t)])
        x_ddddot = np.array([self.radius[0]*(self.omega[0]**4)*np.cos(self.omega[0]*t),
                             self.radius[1]*(self.omega[1]**4)*np.sin(self.omega[1]*t),
                             self.radius[2]*(self.omega[2]**4)*np.sin(self.omega[2]*t)])

        if self.yaw_bool:
            yaw = 0.8*np.pi/2*np.sin(2.5*t)
            yaw_dot = 0.8*2.5*np.pi/2*np.cos(2.5*t)
            yaw_ddot = 0.8*(2.5**2)*np.pi/2*np.sin(2.5*t)
        else:
            yaw = 0.0
            yaw_dot = 0.0
            yaw_ddot = 0.0

        flat_output = { 'x':x, 'x_dot':x_dot, 'x_ddot':x_ddot, 'x_dddot':x_dddot, 'x_ddddot':x_ddddot,
                        'yaw':yaw, 'yaw_dot':yaw_dot, 'yaw_ddot':yaw_ddot}
        return flat_output

class BatchedThreeDCircularTraj(object):
    """ 
    A batched version of the ThreeDCircularTraj.
    """

    def __init__(self, centers, radii, freqs, yaw_bools, device='cpu'):
        """ 
        This is the batched version of ThreeDCircularTraj. Each input must be a list of the same length, M, 
        where M is the number of circles to be completed.
        Inputs:
            centers, the center of each circle (m)
            radii, the radius of each circle (m)
            freqs, the frequency with which each circle is completed (Hz)
        """

        assert len(centers) == len(radii) == len(freqs) == len(yaw_bools), "BatchedThreeDCircularTraj Error: inputs must be of the same length"
        self.M = len(centers)

        self.centers = torch.tensor(centers, device=device)
        self.radii = torch.tensor(radii, device=device)
        self.freqs = torch.tensor(freqs)
        self.yaw_bools = torch.tensor(yaw_bools, device=device)

        self.omegas = (2*np.pi*self.freqs).to(device)

        return 

    def update(self, t):
        """ 
        Given the present time, return the desired flat output and derivatives.
        """ 

        x = torch.stack([self.centers[:,0] + self.radii[:,0]*torch.cos(self.omegas[:,0]*t),
                            self.centers[:,1] + self.radii[:,1]*torch.sin(self.omegas[:,1]*t),
                            self.centers[:,2] + self.radii[:,2]*torch.sin(self.omegas[:,2]*t)], dim=1)
        x_dot = torch.stack([-self.radii[:,0]*self.omegas[:,0]*torch.sin(self.omegas[:,0]*t),
                            self.radii[:,1]*self.omegas[:,1]*torch.cos(self.omegas[:,1]*t),
                            self.radii[:,2]*self.omegas[:,2]*torch.cos(self.omegas[:,2]*t)], dim=1)
        x_ddot = torch.stack([-self.radii[:,0]*(self.omegas[:,0]**2)*torch.cos(self.omegas[:,0]*t),
                            -self.radii[:,1]*(self.omegas[:,1]**2)*torch.sin(self.omegas[:,1]*t),
                            -self.radii[:,2]*(self.omegas[:,2]**2)*torch.sin(self.omegas[:,2]*t)], dim=1)
        x_dddot = torch.stack([ self.radii[:,0]*(self.omegas[:,0]**3)*torch.sin(self.omegas[:,0]*t),
                            -self.radii[:,1]*(self.omegas[:,1]**3)*torch.cos(self.omegas[:,1]*t),
                             self.radii[:,2]*(self.omegas[:,2]**3)*torch.cos(self.omegas[:,2]*t)], dim=1)
        x_ddddot = torch.stack([self.radii[:,0]*(self.omegas[:,0]**4)*torch.cos(self.omegas[:,0]*t),
                            self.radii[:,1]*(self.omegas[:,1]**4)*torch.sin(self.omegas[:,1]*t),
                            self.radii[:,2]*(self.omegas[:,2]**4)*torch.sin(self.omegas[:,2]*t)], dim=1)
                        
        yaw = 0.8*np.pi/2*np.sin(2.5*t)*self.yaw_bools
        yaw_dot = 0.8*2.5*np.pi/2*np.cos(2.5*t)*self.yaw_bools
        yaw_ddot = 0.8*(2.5**2)*np.pi/2*np.sin(2.5*t)*self.yaw_bools

        flat_output = { 'x':x, 'x_dot':x_dot, 'x_ddot':x_ddot, 'x_dddot':x_dddot, 'x_ddddot':x_ddddot,
                        'yaw':yaw, 'yaw_dot':yaw_dot, 'yaw_ddot':yaw_ddot}

        return flat_output