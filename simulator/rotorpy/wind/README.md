# Wind Module

Currently winds are implemented as independent objects that take the current time and the vehicle's location and computes a wind vector at that location. Winds can be spatially varying, temporally varying, or both! 

In `default_winds.py` we have a collection of common gust models, including sinusoid wind signals, constant wind, and discrete step changes in wind. 

`spatial_winds.py` includes an example of a static spatial wind field, in which a column of air resembling a steady state wind tunnel can be produced. 

`dryden_winds.py` contain a collection of models that uses the [Dryden Wind Turbulence Model](https://en.wikipedia.org/wiki/Dryden_Wind_Turbulence_Model) to produce stochastic wind patterns. The model is implemented using code from the external repository: `wind-dynamics` [(link)](https://github.com/goromal/wind-dynamics).

If you would like to create your own wind module, you can do so by using the template found in `wind_template.py`. 