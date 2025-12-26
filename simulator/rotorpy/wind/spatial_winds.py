"""
Here is an example of a static spatial wind field. 
"""
import numpy as np

class WindTunnel(object):
    """
    This profile is a cylindrical column of air pointing at (0,0,0). Outside of this column the wind is zero. 
    """

    def __init__(self, mag=1, dir=np.array([1,0,0]), radius=1):
        """
        Inputs: 
            magnitude, a scalar representing the strength of the wind column
            direction, a vector describing the direction of the wind column
                e.g., [1,0,0] means the column will be directed in the positive x axis
            radius, the size of the column (the radius of the cylinder) in meters. 
            
        """

        self.mag = mag
        if np.linalg.norm(dir) > 1: # Check if this is a unit vector. If not, normalize it. 
            self.dir = dir/np.linalg.norm(dir)
        else:
            self.dir = dir
        self.radius = radius

    def update(self, t, position):
        """
        Given the present time and position of the vehicle in the world frame, return the
        current wind speed on all three axes. 
        """

        # We can get the perpendicular distance of the vector xi+yj+zk from the line self.dir 
        # using the cross product. This assumes that ||self.dir|| = 1 and that the column
        # passes through the origin (0,0,0). 

        if np.linalg.norm(np.cross(position, self.dir)) <= self.radius:
            # If this condition passes, we're within the column. 
            wind = self.mag*self.dir
        else:
            wind = np.array([0,0,0])

        return wind