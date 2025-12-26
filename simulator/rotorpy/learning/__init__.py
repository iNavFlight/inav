from gymnasium.envs.registration import register

register(
     id="Quadrotor-v0",
     entry_point="rotorpy.learning.quadrotor_environments:QuadrotorEnv",
)