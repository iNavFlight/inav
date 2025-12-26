# Sensor Module

Sensors are implemented as ways to convert the current ground truth vehicle state (and possibly obstacles and/or control commands) to measurements that reflects information that might be available in the real world. 

The most straightforward examples of sensors are in `imu.py` and `external_mocap.py` which mimic an inertial measurement unit and external motion capture system (e.g. Vicon), respectively. 

The IMU sensor can be arbitrarily placed and oriented with respect to the body axes. Bias and noise intensity can be specified for each sensor axis, in addition to bias drift (diffusion). 

The external motion capture sensor provides noisy measurements of pose and twist. The sensor is placed at the center of mass aligned with the body axes. There is also a parameter that enables "artifacts", which are the consequence of numerical differentiation or glitches in the system causing spikes in the measurements. 

See each sensor file for more information. 