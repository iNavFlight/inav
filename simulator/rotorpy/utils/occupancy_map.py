import heapq
import numpy as np
from scipy.spatial import Rectangle
from scipy.spatial.transform import Rotation

from rotorpy.world import World
from rotorpy.utils import shapes


class OccupancyMap:
    def __init__(self, world=World.empty((0, 2, 0, 2, 0, 2)), resolution=(.1, .1, .1), margin=.2):
        """
        This class creates a 3D voxel occupancy map of the configuration space from a flightsim World object.
        Parameters:
            world, a flightsim World object
            resolution, the discretization of the occupancy grid in x,y,z
            margin, the inflation radius used to create the configuration space (assuming a spherical drone)
        """
        self.world = world
        self.resolution = np.array(resolution)
        self.margin = margin
        self._init_map_from_world()

    def get_local_2d_occupancy_map(self, position, region_size=(10, 10)):
        """
        Return a new occupancy map that is centered at the position of the vehicle. 
        Parameters:
            position, a numpy array indicating the (x,y,z) position of the quadrotor in the world frame.
            region_size, indicates the size of the rectangular XY region around the quadrotor. 
        """

        region_size = np.array(region_size)

        # Get the index corresponding to the current position of the quadrotor. 
        center = self.metric_to_index(position)

        # World bounds
        world_bounds = self.world.world['bounds']['extents']

        # Get the extents of the XY region, clipping to the extents of the world.
        xmin = max(position[0] - region_size[0], world_bounds[0])
        xmax = min(position[0] + region_size[0], world_bounds[1])
        ymin = max(position[1] - region_size[1], world_bounds[2])
        ymax = min(position[1] + region_size[1], world_bounds[3])

        # Now collect the portion of the map that is contained within the region.
        bounds = [xmin, xmax, ymin, ymax, position[2], position[2]]
        (inner_min_index, inner_max_index) = self._metric_block_to_index_range(bounds, outer_bound=True)

        a, b = inner_min_index, inner_max_index
        self.local_map = self.map[a[0]:(b[0]+1), a[1]:(b[1]+1), center[2]]
        
        return ((xmin, xmax), (ymin, ymax), self.local_map)

    def index_to_metric_negative_corner(self, index):
        """
        Return the metric position of the most negative corner of a voxel, given its index in the occupancy grid
        """
        return index*np.array(self.resolution) + self.origin

    def index_to_metric_center(self, index):
        """
        Return the metric position of the center of a voxel, given its index in the occupancy grid
        """
        return self.index_to_metric_negative_corner(index) + self.resolution/2.0

    def metric_to_index(self, metric):
        """
        Returns the index of the voxel containing a metric point.
        Remember that this and index_to_metric and not inverses of each other!
        If the metric point lies on a voxel boundary along some coordinate,
        the returned index is the lesser index.
        """
        return np.floor((metric - self.origin)/self.resolution).astype('int')

    def _metric_block_to_index_range(self, bounds, outer_bound=True):
        """
        A fast test that returns the closed index range intervals of voxels
        intercepting a rectangular bound. If outer_bound is true the returned
        index range is conservatively large, if outer_bound is false the index
        range is conservatively small.
        """

        # Implementation note: The original intended resolution may not be
        # exactly representable as a floating point number. For example, the
        # floating point value for "0.1" is actually bigger than 0.1. This can
        # cause surprising results on large maps. The solution used here is to
        # slightly inflate or deflate the resolution by the smallest
        # representative unit to achieve either an upper or lower bound result.
        sign = 1 if outer_bound else -1
        min_index_res = np.nextafter(self.resolution,  sign * np.inf) # Use for lower corner.
        max_index_res = np.nextafter(self.resolution, -sign * np.inf) # Use for upper corner.

        bounds = np.asarray(bounds)
        # Find minimum included index range.
        min_corner = bounds[0::2]
        min_frac_index = (min_corner - self.origin)/min_index_res
        min_index = np.floor(min_frac_index).astype('int')
        min_index[min_index == min_frac_index] -= 1
        min_index = np.maximum(0, min_index)
        # Find maximum included index range.
        max_corner = bounds[1::2]
        max_frac_index = (max_corner - self.origin)/max_index_res
        max_index = np.floor(max_frac_index).astype('int')
        max_index = np.minimum(max_index, np.asarray(self.map.shape)-1)
        return (min_index, max_index)

    def _init_map_from_world(self):
        """
        Creates the occupancy grid (self.map) as a boolean numpy array. True is
        occupied, False is unoccupied. This function is called during
        initialization of the object.
        """

        # Initialize the occupancy map, marking all free.
        bounds = self.world.world['bounds']['extents']
        voxel_dimensions_metric = []
        voxel_dimensions_indices = []
        for i in range(3):
            voxel_dimensions_metric.append(abs(bounds[1+i*2]-bounds[i*2]))
            voxel_dimensions_indices.append(int(np.ceil(voxel_dimensions_metric[i]/self.resolution[i])))
        self.map = np.zeros(voxel_dimensions_indices, dtype=bool)
        self.origin = np.array(bounds[0::2])

        # Iterate through each block obstacle.
        for block in self.world.world.get('blocks', []):
            extent = block['extents']
            block_rect = Rectangle([extent[1], extent[3], extent[5]], [extent[0], extent[2], extent[4]])
            # Get index range that is definitely occupied by this block.
            (inner_min_index, inner_max_index) = self._metric_block_to_index_range(extent, outer_bound=False)
            a, b = inner_min_index, inner_max_index
            self.map[a[0]:(b[0]+1), a[1]:(b[1]+1), a[2]:(b[2]+1)] = True
            # Get index range that is definitely not occupied by this block.
            outer_extent = extent + self.margin * np.array([-1, 1, -1, 1, -1, 1])
            (outer_min_index, outer_max_index) = self._metric_block_to_index_range(outer_extent, outer_bound=True)
            # Iterate over uncertain voxels with rect-rect distance check.
            for i in range(outer_min_index[0], outer_max_index[0]+1):
                for j in range(outer_min_index[1], outer_max_index[1]+1):
                    for k in range(outer_min_index[2], outer_max_index[2]+1):
                        # If map is not already occupied, check for collision.
                        if not self.map[i,j,k]:
                            metric_loc = self.index_to_metric_negative_corner((i,j,k))
                            voxel_rect = Rectangle(metric_loc+self.resolution, metric_loc)
                            rect_distance = voxel_rect.min_distance_rectangle(block_rect)
                            self.map[i,j,k] = rect_distance <= self.margin

    def draw_filled(self, ax):
        """
        Visualize the occupancy grid (mostly for debugging)
        Warning: may be slow with O(10^3) occupied voxels or more
        Parameters:
            ax, an Axes3D object
        """
        self.world.draw_empty_world(ax)
        it = np.nditer(self.map, flags=['multi_index'])
        while not it.finished:
            if self.map[it.multi_index] == True:
                metric_loc = self.index_to_metric_negative_corner(it.multi_index)
                xmin, ymin, zmin = metric_loc
                xmax, ymax, zmax = metric_loc + self.resolution
                c = shapes.Cuboid(ax, xmax-xmin, ymax-ymin, zmax-zmin, alpha=0.1, linewidth=1, edgecolors='k', facecolors='b')
                c.transform(position=(xmin, ymin, zmin))
            it.iternext()

    def _draw_voxel_face(self, ax, index, direction):
        # Normalized coordinates of the top face.
        face = np.array([(1,1,1), (-1,1,1), (-1,-1,1), (1,-1,1)])
        # Rotate to find normalized coordinates of target face.
        if   direction[0] != 0:
            axis = np.array([0, 1, 0]) * np.pi/2 * direction[0]
        elif direction[1] != 0:
            axis = np.array([-1, 0, 0]) * np.pi/2 * direction[1]
        elif direction[2] != 0:
            axis = np.array([1, 0, 0]) * np.pi/2 * (1-direction[2])
        face = (Rotation.from_rotvec(axis).as_matrix() @ face.T).T
        # Scale, position, and draw using Face object.
        face = 0.5 * face * np.reshape(self.resolution, (1,3))
        f = shapes.Face(ax, face, alpha=0.1, linewidth=1, edgecolors='k', facecolors='b')
        f.transform(position=(self.index_to_metric_center(index)))

    def draw_shell(self, ax):
        self.world.draw_empty_world(ax)
        it = np.nditer(self.map, flags=['multi_index'])
        while not it.finished:
            idx = it.multi_index
            if self.map[idx] == True:
                for d in [(0,0,-1), (0,0,1), (0,-1,0), (0,1,0), (-1,0,0), (1,0,0)]:
                    neigh_idx = (idx[0]+d[0], idx[1]+d[1], idx[2]+d[2])
                    neigh_exists = self.is_valid_index(neigh_idx)
                    if not neigh_exists or (neigh_exists and not self.map[neigh_idx]):
                        self._draw_voxel_face(ax, idx, d)
            it.iternext()

    def draw(self, ax):
        self.draw_shell(ax)

    def is_valid_index(self, voxel_index):
        """
        Test if a voxel index is within the map.
        Returns True if it is inside the map, False otherwise.
        """
        for i in range(3):
            if voxel_index[i] >= self.map.shape[i] or voxel_index[i] < 0:
                return False
        return True

    def is_valid_metric(self, metric):
        """
        Test if a metric point is within the world.
        Returns True if it is inside the world, False otherwise.
        """
        bounds = self.world.world['bounds']['extents']
        for i in range(3):
            if metric[i] <= bounds[i*2] or metric[i] >= bounds[i*2+1]:
                return False
        return True

    def is_occupied_index(self, voxel_index):
        """
        Test if a voxel index is occupied.
        Returns True if occupied or outside the map, False otherwise.
        """
        return (not self.is_valid_index(voxel_index)) or self.map[tuple(voxel_index)]

    def is_occupied_metric(self, voxel_metric):
        """
        Test if a metric point is within an occupied voxel.
        Returns True if occupied or outside the map, False otherwise.
        """
        ind = self.metric_to_index(voxel_metric)
        return (not self.is_valid_index(ind)) or self.is_occupied_index(ind)