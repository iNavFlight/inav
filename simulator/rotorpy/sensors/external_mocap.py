import numpy as np
from scipy.spatial.transform import Rotation
import copy

def hat_map(s):
        """
        Given vector s in R^3, return associate skew symmetric matrix S in R^3x3
        """
        return np.array([[    0, -s[2],  s[1]],
                         [ s[2],     0, -s[0]],
                         [-s[1],  s[0],     0]])

class MotionCapture():
    """
    The external motion capture is able to provide pose and twist measurements of the vehicle. 
    Given the current ground truth state of the vehicle, it will output noisy measurements of the 
    pose and twist. Artifacts can be introduced 
    """
    def __init__(self, sampling_rate, 
                 mocap_params={'pos_noise_density': 0.0005*np.ones((3,)),  # noise density for position 
                                                    'vel_noise_density': 0.005*np.ones((3,)),          # noise density for velocity
                                                    'att_noise_density': 0.0005*np.ones((3,)),          # noise density for attitude 
                                                    'rate_noise_density': 0.0005*np.ones((3,)),         # noise density for body rates
                                                    'vel_artifact_max': 5,                              # maximum magnitude of the artifact in velocity (m/s)
                                                    'vel_artifact_prob': 0.001,                         # probability that an artifact will occur for a given velocity measurement
                                                    'rate_artifact_max': 1,                             # maximum magnitude of the artifact in body rates (rad/s)
                                                    'rate_artifact_prob': 0.0002                        # probability that an artifact will occur for a given rate measurement
                                                    }, 
                with_artifacts=False):
        """
        Parameters: 
            sampling_rate, Hz, the rate at which this sensor is being sampled. Used for computing the noise.
            mocap_params, a dict with keys
                pos_noise_density, position noise density, [m/sqrt(Hz)] 
                vel_noise_density, velocity noise density, [m/s / sqrt(Hz)]
                att_noise_density, attitude noise density, [rad / sqrt(Hz)]
                rate_noise_density, attitude rate noise density, [rad/s /sqrt(Hz)]
                vel_artifact_prob, probability that a spike will occur for a given velocity measurement
                vel_artifact_max, the maximum magnitude of the artifact spike. [m/s]
                rate_artifact_prob, probability that a spike will occur for a given body rate measurement
                rate_artifact_max, the maximum magnitude of hte artifact spike. [rad/s]
        """

        self.rate_scale = np.sqrt(sampling_rate/2)

        # Noise densities
        self.pos_density = mocap_params['pos_noise_density']
        self.vel_density = mocap_params['vel_noise_density']
        self.att_density = mocap_params['att_noise_density']
        self.rate_density = mocap_params['rate_noise_density']

        # Artifacts
        self.vel_artifact_prob = mocap_params['vel_artifact_prob']
        self.vel_artifact_max = mocap_params['vel_artifact_max']
        self.rate_artifact_prob = mocap_params['rate_artifact_prob']
        self.rate_artifact_max = mocap_params['rate_artifact_max']

        self.initialized = True
        self.with_artifacts = with_artifacts

    def measurement(self, state, with_noise=False, with_artifacts=False):
        """
        Computes and returns the sensor measurement at a time step. 
        Inputs:
            state, a dict describing the state with keys
                    x, position, m, shape=(3,)
                    v, linear velocity, m/s, shape=(3,)
                    q, quaternion [i,j,k,w], shape=(4,)
                    w, angular velocity (in LOCAL frame!), rad/s, shape=(3,)
            with_noise, a boolean to indicate if noise is added
            with_artifacts, a boolean to indicate if artifacts are added. 
                    Artifacts are added to the velocity and angular rates, and are due 
                    to the numerical differentiation scheme used by motion capture systems. 
                    They will appear as random spikes in the data. 
        Outputs:
            observation, a dictionary with keys
                    x_m, noisy position measurement, m, shape=(3,)
                    v_m, noisy linear velocity, m/s, shape=(3,)
                    q_m, noisy quaternion [i,j,k,w], shape=(4,)
                    w_m, noisy angular velocity (in LOCAL frame!), rad/s, shape=(3,)
        """
        x_measured = copy.deepcopy(state['x']).astype(float)
        v_measured = copy.deepcopy(state['v']).astype(float)
        q_measured = Rotation.from_quat(copy.deepcopy(state['q']))
        w_measured = copy.deepcopy(state['w']).astype(float)

        if with_noise:
            # Add noise to the measurements based on the provided measurement noise.
            x_measured += self.rate_scale * np.random.normal(scale=np.abs(self.pos_density))
            v_measured += self.rate_scale * np.random.normal(scale=np.abs(self.vel_density))
            w_measured += self.rate_scale * np.random.normal(scale=np.abs(self.rate_density))

            # Noise has to be treated differently with quaternions... 
            # Following https://www.iri.upc.edu/people/jsola/JoanSola/objectes/notes/kinematics.pdf  pg 43
            # First, let's produce a perturbation vector in R3
            delta_phi = self.rate_scale*np.random.normal(scale=np.abs(self.att_density))

            # Now convert that to a rotation matrix
            delta_rotation = Rotation.from_matrix(np.eye(3) + hat_map(delta_phi))

            # Now apply that rotation to the quaternion
            q_measured = (q_measured * delta_rotation).as_quat()
        else:
            q_measured = q_measured.as_quat()

        if with_artifacts:
            # If including artifacts, first roll the dice on whether or not a spike should occur for each measurement: 
            vel_spike_bool = np.random.choice([0,1], p=[1-self.vel_artifact_prob, self.vel_artifact_prob])
            rate_spike_bool = np.random.choice([0,1], p=[1-self.rate_artifact_prob, self.rate_artifact_prob])

            # Choose the axis that the spike will occur on
            vel_axis = np.random.choice([0,1,2])
            rate_axis = np.random.choice([0,1,2])

            # Choose the sign of the spike
            vel_sign = np.random.choice([-1,1])
            rate_sign = np.random.choice([-1,1])

            if vel_spike_bool:
                v_measured[vel_axis] += vel_sign*np.random.uniform(low=0, high=self.vel_artifact_max)
            if rate_spike_bool:
                w_measured[rate_axis] += rate_sign*np.random.uniform(low=0, high=self.rate_artifact_max)

        return {'x': x_measured, 'q': q_measured, 'v': v_measured, 'w': w_measured}

