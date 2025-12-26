import numpy as np
from scipy.spatial.transform import Rotation
import matplotlib.pyplot as plt

from rotorpy.utils.animate import animate

import os

"""
Functions for showing the results from the simulator.

"""

class Plotter():

    def __init__(self, results, world):

        (self.time, self.x, self.x_des, self.v, 
        self.v_des, self.q, self.q_des, self.w, 
        self.s, self.s_des, self.M, self.T, self.wind,
        self.accel, self.gyro, self.accel_gt,
        self.x_mc, self.v_mc, self.q_mc, self.w_mc, 
        self.filter_state, self.covariance, self.sd) = self.unpack_results(results)

        self.R = Rotation.from_quat(self.q).as_matrix()
        self.R_mc = Rotation.from_quat(self.q_mc).as_matrix() # Rotation as measured by motion capture.

        self.world = world

        return

    def plot_results(self, plot_mocap, plot_estimator, plot_imu, fname=None):
        """
        Plot the results

        """

        # 3D Paths
        fig_3d = plt.figure('3D Path')
        # ax = Axes3Ds(fig)
        ax = fig_3d.add_subplot(projection='3d')
        self.world.draw(ax)
        ax.plot3D(self.x[:,0], self.x[:,1], self.x[:,2], 'b.')
        ax.plot3D(self.x_des[:,0], self.x_des[:,1], self.x_des[:,2], 'k')

        # Position and Velocity vs. Time
        (fig_posvel, axes) = plt.subplots(nrows=2, ncols=1, sharex=True, num='Pos/Vel vs Time')
        ax = axes[0]
        ax.plot(self.time, self.x_des[:,0], 'r', self.time, self.x_des[:,1], 'g', self.time, self.x_des[:,2], 'b')
        ax.plot(self.time, self.x[:,0], 'r.',    self.time, self.x[:,1], 'g.',    self.time, self.x[:,2], 'b.')
        ax.legend(('x', 'y', 'z'))
        ax.set_ylabel('position, m')
        ax.grid('major')
        ax.set_title('Position')
        ax = axes[1]
        ax.plot(self.time, self.v_des[:,0], 'r', self.time, self.v_des[:,1], 'g', self.time, self.v_des[:,2], 'b')
        ax.plot(self.time, self.v[:,0], 'r.',    self.time, self.v[:,1], 'g.',    self.time, self.v[:,2], 'b.')
        ax.legend(('x', 'y', 'z'))
        ax.set_ylabel('velocity, m/s')
        ax.set_xlabel('time, s')
        ax.grid('major')

        # Orientation and Angular Velocity vs. Time
        (fig_attrate, axes) = plt.subplots(nrows=2, ncols=1, sharex=True, num='Attitude/Rate vs Time')
        ax = axes[0]
        ax.plot(self.time, self.q_des[:,0], 'r', self.time, self.q_des[:,1], 'g', self.time, self.q_des[:,2], 'b', self.time, self.q_des[:,3], 'm')
        ax.plot(self.time, self.q[:,0], 'r.',    self.time, self.q[:,1], 'g.',    self.time, self.q[:,2], 'b.',    self.time, self.q[:,3],     'm.')
        ax.legend(('i', 'j', 'k', 'w'))
        ax.set_ylabel('quaternion')
        ax.set_xlabel('time, s')
        ax.grid('major')
        ax = axes[1]
        ax.plot(self.time, self.w[:,0], 'r.', self.time, self.w[:,1], 'g.', self.time, self.w[:,2], 'b.')
        ax.legend(('x', 'y', 'z'))
        ax.set_ylabel('angular velocity, rad/s')
        ax.set_xlabel('time, s')
        ax.grid('major')

        if plot_mocap:  # if mocap should be plotted. 
            # Motion capture position and velocity vs time
            (fig_mocapposvel, axes) = plt.subplots(nrows=2, ncols=1, sharex=True, num='Motion Capture Pos/Vel vs Time')
            ax = axes[0]
            ax.plot(self.time, self.x_mc[:,0], 'r.', self.time, self.x_mc[:,1], 'g.',    self.time, self.x_mc[:,2], 'b.')
            ax.legend(('x', 'y', 'z'))
            ax.set_ylabel('position, m')
            ax.grid('major')
            ax.set_title('MOTION CAPTURE Position/Velocity')
            ax = axes[1]
            ax.plot(self.time, self.v_mc[:,0], 'r.',    self.time, self.v_mc[:,1], 'g.',    self.time, self.v_mc[:,2], 'b.')
            ax.legend(('x', 'y', 'z'))
            ax.set_ylabel('velocity, m/s')
            ax.set_xlabel('time, s')
            ax.grid('major')
            # Motion Capture Orientation and Angular Velocity vs. Time
            (fig_mocapattrate, axes) = plt.subplots(nrows=2, ncols=1, sharex=True, num='Motion Capture Attitude/Rate vs Time')
            ax = axes[0]
            ax.plot(self.time, self.q_mc[:,0], 'r.',    self.time, self.q_mc[:,1], 'g.',    self.time, self.q_mc[:,2], 'b.',    self.time, self.q_mc[:,3],     'm.')
            ax.legend(('i', 'j', 'k', 'w'))
            ax.set_ylabel('quaternion')
            ax.set_xlabel('time, s')
            ax.grid('major')
            ax.set_title("MOTION CAPTURE Attitude/Rate")
            ax = axes[1]
            ax.plot(self.time, self.w_mc[:,0], 'r.', self.time, self.w_mc[:,1], 'g.', self.time, self.w_mc[:,2], 'b.')
            ax.legend(('x', 'y', 'z'))
            ax.set_ylabel('angular velocity, rad/s')
            ax.set_xlabel('time, s')
            ax.grid('major')

        # Commands vs. Time
        (fig_commands, axes) = plt.subplots(nrows=3, ncols=1, sharex=True, num='Commands vs Time')
        ax = axes[0]
        ax.plot(self.time, self.s_des[:,0], 'r', self.time, self.s_des[:,1], 'g', self.time, self.s_des[:,2], 'b',  self.time, self.s_des[:,3], 'k')
        ax.plot(self.time, self.s[:,0], 'r.',    self.time, self.s[:,1], 'g.',    self.time, self.s[:,2], 'b.',     self.time, self.s[:,3], 'k.')
        ax.legend(('1', '2', '3', '4'))
        ax.set_ylabel('motor speeds, rad/s')
        ax.grid('major')
        ax.set_title('Commands')
        ax = axes[1]
        ax.plot(self.time, self.M[:,0], 'r.', self.time, self.M[:,1], 'g.', self.time, self.M[:,2], 'b.')
        ax.legend(('x', 'y', 'z'))
        ax.set_ylabel('moment, N*m')
        ax.grid('major')
        ax = axes[2]
        ax.plot(self.time, self.T, 'k.')
        ax.set_ylabel('thrust, N')
        ax.set_xlabel('time, s')
        ax.grid('major')

        # Winds
        (fig_wind, axes) = plt.subplots(nrows=3, ncols=1, sharex=True, num='Winds vs Time')
        ax = axes[0]
        ax.plot(self.time, self.wind[:,0], 'r')
        ax.set_ylabel("wind X, m/s")
        ax.grid('major')
        ax.set_title('Winds')
        ax = axes[1]
        ax.plot(self.time, self.wind[:,1], 'g')
        ax.set_ylabel("wind Y, m/s")
        ax.grid('major')
        ax = axes[2]
        ax.plot(self.time, self.wind[:,2], 'b')
        ax.set_ylabel("wind Z, m/s")
        ax.set_xlabel("time, s")
        ax.grid('major')

        # IMU sensor
        if plot_imu:
            (fig_imu, axes) = plt.subplots(nrows=2, ncols=1, sharex=True, num="IMU Measurements vs Time")
            ax = axes[0]
            ax.plot(self.time, self.accel[:,0], 'r.', self.time, self.accel[:,1], 'g.', self.time, self.accel[:,2], 'b.')
            ax.plot(self.time, self.accel_gt[:,0], 'k', self.time, self.accel_gt[:,1], 'c', self.time, self.accel_gt[:,2], 'm')
            ax.set_ylabel("linear acceleration, m/s/s")
            ax.grid()
            ax = axes[1]
            ax.plot(self.time, self.gyro[:,0], 'r.', self.time, self.gyro[:,1], 'g.', self.time, self.gyro[:,2], 'b.')
            ax.set_ylabel("angular velocity, rad/s")
            ax.grid()
            ax.legend(('x','y','z'))
            ax.set_xlabel("time, s")

        if plot_estimator:
            if self.estimator_exists:
                N_filter = self.filter_state.shape[1]
                (fig_filter, axes) = plt.subplots(nrows=N_filter, ncols=1, sharex=True, num="Filter States vs Time")
                fig_filter.set_size_inches(11, 8.5)
                for i in range(N_filter):
                    ax = axes[i]
                    ax.plot(self.time, self.filter_state[:,i], 'k', )
                    ax.fill_between(self.time, self.filter_state[:,i]-self.sd[:,i], self.filter_state[:,i]+self.sd[:,i], alpha=0.3, color='k')
                    ax.set_ylabel("x"+str(i))
                ax.set_xlabel("Time, s")

                (fig_cov, axes) = plt.subplots(nrows=N_filter, ncols=1, sharex=True, num="Filter Covariance vs Time")
                fig_cov.set_size_inches(11, 8.5)
                for i in range(N_filter):
                    ax = axes[i]
                    ax.plot(self.time, self.sd[:,i]**2, 'k', )
                    ax.set_ylabel("cov(x"+str(i)+")")
                ax.set_xlabel("Time, s")

        if fname is not None:

            root_path = os.path.join(os.path.dirname(__file__),'..','data_out')
            fig_3d.savefig(os.path.join(root_path, fname+'_3dpath.png'))
            fig_posvel.savefig(os.path.join(root_path, fname+'_posvel.png'))
            fig_attrate.savefig(os.path.join(root_path, fname+'_attrate.png'))
            fig_commands.savefig(os.path.join(root_path, fname+'_commands.png'))
            fig_wind.savefig(os.path.join(root_path, fname+'_wind.png'))
            if plot_mocap:
                fig_mocapposvel.savefig(os.path.join(root_path, fname+'_mocapposvel.png'))
                fig_mocapattrate.savefig(os.path.join(root_path, fname+'_mocapattrate.png'))
            if plot_imu:
                fig_imu.savefig(os.path.join(root_path, fname+'_imu.png'))
            if plot_estimator:
                if self.estimator_exists:
                    fig_filter.savefig(os.path.join(root_path, fname+'_filter.png'))
                    fig_cov.savefig(os.path.join(root_path, fname+'_cov.png'))

        plt.show()

        return

    def animate_results(self, animate_wind, fname=None):
        """
        Animate the results
        
        """

        # Animation (Slow)
        # Instead of viewing the animation live, you may provide a .mp4 filename to save.
        ani = animate(self.time, self.x, self.R, self.wind, animate_wind, world=self.world, filename=fname)
        plt.show()

        return

    def unpack_results(self, result):

        # Unpack the dictionary of results
        time                = result['time']
        state               = result['state']
        control             = result['control']
        flat                = result['flat']
        imu_measurements    = result['imu_measurements']
        imu_gt              = result['imu_gt']
        mocap               = result['mocap_measurements']
        state_estimate      = result['state_estimate']

        # Unpack each result into NumPy arrays
        x = state['x']
        x_des = flat['x']
        v = state['v']
        v_des = flat['x_dot']

        q = state['q']
        q_des = control['cmd_q']
        w = state['w']

        s_des = control['cmd_motor_speeds']
        s = state['rotor_speeds']
        M = control['cmd_moment']
        T = control['cmd_thrust']

        wind = state['wind']

        accel   = imu_measurements['accel']
        gyro    = imu_measurements['gyro']

        accel_gt = imu_gt['accel']

        x_mc = mocap['x']
        v_mc = mocap['v']
        q_mc = mocap['q']
        w_mc = mocap['w']

        filter_state = state_estimate['filter_state']
        covariance = state_estimate['covariance']
        if filter_state.shape[1] > 0:
            sd = 3*np.sqrt(np.diagonal(covariance, axis1=1, axis2=2))
            self.estimator_exists = True
        else:
            sd = []
            self.estimator_exists = False

        return (time, x, x_des, v, v_des, q, q_des, w, s, s_des, M, T, wind, accel, gyro, accel_gt, x_mc, v_mc, q_mc, w_mc, filter_state, covariance, sd)

