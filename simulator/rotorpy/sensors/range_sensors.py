import numpy as np
from scipy.spatial.transform import Rotation
import copy

from rotorpy.utils.occupancy_map import OccupancyMap

def edges_from_extents_2d(extents):
    """
    Return the 4 edges of the rectangle given by the extents.
    Inputs:
        extents: the array (xmin, xmax, ymin, ymax) that describe the rectangle of interest.
    Outputs:
        edges: a list of vertices [[(a0.x, a0.y), (b0.x, b0.y)]
                                   [(a1.x, a1.y), (b1.x, b1.y)]
                                   .....
                                  ]
               that describe the edges of the rectangle give by extents.

        (xmin,ymax)--------(2)---------(xmax,ymax)
           |                           |
           |                          (1)
          (3)                          |
           |                           |
        (xmin,ymin)--------(0)---------(xmax,ymin)


    """
    xmin = extents[0]
    xmax = extents[1]
    ymin = extents[2]
    ymax = extents[3]

    edges = [(xmin, ymin, xmax, ymin), 
             (xmax, ymin, xmax, ymax), 
             (xmax, ymax, xmin, ymax), 
             (xmin, ymax, xmin, ymin)]

    return edges

class TwoDRangeSensor():
    """
    The 2D Range sensor will provide distance measurements around the UAV on the XY plane (top-down). 

    The sensor is placed at the CoM of the UAV, and casts rays from the CoM outward toward objects. The 
    distance between objects and the UAV, as measured by the i'th ray, is given in the i'th row of the 
    sensor output. The distance measurement is clipped by Dmin and Dmax, bounds that are specified by 
    the user (with defaults of course). The distance measurement can also be optionally corrupted by 
    white noise using the specified noise density term. The angular resolution specified will determine
    the number of rays casted. 
    
    """

    def __init__(self, world, sampling_rate, 
                 map_resolution = 1,        # The resolution of the occupancy grid used for generating measurements, in meters. 
                 angular_resolution=10,     # The angular resolution, measured in degrees. 
                 angular_fov=360,           # Angular field of view, this determines the width of the region with which rays are cast, relative to the sensor heading, degrees.
                 Dmin=0.025,                 # The minimum distance measurable by the range sensor, m.
                 Dmax=100,                  # The maximum distance measurable by the range sensor, m.
                 noise_density=0.0,         # The noise density of the white noise added to the range sensor, [m / sqrt(Hz)].
                 fixed_heading=True,        # If true, the heading of the sensor will be world-fixed; if false, the rays will be cast relative to the robot's heading
                 ):


        self.world = world
        self.occ_map = OccupancyMap(world=self.world, resolution=(map_resolution, map_resolution, map_resolution), margin=0)

        self.rate_scale = np.sqrt(sampling_rate/2)
        self.angular_resolution = angular_resolution
        self.angular_fov = angular_fov
        self.Dmin = Dmin
        self.Dmax = Dmax
        self.sensor_variance = noise_density
        self.fixed_heading = fixed_heading

        # First, construct an array of ray directions that we will iterate over. 
        self.ray_angles = np.arange(-self.angular_fov/2, self.angular_fov/2+self.angular_resolution, step=self.angular_resolution)  # Centered at 0
        self.N_rays = len(self.ray_angles)   # save the number of rays for later looping
        self.set_ray_vectors(heading=0)

        # Next save relevant quantities for each physical block in the map
        self.world_edges = []
        world_slopes = []
        for block in self.world.world['blocks']:
            extents = block['extents']                  # Get the extents of the block
            edges = edges_from_extents_2d(extents)      # Extract the four edges of each block 
            self.world_edges.append(edges)

        return

    def set_ray_vectors(self, heading=0):
        """
        Set the ray vectors array based on the current heading. 
        Inputs:
            heading: the current heading angle in degrees.
        """

        rotated_ray_angles = self.ray_angles + heading

        x_rays = np.cos(rotated_ray_angles*np.pi/180).reshape(-1,1)
        y_rays = np.sin(rotated_ray_angles*np.pi/180).reshape(-1,1)

        self.ray_vectors = np.hstack((x_rays, y_rays))

        return

    def cast_rays(self, x, y):
        """
        Cast rays from (x,y) and return the distances of the point (x,y) to the nearest obstacles in the map.
        Raycasting is implemented using the algorithm described here: https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection

        Inputs:
            x: the x position of the point of interest
            y: the y position of the point of interest
        Outputs:
            d: an array of distances that is of shape (self.N_rays,) 
        """
        d = np.zeros((self.N_rays,))

        # For ray casting we'll have to use (x,y) as one of our points for all rays. Do this now to be more efficient
        x3 = x
        y3 = y

        # Loop over each object
        distances = np.ones((self.N_rays,))*1e5   # Begin with 
        for object_edges in self.world_edges:
            # Object edges is a list of tuples, each tuple describing one of the four edges

            # Check if the object is within the field of view of the sensor
            xmin = object_edges[0][0]
            xmax = object_edges[1][0]
            ymin = object_edges[0][1]
            ymax = object_edges[2][1]

            if (xmin > x + self.Dmax) or (xmax < x - self.Dmax) or (ymin > y + self.Dmax) or (ymax < y - self.Dmax):  
                # Don't check if the object is outside the field of view
                continue

            for edge in object_edges:
                # First collect the coordinates for point 1 and 2 corresponding to this edge
                x1 = edge[0]
                y1 = edge[1]
                x2 = edge[2]
                y2 = edge[3]

                # Using self.ray_vectors and the current position (x,y), create the ray's other point (x4,y4)
                # The rays are of length Dmax. 
                x4 = x + self.Dmax*self.ray_vectors[:,0]  # This gives us an array of x coordinates for each ray
                y4 = y + self.Dmax*self.ray_vectors[:,1]  # This gives us an array of y coordinates for each ray

                # Compute the denominator of the t-u test, which will be used to check for intersections
                den = (x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4)

                # Assign nan to any instances where the denominator equals 0, because this indicates that the ray and boundary are parallel
                den[den == 0] = np.nan

                # Compute t
                t = ((x1 - x3)*(y3 - y4) - (y1 - y3)*(x3 - x4))/den

                # Compute u
                u = ((x1 - x3)*(y1 - y2) - (y1 - y3)*(x1 - x2))/den

                # Now do the check to see if these lines intersect and compute the intersection point
                intersected = (t > 0)*(t < 1)*(u > 0)*1.
                intersected[intersected==0] = np.nan

                # Compute the distance away from the point using u and (x,y)
                dx = u*(x4-x3)
                dy = u*(y4-y3)

                # Now that we have (x_int, y_int), compute the distance away from (x,y) for those that are intersected
                object_distances = np.sqrt(dx**2 + dy**2)*intersected

                # If any of these distances are less than the current distance saved in distances, rewrite that measurement
                distances[object_distances < distances] = object_distances[object_distances < distances]

        return distances

    def measurement(self, state, with_noise=True):
        """
        Computes and returns the measurement at the current time step. 
        Inputs:
            state, a dict describing the state with keys
                    x, position, m, shape=(3,)
                    v, linear velocity, m/s, shape=(3,)
                    q, quaternion [i,j,k,w], shape=(4,)
                    w, angular velocity (in LOCAL frame!), rad/s, shape=(3,)
            with_noise, a boolean to indicate if noise is added
        Outputs:
            ranges, an array that is 
        """

        pos = state['x']

        # First update the ray angles and recompute the ray vectors if the heading should follow the robot
        if not self.fixed_heading:
            orientation = Rotation.from_quat(state['q']).as_euler('zyx', degrees=True)
            heading = orientation[0]        # Extract the yaw angle in degrees of the robot.
            self.set_ray_vectors(heading=heading)  # This sets self.ray_vectors appropriately

        # Cast the rays and compute the ranges for each ray
        self.ranges = np.clip(self.cast_rays(pos[0], pos[1]), self.Dmin, self.Dmax)
        if with_noise:
            self.ranges += self.rate_scale * np.random.normal(scale=np.abs(self.sensor_variance), size=self.N_rays)

        return self.ranges