if __name__=="__main__":

    import matplotlib.pyplot as plt

    def merge_dicts(dicts_in):
        """
        Concatenates contents of a list of N state dicts into a single dict by
        prepending a new dimension of size N. This is more convenient for plotting
        and analysis. Requires dicts to have consistent keys and have values that
        are numpy arrays.
        """
        dict_out = {}
        for k in dicts_in[0].keys():
            dict_out[k] = []
            for d in dicts_in:
                dict_out[k].append(d[k])
            dict_out[k] = np.array(dict_out[k])
        return dict_out

    sim_rate = 1/500
    mocap_params = {'pos_noise_density': 0.0005*np.ones((3,)),  # noise density for position 
                'vel_noise_density': 0.005*np.ones((3,)),          # noise density for velocity
                'att_noise_density': 0.0005*np.ones((3,)),          # noise density for attitude 
                'rate_noise_density': 0.0005*np.ones((3,)),         # noise density for body rates
                'vel_artifact_max': 5,                              # maximum magnitude of the artifact in velocity (m/s)
                'vel_artifact_prob': 0.001,                         # probability that an artifact will occur for a given velocity measurement
                'rate_artifact_max': 1,                             # maximum magnitude of the artifact in body rates (rad/s)
                'rate_artifact_prob': 0.0002                        # probability that an artifact will occur for a given rate measurement
                }
    sensor = MotionCapture(sampling_rate=sim_rate, mocap_params=mocap_params, with_artifacts=True)

    measurements = []

    state = {'x': np.zeros((3,)), 'v': np.zeros((3,)), 'q': np.array([0,0,0,1]), 'w': np.zeros((3,))}
    for i in range(1000):
        state = {'x': np.array([np.sin(2*np.pi*i/1000), np.sin(2*np.pi*i/1000 - np.pi/2), np.sin(2*np.pi*i/1000 - np.pi/5)]), 
                 'v': np.array([np.sin(2*np.pi*i/1000), np.sin(2*np.pi*i/1000 - np.pi/2), np.sin(2*np.pi*i/1000 - np.pi/5)]), 
                 'q': np.array([0,0,0,1]), 
                 'w': np.array([np.sin(2*np.pi*i/1000), np.sin(2*np.pi*i/1000 - np.pi/2), np.sin(2*np.pi*i/1000 - np.pi/5)])}
        current = sensor.measurement(state, with_noise=True, with_artifacts=True)
        measurements.append(current)

    measurements = merge_dicts(measurements)

    x_m = measurements['x']
    v_m = measurements['v']
    q_m = measurements['q']
    w_m = measurements['w']

    q_norm = np.linalg.norm(q_m, axis=1)
    
    (fig, axes) = plt.subplots(nrows=4, ncols=1, sharex=True, num="Measurements")
    fig.set_figwidth(9)
    fig.set_figheight(9)
    axe = axes[0]
    axe.plot(x_m[:,0], 'r', markersize=2)
    axe.plot(x_m[:,1], 'g', markersize=2)
    axe.plot(x_m[:,2], 'b', markersize=2)
    axe.set_ylim(bottom=-1.5, top=1.5)
    axe = axes[1]
    axe.plot(v_m[:,0], 'r', markersize=2)
    axe.plot(v_m[:,1], 'g', markersize=2)
    axe.plot(v_m[:,2], 'b', markersize=2)
    axe.set_ylim(bottom=-1.5, top=1.5)
    axe = axes[2]
    axe.plot(q_m[:,0], 'r', markersize=2)
    axe.plot(q_m[:,1], 'g', markersize=2)
    axe.plot(q_m[:,2], 'b', markersize=2)
    axe.plot(q_m[:,3], 'm', markersize=2)
    axe.plot(q_norm, 'k', markersize=2)
    axe = axes[3]
    axe.plot(w_m[:,0], 'r', markersize=2)
    axe.plot(w_m[:,1], 'g', markersize=2)
    axe.plot(w_m[:,2], 'b', markersize=2)
    axe.set_ylim(bottom=-1.5, top=1.5)

    plt.show()