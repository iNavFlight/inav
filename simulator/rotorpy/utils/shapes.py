"""
Parametric 3D shapes for spatial plots and animations. Shapes are drawn on an
Axes3D axes, and then can be moved using .transform(). They can return a list of
artists to support blitting in animations.

TODO:
  There is a fair amount of code duplication here; a superclass may be warranted.

"""

import itertools
import numpy as np
from mpl_toolkits.mplot3d import art3d
import matplotlib.colors as mcolors
from scipy.spatial.transform import Rotation

"""
Necessary functions for visualization 
From original mplot3d version by John Porter (Created: 23 Sep 2005)

Parts fixed by Reinier Heeres <reinier@heeres.eu>
Minor additions by Ben Axelrod <baxelrod@coroware.com>
Significant updates and revisions by Ben Root <ben.v.root@gmail.com>

Current as of matplotlib v3.2.2 but changed at some point.
Modified by Spencer Folk
"""

def _generate_normals(polygons):
    '''
    Generate normals for polygons by using the first three points.
    This normal of course might not make sense for polygons with
    more than three points not lying in a plane.
    '''

    normals = []
    for verts in polygons:
        v1 = np.array(verts[0]) - np.array(verts[1])
        v2 = np.array(verts[2]) - np.array(verts[0])
        normals.append(np.cross(v1, v2))
    return normals

def _shade_colors(color, normals):
    '''
    Shade *color* using normal vectors given by *normals*.
    *color* can also be an array of the same length as *normals*.
    '''

    shade = np.array([np.dot(n / np.linalg.norm(n), [-1, -1, 0.5])
                        if np.linalg.norm(n) else np.nan
                        for n in normals])
    mask = ~np.isnan(shade)

    if len(shade[mask]) > 0:
        norm = mcolors.Normalize(min(shade[mask]), max(shade[mask]))
        shade[~mask] = min(shade[mask])
        color = mcolors.to_rgba_array(color)
        # shape of color should be (M, 4) (where M is number of faces)
        # shape of shade should be (M,)
        # colors should have final shape of (M, 4)
        alpha = color[:, 3]
        colors = (0.5 + norm(shade)[:, np.newaxis] * 0.5) * color
        colors[:, 3] = alpha
    else:
        colors = np.asanyarray(color).copy()

    return colors

class Face():

    def __init__(self, ax, corners, *,
        shade=True,
        alpha=1.0,
        facecolors=None,
        edgecolors=None,
        linewidth=0,
        antialiased=True):
        """
        Parameters
            ax, Axes3D to contain new shape
            corners, shape=(N,3)
            shade, shade faces using default lightsource, default is True
            linewidth, width of lines, default is 0
            alpha, transparency value in domain [0,1], default is 1.0
            edgecolors, color of edges
            facecolors, color of faces
            antialiased, smoother edge lines, default is True
        """
        self.shade = shade
        self.facecolors = facecolors

        self.ax = ax

        if self.facecolors is None:
            self.facecolors = self.ax._get_lines.get_next_color()
        self.facecolors = np.array(mcolors.to_rgba(self.facecolors))

        # Precompute verticies and normal vectors in reference configuration.
        self.verts = np.reshape(corners, (1, -1, 3))
        self.normals = np.asarray(_generate_normals(self.verts))

        # Instantiate and add collection.
        self.polyc = art3d.Poly3DCollection(self.verts, linewidth=linewidth, antialiased=antialiased, alpha=alpha, edgecolors=edgecolors, facecolors=self.facecolors)
        self.artists = (self.polyc,)
        self.transform(np.zeros((3,)), np.identity(3))
        self.ax.add_collection(self.polyc)

    def transform(self, position, rotation=np.identity(3)):

        position = np.array(position)
        position.shape = (3,1)

        # The verts array is indexed as (i_face, j_coordinate, k_point).
        verts = np.swapaxes(self.verts, 1, 2)
        new_verts = np.matmul(rotation, verts) + position
        self.polyc.set_verts(np.swapaxes(new_verts, 1, 2))

        if self.shade:
            normals = np.matmul(rotation, self.normals.T).T
            colset = _shade_colors(self.facecolors, normals)
        else:
            colset = self.facecolors
        self.polyc.set_facecolors(colset)

