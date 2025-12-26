# Estimator Module

Estimators take measurements from sensors and an internal model of the dynamcis to provide filtered estimates of states or quantities of interest. 

Currently we provide two examples of estimators that estimate the local wind vector acting on the vehicle center of mass based on measurements from the IMU and external motion capture--one is a hand-implemented EKF and the other is a UKF that uses the `FilterPy` library for easy implementation.

Both models use a simplified model of the dynamics for their process models. The filter states are: $$\boldsymbol{x}_{filter} = [\psi, \theta, \phi, v_x, v_y, v_z, w_x, w_y, w_z]^\top $$ where $[\psi, \theta, \phi]$ are the ZYX Euler angles, $[v_x, v_y, v_z]$ are translational velocities in the body frame, and $[w_x, w_y, w_z]$ are the wind components in the body frame. Small angle approximations near hover are used to simplify the process model. The model assumes the predominant form of drag is parasitic, or quadratic with airspeed. In other words, rotor drag is neglected. These lumped drag coefficients can be identified using flight data in the absence of wind. 

The inputs to the process model are the commanded thrust from the controller, $f_c$, and the body rates measured by the external motion capture sensor, $\Omega$. 

Noisy measurements of acceleration are provided by the IMU sensor, and attitude and body velocities are measured by the external motion capture sensor. 

If you'd like to write your own estimator, whether it is for parameter or state estimation, you can reference `nullestimator.py` for a template. 