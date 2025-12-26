import numpy as np
from scipy.spatial.transform import Rotation
import torch
import copy

class Imu:
    """
    Simulated IMU measurement given
      1) quadrotor's ground truth state and acceleration, and
      2) IMU's pose in quadrotor body frame.

      CREDIT:
      Partial implementation from Yu-Ming Chen and MEAM 620 Sp2022 Teaching Staff
      Finishing touches added by Alexander Spinos, checked by Spencer Folk.
    """
    def __init__(self, accelerometer_params={'initial_bias': np.array([0.,0.,0.]), # m/s^2
                                             'noise_density': (0.038**2)*np.ones(3,), # m/s^2 / sqrt(Hz)
                                             'random_walk': np.zeros(3,) # m/s^2 * sqrt(Hz)
                                             }, 
                gyroscope_params={'initial_bias': np.array([0.,0.,0.]), # m/s^2
                                  'noise_density': (0.01**2)*np.ones(3,), # rad/s / sqrt(Hz)
                                  'random_walk': np.zeros(3,) # rad/s * sqrt(Hz)
                                  }, 
                R_BS = np.eye(3),
                p_BS = np.zeros(3,),
                sampling_rate=500, 
                gravity_vector=np.array([0,0,-9.81])):
        """
        Parameters:
            R_BS, the rotation matrix of sensor frame S in body frame B
            p_BS, the position vector from frame B's origin to frame S's origin, expressed in frame B
            accelerometer_params, a dict with keys:
                initial_bias, accelerometer contant bias,    [ m / s^2 ]
                noise_density, accelerometer "white noise",  [ m / s^2 / sqrt(Hz) ]
                random_walk, accelerometer bias diffusion,   [ m / s^2 * sqrt(Hz) ]
            gyroscope_params, a dict with keys:
                initial_bias, gyro contant bias,             [ m / s^2 ]
                noise_density, gyro "white noise",           [ rad / s / sqrt(Hz) ]
                random_walk, gyro bias diffusion,            [ rad / s * sqrt(Hz) ]
            sampling_rate, the sampling rate of the sensor, Hz (1/s)
            gravity_vector, the gravitational vector in world frame (should be ~ [0, 0 , -9.81])
        """

        # A few checks
        if type(R_BS) != np.ndarray:
            raise TypeError("R_BS's type is not numpy.ndarray")
        if type(p_BS) != np.ndarray:
            raise TypeError("p_BS's type is not numpy.ndarray")
        if type(gravity_vector) != np.ndarray:
            raise TypeError("gravity_vector's type is not numpy.ndarray")
        if R_BS.shape != (3, 3):
            raise ValueError("R_BS's size is not (3, 3)")
        if p_BS.shape != (3,):
            raise ValueError("p_BS's size is not (3,)")
        if gravity_vector.shape != (3,):
            raise ValueError("gravity_vector's size is not (3,)")

        self.R_BS = R_BS
        self.p_BS = p_BS
        self.rate_scale = np.sqrt(sampling_rate/2)
        self.gravity_vector = gravity_vector

        self.accel_variance = accelerometer_params['noise_density'].astype('float64')
        self.accel_random_walk = accelerometer_params['random_walk'].astype('float64')
        self.accel_bias = accelerometer_params['initial_bias'].astype('float64')
        self.gyro_variance = gyroscope_params['noise_density'].astype('float64')
        self.gyro_random_walk = gyroscope_params['random_walk'].astype('float64')
        self.gyro_bias = gyroscope_params['initial_bias'].astype('float64')

    def bias_step(self):
        """Simulate bias drift"""

        self.accel_bias += np.random.normal(scale=self.accel_random_walk) / self.rate_scale
        self.gyro_bias += np.random.normal(scale=self.gyro_random_walk) / self.rate_scale

        return

    def measurement(self, state, acceleration, with_noise=True):
        """
        Computes and returns the IMU measurement at a time step.

        Inputs:
            state, a dict describing the state with keys
                x, position, m, shape=(3,)
                v, linear velocity, m/s, shape=(3,)
                q, quaternion [i,j,k,w], shape=(4,)
                w, angular velocity (in LOCAL frame!), rad/s, shape=(3,)
            acceleration, a dict describing the acceleration with keys
                vdot, quadrotor's linear acceleration expressed in world frame, m/s^2, shape=(3,)
                wdot, quadrotor's angular acceleration expressed in world frame, rad/s^2, shape=(3,)
        Outputs:
            observation, a dict describing the IMU measurement with keys
                accel, simulated accelerometer measurement, m/s^2, shape=(3,)
                gyro, simulated gyroscope measurement, rad/s^2, shape=(3,)
        """
        q_WB = state['q']
        w_WB = state['w']

        alpha_WB_W = acceleration[10:13] #wdot
        a_WB_W = acceleration[3:6]   #vdot

        # Rotation matrix of the body frame B in world frame W
        R_WB = Rotation.from_quat(q_WB).as_matrix()

        # Sensor position in body frame expressed in world coordinates
        p_BS_W = R_WB @ self.p_BS

        # Linear acceleration of point S (the imu) expressed in world coordinates W.
        a_WS_W = a_WB_W + np.cross(alpha_WB_W, p_BS_W) + np.cross(w_WB, np.cross(w_WB, p_BS_W))

        # Rotation from world to imu: R_SW = R_SB * R_BW
        R_SW = self.R_BS.T @ R_WB.T

        # Rotate to local frame
        accelerometer_measurement = R_SW @ (a_WS_W - self.gravity_vector)
        gyroscope_measurement = copy.deepcopy(w_WB).astype(float)

        # Add the bias drift (default 0)
        self.bias_step()

        # Add biases and noises
        accelerometer_measurement += self.accel_bias
        gyroscope_measurement += self.gyro_bias
        if with_noise:
            accelerometer_measurement += self.rate_scale * np.random.normal(scale=np.abs(self.accel_variance))
            gyroscope_measurement +=  self.rate_scale * np.random.normal(scale=np.abs(self.gyro_variance))

        return {'accel': accelerometer_measurement, 'gyro': gyroscope_measurement}

