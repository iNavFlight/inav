# Modules

RotorPy is a simple dynamics simulator for teaching aerial robotics planning and control and preliminary probing of interesting research questions. It is organized into a set of modules: 

* **Vehicles** - Contains the continuous-time dynamics for the UAV of interest. 
* **Sensors** - Transforms ground truth vehicle state into noisy and biased measurements. 
* **Worlds** - Contains `json` map files for constructing world bounds and obstacles. 
* **Controllers** - Stabilizes the vehicle to a trajectory. 
* **Trajectories** - Dictates the desired motion of the vehicle in time. 
* **Wind** - Custom spatial and/or temporal vector fields that add external disturbances to the vehicle.  
* **Estimators** - Use measurements from sensors and a model of the dynamics to filter or estimate beliefs of states or parameters. 

These modules come together in `environment.py` which contains the necessary functions for instantiation, execution, and plotting/saving. The simulation is actually carried out in `simulate.py` which will step the vehicle forward in time, governed by the dynamics, trajectory, controller, and sensor measurements. 

# License Note

The package rotorpy itself is released under the MIT License. Contributions of improvements, bug fixes, and documentation are welcome. Course instructors would enjoy hearing if you use RotorPy as a template for other projects.

Please note that RotorPy (and its open source license) does not include course materials such as assignments, solutions, and server integrations.
