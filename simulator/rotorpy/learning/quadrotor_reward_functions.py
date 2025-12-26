import numpy as np
from scipy.spatial.transform import Rotation
import torch

import gymnasium as gym
from gymnasium import spaces

import math

"""
Reward functions for quadrotor tasks. 
"""

def hover_reward(observation, action, weights={'x': 1, 'v': 0.1, 'w': 0, 'u': 1e-5}):
    """
    Rewards hovering at (0, 0, 0). It is a combination of position error, velocity error, body rates, and 
    action reward.
    """

    # Compute the distance to goal
    dist_reward = -weights['x']*np.linalg.norm(observation[0:3])

    # Compute the velocity reward
    vel_reward = -weights['v']*np.linalg.norm(observation[3:6])

    # Compute the angular rate reward
    ang_rate_reward = -weights['w']*np.linalg.norm(observation[10:13])

    # Compute the action reward
    action_reward = -weights['u']*np.linalg.norm(action)

    return dist_reward + vel_reward + action_reward + ang_rate_reward


def hover_reward_positive(observation, action, weights={'x': 1, 'v': 0.1, 'w': 0, 'u': 1e-5}):
    """
    Rewards hovering at (0, 0, 0). It is a combination of position error, velocity error, body rates, and
    action reward. It inverts the various terms to assign a positive reward for approaching hover.
    """

    # Compute the distance to goal
    dist_reward = weights['x']*1/(1+np.linalg.norm(observation[0:3]))

    # Compute the velocity reward
    vel_reward = weights['v']*1/(1+np.linalg.norm(observation[3:6]))

    # Compute the angular rate reward
    ang_rate_reward = weights['w']*1/(1+np.linalg.norm(observation[10:13]))

    # Compute the action reward
    action_reward = weights['u']*1/(1+np.linalg.norm(action))

    return dist_reward + vel_reward + action_reward + ang_rate_reward


def vec_hover_reward(observation, action, weights={'x': 1, 'v': 0.1, 'w': 0, 'u': 1e-5}):
    """
    Rewards hovering at (0, 0, 0). It is a combination of position error, velocity error, body rates, and
    action reward. Computes rewards for each environment. Adds a positive survival reward.
    """

    # Compute the distance to goal
    dist_reward = -weights['x']*np.linalg.norm(observation[...,0:3], axis=-1)

    # Compute the velocity reward
    vel_reward = -weights['v']*np.linalg.norm(observation[...,3:6], axis=-1)

    # Compute the angular rate reward
    ang_rate_reward = -weights['w']*np.linalg.norm(observation[...,10:13], axis=-1)

    # Compute the action reward
    action_reward = -weights['u']*np.linalg.norm(action, axis=-1)

    return dist_reward + vel_reward + action_reward + ang_rate_reward + 2


def vec_hover_reward_positive(observation, action, weights={'x': 1, 'v': 0.1, 'w': 0, 'u': 1e-5}):
    """
    Rewards hovering at (0, 0, 0). It is a combination of position error, velocity error, body rates, and
    action reward. Computes rewards for each environment.
    """

    # distance reward - reward getting closer to 0
    dist_reward = weights['x'] * 1/(1+np.linalg.norm(observation[...,0:3], axis=-1))

    # Compute the velocity reward
    vel_reward = weights['v'] * 1/(1+np.linalg.norm(observation[...,3:6], axis=-1))

    # Compute the angular rate reward
    ang_rate_reward = weights['w']*1/(1+np.linalg.norm(observation[...,10:13], axis=-1))

    # Compute the action reward
    action_reward = weights['u']*1/(1+np.linalg.norm(action, axis=-1))

    return dist_reward + vel_reward + action_reward + ang_rate_reward