if __name__=="__main__":

    import matplotlib.pyplot as plt
    from rotorpy.utils.plotter import plot_map
    import matplotlib.colors as mcolors
    from matplotlib.patches import Rectangle
    import os

    fig_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'data_out')

    random_color = np.random.uniform(low=0, high=1, size=(100, 3))

    def plot_range(axes, world, pos, ranges, range_sensor, detected_edges=None):
        axes[0].plot(pos[0], pos[1], 'ro', zorder=10, label="Robot Position")
        if detected_edges is None:
            axes[0].plot((pos[0:2] + ranges[:,np.newaxis]*range_sensor.ray_vectors)[:,0], (pos[0:2] + ranges[:,np.newaxis]*range_sensor.ray_vectors)[:,1], 'ko', markersize=3, label="Ray Intersection Points")
        else:
            for (i,edge) in enumerate(detected_edges):
                axes[0].plot((pos[0:2] + ranges[edge,np.newaxis]*range_sensor.ray_vectors[edge])[:,0], (pos[0:2] + ranges[edge,np.newaxis]*range_sensor.ray_vectors[edge])[:,1], color=random_color[i], marker='o', markersize=3, linestyle='none', label="Ray Intersection Points")
        line_objects = []
        for i in range(range_sensor.N_rays):
            xvals = [pos[0], (pos[0:2] + ranges[:,np.newaxis]*range_sensor.ray_vectors)[i,0]]
            yvals = [pos[1], (pos[0:2] + ranges[:,np.newaxis]*range_sensor.ray_vectors)[i,1]]
            line = axes[0].plot(xvals, yvals, 'k-', linewidth=0.5, alpha=0.5)

        if detected_edges is None:
            axes[1].plot(range_sensor.ray_angles, range_sensor.ranges, 'r.', linewidth=1.5)
        else:
            axes[1].plot(range_sensor.ray_angles, range_sensor.ranges, 'k.', linewidth=1.5, alpha=0.1)
            for (i,edge) in enumerate(detected_edges):
                axes[1].plot(range_sensor.ray_angles[edge], range_sensor.ranges[edge], color=random_color[i], linestyle='none', marker='.')
        axes[1].plot([-180, 180], [range_sensor.Dmin, range_sensor.Dmin], 'k--', linewidth=1)
        axes[1].plot([-180, 180], [range_sensor.Dmax, range_sensor.Dmax], 'k--', linewidth=1)

        return

    # Choose a map
    # Load the map from the world directory
    from rotorpy.world import World

    map_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'worlds')
    maps = [m for m in os.listdir(map_dir) if '.json' in m]
    world = World.grid_forest(n_rows=10, n_cols=10, width=1, height=10, spacing=5)

    # Sensor intrinsics
    angular_fov = 360
    angular_resolution = 1
    fixed_heading = True
    noise_density = 0.005

    # Create sensor
    range_sensor = TwoDRangeSensor(world, sampling_rate=100, angular_fov=angular_fov, angular_resolution=angular_resolution, fixed_heading=fixed_heading, noise_density=noise_density)

    (xmin, xmax, ymin, ymax, zmin, zmax) = world.world['bounds']['extents']

    pos0 = np.array([0.5*(xmax+xmin), 0.5*(ymax+ymin), 0.5*(zmax+zmin)])
    state = {'x': pos0, 'q': np.array([0, 0, 0.3826834, 0.9238795])}
    ranges = range_sensor.measurement(state)

    # Plotting
    (fig, axes) = plt.subplots(nrows=1, ncols=2, num="Ray Intersections Test")

    axes[1].set_xlabel("Ray Angle (deg)")
    axes[1].set_ylabel("Range (m)")
    axes[1].set_xlim([-190, 190])

    plot_map(axes[0], world.world)
    plot_range(axes, world, pos0, ranges, range_sensor)

    plt.show()