class BatchedImu:
    def __init__(self, num_drones,
                 accelerometer_params=None,
                 gyroscope_params=None,
                 R_BS=None,
                 p_BS=None,
                 sampling_rate=500,
                 gravity_vector=None,
                 device='cpu'):
        self.device = device
        self.num_drones = num_drones
        self.sampling_rate = sampling_rate
        self.rate_scale = (sampling_rate / 2) ** 0.5

        self.R_BS = torch.eye(3, device=device).double() if R_BS is None else R_BS.double().to(device)
        self.p_BS = torch.zeros(3, device=device).double() if p_BS is None else p_BS.double().to(device)
        self.gravity_vector = torch.tensor([0, 0, -9.81], device=device, dtype=torch.double) if gravity_vector is None else gravity_vector.double().to(device)

        accel_params = accelerometer_params or {
            'initial_bias': torch.zeros(3, device=device).double(),
            'noise_density': (0.38 ** 2) * torch.ones(3, device=device).double(),
            'random_walk': torch.zeros(3, device=device).double()
        }

        gyro_params = gyroscope_params or {
            'initial_bias': torch.zeros(3, device=device).double(),
            'noise_density': (0.01 ** 2) * torch.ones(3, device=device).double(),
            'random_walk': torch.zeros(3, device=device).double()
        }

        # Repeat biases for each drone
        self.accel_bias = torch.Tensor(accel_params['initial_bias'][np.newaxis].repeat(num_drones, 1)).double()
        self.gyro_bias = torch.Tensor(gyro_params['initial_bias'][np.newaxis].repeat(num_drones, 1)).double()
        self.accel_noise = torch.Tensor(accel_params['noise_density']).double()
        self.gyro_noise = torch.Tensor(gyro_params['noise_density']).double()
        self.accel_random_walk = torch.Tensor(accel_params['random_walk']).double()
        self.gyro_random_walk = torch.Tensor(gyro_params['random_walk']).double()

    def bias_step(self):
        self.accel_bias += torch.randn_like(self.accel_bias) * (self.accel_random_walk / self.rate_scale)
        self.gyro_bias += torch.randn_like(self.gyro_bias) * (self.gyro_random_walk / self.rate_scale)

    def measurement(self, state, acceleration, idxs=None, with_noise=True):
        """
        state['x'], ['v'], ['w']: (num_drones, 3)
        state['q']: (num_drones, 4) (xyzw format)
        acceleration['vdot'], ['wdot']: (B, 3)
        idxs: list of drones in the batch for which to compute IMU measurements.
        """
        if idxs is None:
            idxs = [i for i in range(self.num_drones)]

        q_WB = state['q'][idxs]  # (num_drones, 4)
        w_WB = state['w'][idxs]
        alpha_WB_W = acceleration['wdot'][idxs]
        a_WB_W = acceleration['vdot'][idxs]

        # Get rotation matrices from quaternions (num_drones, 3, 3)
        R_WB = self.quat_to_rotmat(q_WB)

        # Sensor offset in world frame
        p_BS_W = torch.einsum('bij,j->bi', R_WB, self.p_BS)

        cross_w_p = torch.cross(w_WB, p_BS_W, dim=-1)
        cross_w_w_p = torch.cross(w_WB, cross_w_p, dim=-1)
        cross_alpha_p = torch.cross(alpha_WB_W, p_BS_W, dim=-1)
        a_WS_W = a_WB_W + cross_alpha_p + cross_w_w_p

        R_SW = torch.einsum('ij,bij->bij', self.R_BS.T, R_WB.transpose(1, 2))
        a_WS_S = torch.einsum('bij,bj->bi', R_SW, a_WS_W - self.gravity_vector)

        # Apply bias + noise
        self.bias_step()
        a_meas = a_WS_S + self.accel_bias[idxs]
        w_meas = w_WB + self.gyro_bias[idxs]

        if with_noise:
            a_meas += self.rate_scale * torch.randn_like(a_meas) * self.accel_noise
            w_meas += self.rate_scale * torch.randn_like(w_meas) * self.gyro_noise

        a_meas_out = torch.full((self.num_drones, 3), float("nan"), device=a_meas.device).double()
        w_meas_out = torch.full((self.num_drones, 3), float("nan"), device=w_meas.device).double()
        a_meas_out[idxs] = a_meas
        w_meas_out[idxs] = w_meas

        return {'accel': a_meas_out, 'gyro': w_meas_out}

    @staticmethod
    def quat_to_rotmat(q):
        # Input shape (B, 4) in xyzw
        x, y, z, w = q[:, 0], q[:, 1], q[:, 2], q[:, 3]
        B = q.shape[0]
        R = torch.empty((B, 3, 3), device=q.device, dtype=torch.double)
        R[:, 0, 0] = 1 - 2*(y**2 + z**2)
        R[:, 0, 1] = 2*(x*y - z*w)
        R[:, 0, 2] = 2*(x*z + y*w)
        R[:, 1, 0] = 2*(x*y + z*w)
        R[:, 1, 1] = 1 - 2*(x**2 + z**2)
        R[:, 1, 2] = 2*(y*z - x*w)
        R[:, 2, 0] = 2*(x*z - y*w)
        R[:, 2, 1] = 2*(y*z + x*w)
        R[:, 2, 2] = 1 - 2*(x**2 + y**2)
        return R

