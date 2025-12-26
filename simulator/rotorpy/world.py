import json
import numpy as np

from rotorpy.utils.shapes import Cuboid
from rotorpy.utils.numpy_encoding import NumpyJSONEncoder, to_ndarray

def interp_path(path, res):
    if path.size == 3:
        # There's only one datapoint. Return the point. 
        return path.reshape(1,-1)
    else:
        cumdist = np.cumsum(np.linalg.norm(np.diff(path, axis=0),axis=1))
        if cumdist[-1] > 0:
            t = np.insert(cumdist,0,0)
            ts = np.arange(0, cumdist[-1], res)
            pts = np.empty((ts.size, 3), dtype=np.float64)
            for k in range(3):
                pts[:,k] = np.interp(ts, t, path[:,k])
        else:
            pts = path[[0],:]
        return pts

class World(object):

    def __init__(self, world_data):
        """
        Construct World object from data. Instead of using this constructor
        directly, see also class methods 'World.from_file()' for building a
        world from a saved .json file or 'World.grid_forest()' for building a
        world object of a parameterized style.

        Parameters:
            world_data, dict containing keys 'bounds' and 'blocks'
                bounds, dict containing key 'extents'
                    extents, list of [xmin, xmax, ymin, ymax, zmin, zmax]
                blocks, list of dicts containing keys 'extents' and 'color'
                    extents, list of [xmin, xmax, ymin, ymax, zmin, zmax]
                    color, color specification
        """
        self.world = world_data

    @classmethod
    def from_file(cls, filename):
        """
        Read world definition from a .json text file and return World object.

        Parameters:
            filename

        Returns:
            world, World object

        Example use:
            my_world = World.from_file('my_filename.json')
        """
        with open(filename) as file:
            return cls(to_ndarray(json.load(file)))

    def to_file(self, filename):
        """
        Write world definition to a .json text file.

        Parameters:
            filename

        Example use:
            my_word.to_file('my_filename.json')
        """
        with open(filename, 'w') as file:  # TODO check for directory to exist
            file.write(json.dumps(self.world, cls=NumpyJSONEncoder, indent=4))

    def closest_points(self, points):
        """
        For each point, return the closest occupied point in the world and the
        distance to that point. This is appropriate for computing sphere-vs-world
        collisions.

        Input
            points, (N,3)
        Returns
            closest_points, (N,3)
            closest_distances, (N,)
        """

        closest_points = np.empty_like(points)
        closest_distances = np.full(points.shape[0], np.inf)
        p = np.empty_like(points)
        for block in self.world.get('blocks', []):
            # Computation takes advantage of axes-aligned blocks. Note that
            # scipy.spatial.Rectangle can compute this distance, but wouldn't
            # return the point itself.
            r = block['extents']
            for i in range(3):
                p[:, i] = np.clip(points[:, i], r[2*i], r[2*i+1])
            d = np.linalg.norm(points-p, axis=1)
            mask = d < closest_distances
            closest_points[mask, :] = p[mask, :]
            closest_distances[mask] = d[mask]
        return (closest_points, closest_distances)

    def min_dist_boundary(self, points):
        """
        For each point, calculate the minimum distance to the boundary checking, x,y,z. A negative distance means the
        point is outside the boundary
        Input
            points, (N,3)
        Returns
            closest_distances, (N,)
        """

        # Bounds with upper limits negated [xmin, -xmax, ymin, -ymax, ...]
        test_bounds = np.array(self.world['bounds']['extents'])
        test_bounds[1::2] = -test_bounds[1::2]

        # Repeated coordinates with second entry negated [x, -x, y, -y, ...]
        test_points = np.repeat(points, 2, 1)
        test_points[:,1::2] = -test_points[:,::2]

        # Compute [x-xmin, xmax-x, y-ymin, ymax-y, z-zmin, zmax-z].
        # Minimum distance is the minimum for each point to all walls.
        distances = test_points - test_bounds
        min_distances = np.amin(distances, 1)

        return min_distances

    def path_collisions(self, path, margin):
        """
        Densely sample the path and check for collisions. Return a boolean mask
        over the samples and the sample points themselves.
        """
        pts = interp_path(path, res=0.001)
        (closest_pts, closest_dist) = self.closest_points(pts)
        collisions_blocks = closest_dist < margin
        collisions_points = self.min_dist_boundary(pts) < 0
        collisions = np.logical_or(collisions_points, collisions_blocks)
        return pts[collisions]

    def draw_empty_world(self, ax):
        """
        Draw just the world without any obstacles yet. The boundary is represented with a black line.
        Parameters:
            ax, Axes3D object
        """
        (xmin, xmax, ymin, ymax, zmin, zmax) = self.world['bounds']['extents']

        # Set axes limits all equal to approximate 'axis equal' display.
        x_width = xmax-xmin
        y_width = ymax-ymin
        z_width = zmax-zmin
        width = np.max((x_width, y_width, z_width))
        ax.set_xlim((xmin, xmin+width))
        ax.set_ylim((ymin, ymin+width))
        ax.set_zlim((zmin, zmin+width))
        ax.set_xlabel('x')
        ax.set_ylabel('y')
        ax.set_zlabel('z')
        c = Cuboid(ax, xmax - xmin, ymax - ymin, zmax - zmin, alpha=0.01, linewidth=1, edgecolors='k')
        c.transform(position=(xmin, ymin, zmin))
        return list(c.artists)

    def draw(self, ax, alpha=None, edgecolor=None, facecolor=None):
        """
        Draw world onto existing Axes3D axes and return artists corresponding to the
        blocks.

        Parameters:
            ax, Axes3D object

        Returns:
            block_artists, list of Artists associated with blocks

        Example use:
            my_world.draw(ax)
        """
        bounds_artists = self.draw_empty_world(ax)

        if alpha is None:
            alpha = 0.7
        
        if edgecolor is None:
            edgecolor = 'k'

        block_artists = []
        for b in self.world.get('blocks', []):
            (xmin, xmax, ymin, ymax, zmin, zmax) = b['extents']
            if facecolor is None:
                fc = b.get('color', None)
            else:
                fc = facecolor
            c = Cuboid(ax, xmax-xmin, ymax-ymin, zmax-zmin, alpha=alpha, linewidth=1, edgecolors=edgecolor, facecolors=fc)
            c.transform(position=(xmin, ymin, zmin))
            block_artists.extend(c.artists)
        return bounds_artists + block_artists

    def draw_line(self, ax, points, color=None, linewidth=2):
        path_length = np.sum(np.linalg.norm(np.diff(points, axis=0),axis=1))
        pts = interp_path(points, res=path_length/1000)
        # The scatter object is assigned a single z-order value. Split for better occlusion rendering.
        for p in np.array_split(pts, 20):
            ax.scatter(p[:,0], p[:,1], p[:,2], s=linewidth**2, c=color, edgecolors='none', depthshade=False)

    def draw_points(self, ax, points, color=None, markersize=4):
        # The scatter object is assigned a single z-order value. Split for better occlusion rendering.
        for p in np.array_split(points, 20):
            ax.scatter(p[:,0], p[:,1], p[:,2], s=markersize**2, c=color, edgecolors='none', depthshade=False)

    # The follow class methods are convenience functions for building different
    # kinds of parametric worlds.

    @classmethod
    def empty(cls, extents):
        """
        Return World object for bounded empty space.

        Parameters:
            extents, tuple of (xmin, xmax, ymin, ymax, zmin, zmax)

        Returns:
            world, World object

        Example use:
            my_world = World.empty((xmin, xmax, ymin, ymax, zmin, zmax))
        """
        bounds = {'extents': extents}
        blocks = []
        world_data = {'bounds': bounds, 'blocks': blocks}
        return cls(world_data)

    @classmethod
    def grid_forest(cls, n_rows, n_cols, width, height, spacing):
        """
        Return World object describing a grid forest world parameterized by
        arguments. The boundary extents fit tightly to the included trees.

        Parameters:
            n_rows, rows of trees stacked in the y-direction
            n_cols, columns of trees stacked in the x-direction
            width, weight of square cross section trees
            height, height of trees
            spacing, spacing between centers of rows and columns

        Returns:
            world, World object

        Example use:
            my_world = World.grid_forest(n_rows=4, n_cols=3, width=0.5, height=3.0, spacing=2.0)
        """

        # Bounds are outer boundary for world, which are an implicit obstacle.
        x_max = (n_cols-1)*spacing + width
        y_max = (n_rows-1)*spacing + width
        bounds = {'extents': [0, x_max, 0, y_max, 0, height]}

        # Blocks are obstacles in the environment.
        x_root = spacing * np.arange(n_cols)
        y_root = spacing * np.arange(n_rows)
        blocks = []
        for x in x_root:
            for y in y_root:
                blocks.append({'extents': [x, x+width, y, y+width, 0, height], 'color': [1, 0, 0]})

        world_data = {'bounds': bounds, 'blocks': blocks}
        return cls(world_data)

    @classmethod
    def random_forest(cls, world_dims, tree_width, tree_height, num_trees):
        """
        Return World object describing a random forest world parameterized by
        arguments.

        Parameters:
            world_dims, a tuple of (xmax, ymax, zmax). xmin,ymin, and zmin are set to 0.
            tree_width, weight of square cross section trees
            tree_height, height of trees
            num_trees, number of trees

        Returns:
            world, World object
        """

        # Bounds are outer boundary for world, which are an implicit obstacle.
        bounds = {'extents': [0, world_dims[0], 0, world_dims[1], 0, world_dims[2]]}

        # Blocks are obstacles in the environment.
        xs = np.random.uniform(0, world_dims[0], num_trees)
        ys = np.random.uniform(0, world_dims[1], num_trees)
        pts = np.stack((xs, ys), axis=-1) # min corner location of trees
        w, h = tree_width, tree_height
        blocks = []
        for pt in pts:
            extents = list(np.round([pt[0], pt[0]+w, pt[1], pt[1]+w, 0, h], 2))
            blocks.append({'extents': extents, 'color': [1, 0, 0]})

        world_data = {'bounds': bounds, 'blocks': blocks}
        return cls(world_data)


if __name__ == '__main__':
    import argparse
    from pathlib import Path
    import matplotlib.pyplot as plt

    parser = argparse.ArgumentParser(description='Display a map file in a Matplotlib window.')
    parser.add_argument('filename', help="Filename for map file json.")
    p = parser.parse_args()

    file = Path(p.filename)
    world = World.from_file(file)

    fig = plt.figure(f"{file.name}")
    ax = fig.add_subplot(projection='3d')
    world.draw(ax)

    plt.show()
