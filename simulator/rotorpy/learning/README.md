# Learning Module

In the learning module, we provide a bridge between RotorPy's dynamic models with aerodynamics, wind, motor dynamics, and more with popular reinforcement learning (RL) APIs to develop and compare RL algorithms and control abstractions for UAV control. 

An environment compatible with [Gymnasium](https://github.com/Farama-Foundation/Gymnasium) is implemented as `QuadrotorEnv`. In this environment, the user can provide a reward function and choose an appropriate control abstraction ranging from high level (velocity vector) all the way down to low level individual motor speed control. For higher level abstractions, there are lower level controllers that run within the dynamics to track your commands (note, tuning may be necessary here to get desired performance). 

In the current implementation, the observation space contains the entire state space of the UAV but the environment can easily be changed to suit your needs. All actions are assumed to be in the range $[-1,1]$. 