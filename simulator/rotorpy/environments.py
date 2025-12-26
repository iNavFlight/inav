import numpy as np
from scipy.spatial.transform import Rotation
import matplotlib.pyplot as plt
import time as clk

from rotorpy.simulate import simulate
from rotorpy.utils.plotter import *
from rotorpy.world import World

from rotorpy.utils.postprocessing import unpack_sim_data

import os


class Environment():
    """
    Sandbox represents an instance of the simulation environment containing a unique vehicle, 
    controller, trajectory generator, wind profile. 

    """

    def __init__(self, vehicle,                 # vehicle object, must be specified. 
                       controller,              # controller object, must be specified.
                       trajectory,              # trajectory object, must be specified.
                       wind_profile = None,     # wind profile object, if none is supplied it will choose no wind. 
                       imu = None,              # imu sensor object, if none is supplied it will choose a default IMU sensor.
                       mocap = None,            # mocap sensor object, if none is supplied it will choose a default mocap.
                       world        = None,     # The world object
                       estimator    = None,     # estimator object
                       sim_rate     = 100,      # The update frequency of the simulator in Hz
                       safety_margin = 0.25,    # The radius of the safety region around the robot. 
                       ):

        self.sim_rate = sim_rate
        self.vehicle = vehicle
        self.controller = controller
        self.trajectory = trajectory

        self.safety_margin = safety_margin

        if world is None:
            # If no world is specified, assume that it means that the intended world is free space.
            wbound = 3 
            self.world = World.empty((-wbound, wbound, -wbound, 
                                       wbound, -wbound, wbound))
        else:
            self.world = world

        if wind_profile is None:
            # If wind is not specified, default to no wind. 
            from rotorpy.wind.default_winds import NoWind
            self.wind_profile = NoWind()
        else:
            self.wind_profile = wind_profile
        
        if imu is None:
            # In the event of specified IMU, default to 0 bias with white noise with default parameters as specified below. 
            from rotorpy.sensors.imu import Imu
            self.imu = Imu(p_BS = np.zeros(3,),
                           R_BS = np.eye(3),
                           sampling_rate=sim_rate)
        else:
            self.imu = imu

        if mocap is None:
            # If no mocap is specified, set a default mocap. 
            # Default motion capture properties. Pretty much made up based on qualitative comparison with real data from Vicon. 
            mocap_params = {'pos_noise_density': 0.0005*np.ones((3,)),  # noise density for position 
                    'vel_noise_density': 0.0010*np.ones((3,)),          # noise density for velocity
                    'att_noise_density': 0.0005*np.ones((3,)),          # noise density for attitude 
                    'rate_noise_density': 0.0005*np.ones((3,)),         # noise density for body rates
                    'vel_artifact_max': 5,                              # maximum magnitude of the artifact in velocity (m/s)
                    'vel_artifact_prob': 0.001,                         # probability that an artifact will occur for a given velocity measurement
                    'rate_artifact_max': 1,                             # maximum magnitude of the artifact in body rates (rad/s)
                    'rate_artifact_prob': 0.0002                        # probability that an artifact will occur for a given rate measurement
            }
            from rotorpy.sensors.external_mocap import MotionCapture
            self.mocap = MotionCapture(sampling_rate=sim_rate, mocap_params=mocap_params, with_artifacts=False)
        else:
            self.mocap = mocap

        if estimator is None:
            # In the likely case where an estimator is not supplied, default to the null state estimator. 
            from rotorpy.estimators.nullestimator import NullEstimator
            self.estimator = NullEstimator()
        else:
            self.estimator = estimator

        return 

    def run(self,   t_final      = 10,       # The maximum duration of the environment in seconds
                    use_mocap    = False,    # boolean determines if the controller should use
                    terminate    = False,
                    plot            = False,    # Boolean: plots the vehicle states and commands   
                    plot_mocap      = True,     # Boolean: plots the motion capture pose and twist measurements
                    plot_estimator  = True,     # Boolean: plots the estimator filter states and covariance diagonal elements
                    plot_imu        = True,     # Boolean: plots the IMU measurements
                    animate_bool    = False,    # Boolean: determines if the animation of vehicle state will play. 
                    animate_wind    = False,    # Boolean: determines if the animation will include a wind vector.
                    verbose         = False,    # Boolean: will print statistics regarding the simulation. 
                    fname   = None      # Filename is specified if you want to save the animation. Default location is the home directory. 
                    ):

        """
        Run the simulator
        """

        self.t_step = 1/self.sim_rate
        self.t_final = t_final
        self.t_final = t_final
        self.terminate = terminate
        self.use_mocap = use_mocap

        start_time = clk.time()
        (time, state, control, flat, imu_measurements, imu_gt, mocap_measurements, state_estimate, exit) = simulate(self.world,
                                                                                                                    self.vehicle.initial_state,
                                                                                                                    self.vehicle,
                                                                                                                    self.controller,
                                                                                                                    self.trajectory,
                                                                                                                    self.wind_profile,
                                                                                                                    self.imu,
                                                                                                                    self.mocap,
                                                                                                                    self.estimator,
                                                                                                                    self.t_final,
                                                                                                                    self.t_step,
                                                                                                                    self.safety_margin,
                                                                                                                    self.use_mocap,
                                                                                                                    terminate=self.terminate,
                                                                                                                    )
        if verbose:
            # Print relevant statistics or simulator status indicators here
            print('-------------------RESULTS-----------------------')
            print('SIM TIME -- %3.2f seconds | WALL TIME -- %3.2f seconds' % (min(self.t_final, time[-1]) , (clk.time()-start_time)))
            print('EXIT STATUS -- '+exit.value)

        self.result = dict(time=time, state=state, control=control, flat=flat, imu_measurements=imu_measurements, imu_gt=imu_gt, mocap_measurements=mocap_measurements, state_estimate=state_estimate, exit=exit)

        visualizer = Plotter(self.result, self.world)

        # Remove gif or mp4 in filename if it exists (the respective functions will add appropriate extensions)
        if fname is not None:
            if ".gif" in fname:
                fname = fname.replace(".gif", "")
            if ".mp4" in fname:
                fname = fname.replace(".mp4", "")
                
        if animate_bool:
            # Do animation here
            visualizer.animate_results(fname=fname, animate_wind=animate_wind)
        if plot:
            # Do plotting here
            visualizer.plot_results(fname=fname,plot_mocap=plot_mocap,plot_estimator=plot_estimator,plot_imu=plot_imu)
            if not animate_bool:
                plt.show()

        return self.result
    
    def save_to_csv(self, savepath=None):
        """
        Save the simulation data in self.results to a file. 
        """

        if savepath is None:
            savepath = "rotorpy_simulation_results.csv"

        if self.result is None:
            print("Error: cannot save if no results have been generated! Aborting save.")
            return
        else:
            if not ".csv" in savepath:
                savepath = savepath + ".csv"
            dataframe = unpack_sim_data(self.result)
            dataframe.to_csv(savepath)


if __name__=="__main__":
    from rotorpy.vehicles.crazyflie_params import quad_params
    from rotorpy.trajectories.hover_traj import HoverTraj

    from rotorpy.vehicles.multirotor import Multirotor
    from rotorpy.controllers.quadrotor_control import SE3Control

    sim = Environment(vehicle=Multirotor(quad_params),
                      controller=SE3Control(quad_params),
                      trajectory=HoverTraj(),
                      sim_rate=100
                      )
    
    result = sim.run(t_final=1, plot=True)