def plot_map(ax, world_data, equal_aspect=True, color=None, edgecolor=None, alpha=1, world_bounds=True, axes=True):
    """
    Plots the map in the world data in a top-down 2D view. 
    Inputs:
        ax: The axis to plot on
        world_data: The world data to plot
        equal_aspect: Determines if the aspect ratio of the plot should be equal.
        color: The color of the buildings. If None (default), it will use the color of the buildings. 
        edgecolor: The edge color of the buildings. If None (default), it will use the color of the buildings.
        alpha: The alpha value of the buildings. If None (default), it will use the color of the buildings.
        world_bounds: Whether or not to plot the world bounds as a dashed line around the 2D plot. 
        axes: Whether or not to plot the axis labels
    Outputs:
        Plots the map in the axis of interest. 
    """
    from matplotlib.patches import Rectangle

    # Add a dashed rectangle for the world bounds
    if world_bounds:
        world_patch = Rectangle((world_data['bounds']['extents'][0], world_data['bounds']['extents'][2]), 
                                world_data['bounds']['extents'][1]-world_data['bounds']['extents'][0], world_data['bounds']['extents'][3]-world_data['bounds']['extents'][2], 
                                linewidth=1, edgecolor='k', facecolor='none', linestyle='dashed')
        ax.add_patch(world_patch)

    plot_xmin = world_data['bounds']['extents'][0]
    plot_xmax = world_data['bounds']['extents'][1]
    plot_ymin = world_data['bounds']['extents'][2]
    plot_ymax = world_data['bounds']['extents'][3]

    for block in world_data['blocks']:
        xmin = block['extents'][0]
        xmax = block['extents'][1]
        ymin = block['extents'][2]
        ymax = block['extents'][3]
        if color is None:
            building_color = tuple(block['color'])
        else:
            building_color = color
        if edgecolor is None:
            building_edge_color = tuple(block['color'])
        else:
            building_edge_color = edgecolor
        block_patch = Rectangle((xmin, ymin), (xmax-xmin), (ymax-ymin), linewidth=1, edgecolor=building_edge_color, facecolor=building_color, alpha=alpha, fill=True)
        ax.add_patch(block_patch)

        if xmin < plot_xmin:
            plot_xmin = xmin
        if xmax > plot_xmax:
            plot_xmax = xmax
        if ymin < plot_ymin:
            plot_ymin = ymin
        if ymax > plot_ymax:
            plot_ymax = ymax

    ax.set_xlim([plot_xmin, plot_xmax])
    ax.set_ylim([plot_ymin, plot_ymax])

    if axes:
        ax.set_xlabel("X (m)")
        ax.set_ylabel("Y (m)")

    # Set the aspect ratio equal
    if equal_aspect:
        ax.set_aspect('equal')

    return

