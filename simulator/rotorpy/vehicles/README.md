# Vehicle Module

Vehicle classes are where the continuous-time dynamics are stored. These dynamics are integrated using the `step` method. The IMU requires a `statedot` method which outputs the acceleration and body rate to be transformed by the sensor. 

Besides that, integration must also be handled in `step`, which is done via `scipy.integrate.solve_ivp` [(reference)](https://docs.scipy.org/doc/scipy/reference/generated/scipy.integrate.solve_ivp.html#r179348322575-1). This is an RK45 method with adaptive step. It is recommended that other vehicles use this method for integration as well, since it preserves the integration accuracy for larger timesteps compared to forward/backward Euler. 

Currently `Multirotor` is the only vehicle supported with automatic plotting and saving of the states and controller outputs.