class Cuboid():

    def __init__(self, ax, x_span, y_span, z_span, *,
        shade=True,
        alpha=1.0,
        facecolors=None,
        edgecolors=None,
        linewidth=0,
        antialiased=True):
        """
        Parameters
            ax, Axes3D to contain new shape
            x_span, width in x-direction
            y_span, width in y-direction
            z_span, width in z-direction
            shade, shade faces using default lightsource, default is True
            linewidth, width of lines, default is 0
            alpha, transparency value in domain [0,1], default is 1.0
            edgecolors, color of edges
            facecolors, color of faces
            antialiased, smoother edge lines, default is True
        """
        self.shade = shade
        self.facecolors = facecolors

        self.ax = ax

        if self.facecolors is None:
            self.facecolors = self.ax._get_lines.get_next_color()
        self.facecolors = np.array(mcolors.to_rgba(self.facecolors))

        # Precompute verticies and normal vectors in reference configuration.
        self.verts = self.build_verts(x_span, y_span, z_span)
        self.normals = np.asarray(_generate_normals(self.verts))

        # Instantiate and add collection.
        self.polyc = art3d.Poly3DCollection(self.verts, linewidth=linewidth, antialiased=antialiased, alpha=alpha, edgecolors=edgecolors, facecolors=self.facecolors)
        self.artists = (self.polyc,)
        self.transform(np.zeros((3,)), np.identity(3))
        self.ax.add_collection(self.polyc)

    def transform(self, position, rotation=np.identity(3)):

        position = np.array(position)
        position.shape = (3,1)

        # The verts array is indexed as (i_face, j_coordinate, k_point).
        verts = np.swapaxes(self.verts, 1, 2)
        new_verts = np.matmul(rotation, verts) + position
        self.polyc.set_verts(np.swapaxes(new_verts, 1, 2))

        if self.shade:
            normals = np.matmul(rotation, self.normals.T).T
            colset = _shade_colors(self.facecolors, normals)
        else:
            colset = self.facecolors
        self.polyc.set_facecolors(colset)

    def build_verts(self, x_span, y_span, z_span):
        """
        Input
            x_span, width in x-direction
            y_span, width in y-direction
            z_span, width in z-direction
        Returns
            verts, shape=(6_faces, 4_points, 3_coordinates)
        """

        # Coordinates of each point.
        (x, y, z) = (x_span, y_span, z_span)
        bot_pts = np.array([
            [0, 0, 0],
            [x, 0, 0],
            [x, y, 0],
            [0, y, 0]])
        top_pts = np.array([
            [0, 0, z],
            [x, 0, z],
            [x, y, z],
            [0, y, z]])
        pts = np.concatenate((bot_pts, top_pts), axis=0)

        # Indices of points for each face.
        side_faces = [(i, (i+1)%4, 4+((i+1)%4), 4+i) for i in range(4)]
        side_faces = np.array(side_faces, dtype=int)
        bot_faces = np.arange(4, dtype=int)
        bot_faces.shape = (1,4)
        top_faces = 4 + bot_faces
        all_faces = np.concatenate((side_faces, bot_faces, top_faces), axis=0)

        # Vertex list.
        xt = pts[:,0][all_faces]
        yt = pts[:,1][all_faces]
        zt = pts[:,2][all_faces]
        verts = np.stack((xt, yt, zt), axis=-1)

        return verts