def plot_map(ax, world_data, equal_aspect=True, color=None, edgecolor=None, alpha=1, axes=True):
    """
    Plots the map in the world data in a top-down 2D view. 
    Inputs:
        ax: The axis to plot on
        world_data: The world data to plot
        equal_aspect: Determines if the aspect ratio of the plot should be equal.
        color: The color of the buildings. If None (default), it will use the color of the buildings. 
        edgecolor: The edge color of the buildings. If None (default), it will use the color of the buildings.
        alpha: The alpha value of the buildings. If None (default), it will use the color of the buildings.
        world_bounds: Whether or not to plot the world bounds as a dashed line around the 2D plot. 
        axes: Whether or not to plot the axis labels
    Outputs:
        Plots the map in the axis of interest. 
    """
    from matplotlib.patches import Rectangle

    plot_xmin = world_data['bounds']['extents'][0]
    plot_xmax = world_data['bounds']['extents'][1]
    plot_ymin = world_data['bounds']['extents'][2]
    plot_ymax = world_data['bounds']['extents'][3]

    for block in world_data['blocks']:
        xmin = block['extents'][0]
        xmax = block['extents'][1]
        ymin = block['extents'][2]
        ymax = block['extents'][3]
        if color is None:
            try:
                building_color = tuple(block['color'])
            except:
                building_color = 'k'
        else:
            building_color = color
        if edgecolor is None:
            try:
                building_edge_color = tuple(block['color'])
            except:
                building_edge_color = 'k'
        else:
            building_edge_color = edgecolor
        block_patch = Rectangle((xmin, ymin), (xmax-xmin), (ymax-ymin), linewidth=1, edgecolor=building_edge_color, facecolor=building_color, alpha=alpha, fill=True)
        ax.add_patch(block_patch)

        if xmin < plot_xmin:
            plot_xmin = xmin
        if xmax > plot_xmax:
            plot_xmax = xmax
        if ymin < plot_ymin:
            plot_ymin = ymin
        if ymax > plot_ymax:
            plot_ymax = ymax

    ax.set_xlim([plot_xmin, plot_xmax])
    ax.set_ylim([plot_ymin, plot_ymax])

    if axes:
        ax.set_xlabel("X (m)")
        ax.set_ylabel("Y (m)")

    # Set the aspect ratio equal
    if equal_aspect:
        ax.set_aspect('equal')

    return

if __name__ == "__main__":

    from rotorpy.world import World

    # Get a list of the maps available under worlds. 
    available_worlds = [fname for fname in os.listdir(os.path.abspath(os.path.join(os.path.dirname(__file__),'..','worlds'))) if 'json' in fname]

    # Load a random world
    world_fname = np.random.choice(available_worlds)
    world = World.from_file(os.path.abspath(os.path.join(os.path.dirname(__file__),'..','worlds', world_fname)))
    
    # Plot the world. 
    (fig, ax) = plt.subplots(nrows=1, ncols=1, num="Top Down World View")
    plot_map(ax, world.world)
    ax.set_title(world_fname)
    plt.show()