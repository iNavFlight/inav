from datetime import datetime
from pathlib import Path

import numpy as np
from matplotlib.animation import FuncAnimation
import matplotlib.pyplot as plt
from scipy.spatial.transform import Rotation

from rotorpy.utils.shapes import Quadrotor

import os

class ClosingFuncAnimation(FuncAnimation):
    def __init__(self, fig, func, *args, **kwargs):
        self._close_on_finish = kwargs.pop('close_on_finish')
        FuncAnimation.__init__(self, fig, func, *args, **kwargs)
        
    def _step(self, *args):
        still_going = FuncAnimation._step(self, *args)
        if self._close_on_finish and not still_going:
            plt.close(self._fig)

def _decimate_index(time, sample_time):
    """
    Given sorted lists of source times and sample times, return indices of
    source time closest to each sample time.
    """
    index = np.arange(time.size)
    sample_index = np.round(np.interp(sample_time, time, index)).astype(int)
    return sample_index

def animate(time, position, rotation, wind, animate_wind, world, filename=None, blit=False, show_axes=True, close_on_finish=False):
    """
    Animate a completed simulation result based on the time, position, and
    rotation history. The animation may be viewed live or saved to a .mp4 video
    (slower, requires additional libraries).

    For a live view, it is absolutely critical to retain a reference to the
    returned object in order to prevent garbage collection before the animation
    has completed displaying.

    Below, M corresponds to the number of drones you're animating. If M is None, i.e. the arrays are (N,3) and (N,3,3), then it is assumed that there is only one drone.
    Otherwise, we iterate over the M drones and animate them on the same axes.

    N is the number of time steps in the simulation.

    Parameters
        time, (N,) with uniform intervals
        position, (N,M,3)
        rotation, (N,M,3,3)
        wind, (N,M,3) world wind velocity
        animate_wind, if True animate wind vector
        world, a World object
        filename, for saved video, or live view if None
        blit, if True use blit for faster animation, default is False
        show_axes, if True plot axes, default is True
        close_on_finish, if True close figure at end of live animation or save, default is False
    """

    # Check if there is only one drone.
    if len(position.shape) == 2:
        position = np.expand_dims(position, axis=1)
        rotation = np.expand_dims(rotation, axis=1)
        wind = np.expand_dims(wind, axis=1)
    M = position.shape[1]

    # Temporal style.
    rtf = 1.0 # real time factor > 1.0 is faster than real time playback
    render_fps = 30

    # Normalize the wind by the max of the wind magnitude on each axis, so that the maximum length of the arrow is decided by the scale factor
    wind_mag = np.max(np.linalg.norm(wind, axis=-1), axis=1)             # Get the wind magnitude time series
    max_wind = np.max(wind_mag)                         # Find the maximum wind magnitude in the time series

    if max_wind != 0:
        wind_arrow_scale_factor = 1                         # Scale factor for the wind arrow
        wind = wind_arrow_scale_factor*wind / max_wind

    # Decimate data to render interval; always include t=0.
    if time[-1] != 0:
        sample_time = np.arange(0, time[-1], 1/render_fps * rtf)
    else:
        sample_time = np.zeros((1,))
    index = _decimate_index(time, sample_time)
    time = time[index]
    position = position[index,:]
    rotation = rotation[index,:]
    wind = wind[index,:]

    # Set up axes.
    if filename is not None:
        if isinstance(filename, Path):
            fig = plt.figure(filename.name)
        else:
            fig = plt.figure(filename)
    else:
        fig = plt.figure('Animation')
    fig.clear()
    ax = fig.add_subplot(projection='3d')
    if not show_axes:
        ax.set_axis_off()

    quads = [Quadrotor(ax, wind=animate_wind, wind_scale_factor=1) for _ in range(M)]

    world_artists = world.draw(ax)

    title_artist = ax.set_title('t = {}'.format(time[0]))

    def init():
        ax.draw(fig.canvas.get_renderer())
        # return world_artists + list(cquad.artists) + [title_artist]
        return world_artists + [title_artist] + [q.artists for q in quads]

    def update(frame):
        title_artist.set_text('t = {:.2f}'.format(time[frame]))
        for i, quad in enumerate(quads):
            quad.transform(position=position[frame,i,:], rotation=rotation[frame,i,:,:], wind=wind[frame,i,:])
        # [a.do_3d_projection(fig.canvas.get_renderer()) for a in quad.artists]   # No longer necessary in newer matplotlib?
        # return world_artists + list(quad.artists) + [title_artist]
        return world_artists + [title_artist] + [q.artists for q in quads]

    ani = ClosingFuncAnimation(fig=fig,
                        func=update,
                        frames=time.size,
                        init_func=init,
                        interval=1000.0/render_fps,
                        repeat=False,
                        blit=blit,
                        close_on_finish=close_on_finish)

    if filename is not None:
        print('Saving Animation')
        if not ".mp4" in filename:
            filename = filename + ".mp4"
        path = os.path.join(os.path.dirname(__file__),'..','data_out',filename)
        ani.save(path,
                 writer='ffmpeg',
                 fps=render_fps,
                 dpi=100)
        if close_on_finish:
            plt.close(fig)
            ani = None

    return ani