class Cylinder():

    def __init__(self, ax, radius, height, n_pts=8, shade=True, color=None):
        self.shade = shade

        self.ax = ax

        if color is None:
            color = self.ax._get_lines.get_next_color()
        self.color = np.array(mcolors.to_rgba(color))

        # Precompute verticies and normal vectors in reference configuration.
        self.verts = self.build_verts(radius, height, n_pts)
        self.normals = np.asarray(_generate_normals(self.verts))

        # Instantiate and add collection.
        self.polyc = art3d.Poly3DCollection(self.verts, color='b', linewidth=0, antialiased=False)
        self.artists = (self.polyc,)
        self.transform(np.zeros((3,)), np.identity(3))
        self.ax.add_collection(self.polyc)

    def transform(self, position, rotation):

        position.shape = (3,1)

        # The verts array is indexed as (i_triangle, j_coordinate, k_point).
        verts = np.swapaxes(self.verts, 1, 2)
        new_verts = np.matmul(rotation, verts) + position
        self.polyc.set_verts(np.swapaxes(new_verts, 1, 2))

        if self.shade:
            normals = np.matmul(rotation, self.normals.T).T
            colset = _shade_colors(self.color, normals)
        else:
            colset = self.color
        self.polyc.set_facecolors(colset)

    def build_verts(self, radius, height, n_pts):
        """
        Input
            radius, radius of cylinder
            height, height of cylinder
            n_pts, number of points used to describe rim of cylinder
        Returns
            verts, [n_triangles, 3_points, 3_coordinates]
        """

        theta = np.linspace(0, 2*np.pi, n_pts, endpoint=False)
        delta_theta = (theta[1]-theta[0])/2

        # Points around the bottom rim, top rim, bottom center, and top center.
        bot_pts = np.zeros((3, n_pts))
        bot_pts[0,:] = radius * np.cos(theta)
        bot_pts[1,:] = radius * np.sin(theta)
        bot_pts[2,:] = np.full(n_pts, -height/2)
        top_pts = np.zeros((3, n_pts))
        top_pts[0,:] = radius * np.cos(theta + delta_theta)
        top_pts[1,:] = radius * np.sin(theta + delta_theta)
        top_pts[2,:] = np.full(n_pts, height/2)
        bot_center = np.array([[0], [0], [-height/2]])
        top_center = np.array([[0], [0],  [height/2]])
        pts = np.concatenate((bot_pts, top_pts, bot_center, top_center), axis=1)

        # Triangle indices for the shell.
        up_triangles = np.stack((
            np.arange(0, n_pts, dtype=int),
            np.arange(1, n_pts+1, dtype=int),
            np.arange(n_pts+0, n_pts+n_pts, dtype=int)))
        up_triangles[1,-1] = 0
        down_triangles = np.stack((
            np.arange(0, n_pts, dtype=int),
            np.arange(n_pts, n_pts+n_pts, dtype=int),
            np.arange(n_pts-1, n_pts+n_pts-1, dtype=int)))
        down_triangles[2,0] = n_pts+n_pts-1
        shell_triangles = np.concatenate((up_triangles, down_triangles), axis=1)

        # Triangle indices for the bottom.
        bot_triangles = np.stack((
            np.arange(0, n_pts, dtype=int),
            np.arange(1, n_pts+1, dtype=int),
            np.full(n_pts, 2*n_pts, dtype=int)))
        bot_triangles[1,-1] = 0
        top_triangles = np.stack((
            np.arange(n_pts+0, n_pts+n_pts, dtype=int),
            np.arange(n_pts+1, n_pts+n_pts+1, dtype=int),
            np.full(n_pts, 2*n_pts+1, dtype=int)))
        top_triangles[1,-1] = n_pts

        all_triangles = np.concatenate((shell_triangles, bot_triangles, top_triangles), axis=1)

        xt = pts[0,:][all_triangles.T]
        yt = pts[1,:][all_triangles.T]
        zt = pts[2,:][all_triangles.T]
        verts = np.stack((xt, yt, zt), axis=-1)

        return verts

class Quadrotor():

    def __init__(self, ax,
        arm_length=0.125, rotor_radius=0.08, n_rotors=4,
        shade=True, color=None, wind=True, wind_scale_factor=5):

        self.ax = ax
        self.wind_bool = wind
        self.wind_scale_factor = wind_scale_factor

        # Apply same color to all rotor objects.
        if color is None:
            color = self.ax._get_lines.get_next_color()
        self.color = np.array(mcolors.to_rgba(color))

        # Precompute positions and rotations in the reference configuration.
        theta = np.linspace(0, 2*np.pi, n_rotors, endpoint=False)
        theta = theta + np.mean(theta[:2])
        self.rotor_position = np.zeros((3, n_rotors))
        self.rotor_position[0,:] = arm_length*np.cos(theta)
        self.rotor_position[1,:] = arm_length*np.sin(theta)

        # Instantiate.
        self.rotors = [Cylinder(ax,
                                rotor_radius,
                                0.1*rotor_radius,
                                shade=shade,
                                color=color) for _ in range(n_rotors)]
        artists = [r.artists for r in self.rotors]
        if self.wind_bool:
            self.wind_vector = [self.ax.quiver(0,0,0,0,0,0, color='k')]
            artists.append(self.wind_vector)
        self.artists = tuple(itertools.chain.from_iterable(artists))
                             
        self.transform(np.zeros((3,)), np.identity(3), np.zeros((3,)))

    def transform(self, position, rotation, wind=np.array([1,0,0])):
        position.shape = (3,1)
        wind.shape = (3,1)
        for (r, pos) in zip(self.rotors, self.rotor_position.T):
            pos.shape = (3,1)
            r.transform(np.matmul(rotation,pos)+position, rotation)
        if self.wind_bool:
            self.wind_vector[0].remove()
            self.wind_vector = [self.ax.quiver(position[0], position[1], position[2], wind[0]/self.wind_scale_factor, wind[1]/self.wind_scale_factor, wind[2]/self.wind_scale_factor, color='r', linewidth=1.5)]