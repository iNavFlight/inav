"""
Wind profiles should be implemented here to be added as an argument in the 
simulate method found in simulate.py

The main structure of each object is the only thing that must remain consistent.
When creating a new wind profile, the object must have the following:

- An __init__ method. The arguments vary depending on the wind profile. 
- An update method. The arguments *must* include time and position. 

"""
import numpy as np

class WindTemplate(object):
    """
    Winds are implemented with two required methods: __init__() and update(). 
    With __init__() you can specify any parameters or constants used to specify the wind. 
    The update() method is called at each time step of the simulator. The position and time 
    of the vehicle are provided. 
    """

    def __init__(self):
        """
        Inputs: 
            Nothing
        """

    def update(self, t, position):
        """
        Given the present time and position of the vehicle in the world frame, return the
        current wind speed on all three axes. 
        
        Be careful whether or not the wind is specified in the body or world coordinates!. 
        """
        return np.array([0,0,0])