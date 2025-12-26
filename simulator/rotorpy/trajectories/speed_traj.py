"""
Imports
"""
import numpy as np

class ConstantSpeed(object):
    """
    
    """
    def __init__(self, init_pos, dist=1, speed=1, axis=0, repeat=False):
        """
        Constant speed will command a step response in speed on a specified axis. The following inputs
        try to ensure that the vehicle is not commanded to go beyond the boundaries of the environment. 

        INPUTS
            init_pos := the inital position for the trajectory to begin, should be the current quadrotor's position
            dist := the length of the trajectory in meters. 
            speed := the speed of the trajectory in meters. 
            axis := the axis to travel (0 -> X, 1 -> Y, 2 -> Z)
            repeat := determines if the trajectory should repeat, where the direction is switched from its current state.
        """

        self.pt1 = init_pos
        self.dist = dist    # m
        self.speed = speed  # m/s

        # Based on the length of the trajectory (distance) and the desired speed, compute how long the desired speed should be commaned. 
        self.t_width = self.dist/self.speed  # seconds
        
        # Create a vector where the "axis'th" element is 1 and the other elements are 0. This will multiplied by the flat outputs 
        # so that the only "active" axis is the one specified by "axis"
        self.active_axis = np.zeros((3,))
        self.active_axis[axis] = 1

        self.direction = 1

        self.repeat = repeat
        self.reverse_threshold = 0.01

        self.pt2 = self.pt1 + self.speed*self.t_width*self.active_axis

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

        v = (self.speed)*self.active_axis*self.direction
        if t <= self.t_width:
            x       = self.pt1 + v*t
            x_dot   = v
        else:
            x       = self.pt2
            x_dot   = np.zeros((3,))

        x_ddot = np.zeros((3,))
        x_dddot = np.zeros((3,))
        x_ddddot = np.zeros((3,))

        yaw    = 0
        yaw_dot = 0
        yaw_ddot = 0

        flat_output = { 'x':x, 'x_dot':x_dot, 'x_ddot':x_ddot, 'x_dddot':x_dddot, 'x_ddddot':x_ddddot,
                        'yaw':yaw, 'yaw_dot':yaw_dot, 'yaw_ddot':yaw_ddot}
        return flat_output

if __name__=="__main__":

    import matplotlib.pyplot as plt

    traj = ConstantSpeed(init_pos=np.array([0,0,0]))


    t = np.linspace(0,10,500)
    x = np.zeros((t.shape[0], 3))
    
    for i in range(t.shape[0]):
        flat = traj.update(t[i])
        x[i,:] = flat['x']

    (fig, axes) = plt.subplots(nrows=3, ncols=1, num="Constant Speed Trajectory")
    ax = axes[0]
    ax.plot(t, x[:,0], 'r', linewidth=1.5)
    ax.set_ylabel("X, m")
    ax = axes[1]
    ax.plot(t, x[:,1], 'g', linewidth=1.5)
    ax.set_ylabel("Y, m")
    ax = axes[2]
    ax.plot(t, x[:,2], 'b', linewidth=1.5)
    ax.set_ylabel("Z, m")
    ax.set_xlabel("Time, s")
    
    plt.show()
    