if __name__ == "__main__":

    sim_rate = 100
    accelerometer_params = {
        'initial_bias': np.array([0, 0, 0]),  # m/s^2
        'noise_density': (0.38**2) * np.ones(3,),  # m/s^2 / sqrt(Hz)
        'random_walk': np.zeros(3,)  # m/s^2 * sqrt(Hz)
    }
    gyroscope_params = {
        'initial_bias': np.array([0, 0, 0]),  # rad/s
        'noise_density': (0.01**2) * np.ones(3,),  # rad/s / sqrt(Hz)
        'random_walk': np.zeros(3,)  # rad/s * sqrt(Hz)
    }

    imu = Imu(accelerometer_params,
              gyroscope_params,
              p_BS=np.zeros(3,),
              R_BS=np.eye(3),
              sampling_rate=sim_rate)

    state_np = {'x': np.array([0, 0, 0]),
                'v': np.array([0, 0, 0]),
                'q': np.array([0, 0, 0, 1]),
                'w': np.array([0, 0, 0])}

    accel_np = {'vdot': np.array([0, 0, 0]),
                'wdot': np.array([0, 0, 0])}

    meas_single = imu.measurement(state_np, accel_np, with_noise=False)

    print("=== Single Drone IMU Measurement ===")
    print("Accel:", meas_single['accel'])
    print("Gyro: ", meas_single['gyro'])

    num_drones = 10  # batch size
    batched_imu = BatchedImu(num_drones=1,
                              accelerometer_params=accelerometer_params,
                              gyroscope_params=gyroscope_params,
                              p_BS=torch.zeros(3),
                              R_BS=torch.eye(3),
                              sampling_rate=sim_rate)

    state_batch = {
        'x': torch.zeros(num_drones, 3),
        'v': torch.zeros(num_drones, 3),
        'q': torch.tensor([0., 0., 0., 1.]).unsqueeze(0).repeat(num_drones, 1),
        'w': torch.zeros(num_drones, 3)
    }

    accel_batch = {
        'vdot': torch.zeros(num_drones, 3),
        'wdot': torch.zeros(num_drones, 3)
    }

    meas_batch = batched_imu.measurement(state_batch, accel_batch, with_noise=False)

    print("\n=== Batched IMU Measurement ===")
    print("Accel:", meas_batch['accel'].numpy())
    print("Gyro: ", meas_batch['gyro'].numpy())

    print("\n=== Difference ===")
    print("Accel diff:", np.abs(meas_single['accel'] - meas_batch['accel'][0].numpy()))
    print("Gyro diff: ", np.abs(meas_single['gyro'] - meas_batch['gyro'][0].numpy()))