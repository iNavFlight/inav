import numpy as np
import torch
import roma
from scipy.spatial.transform import Rotation

class SE3Control(object):
    """
    Quadrotor trajectory tracking controller based on https://ieeexplore.ieee.org/document/5717652 

    """
    def __init__(self, quad_params):
        """
        Parameters:
            quad_params, dict with keys specified in rotorpy/vehicles
        """

        # Quadrotor physical parameters.
        # Inertial parameters
        self.mass            = quad_params['mass'] # kg
        self.Ixx             = quad_params['Ixx']  # kg*m^2
        self.Iyy             = quad_params['Iyy']  # kg*m^2
        self.Izz             = quad_params['Izz']  # kg*m^2
        self.Ixy             = quad_params['Ixy']  # kg*m^2
        self.Ixz             = quad_params['Ixz']  # kg*m^2
        self.Iyz             = quad_params['Iyz']  # kg*m^2

        # Frame parameters
        self.c_Dx            = quad_params['c_Dx']  # drag coeff, N/(m/s)**2
        self.c_Dy            = quad_params['c_Dy']  # drag coeff, N/(m/s)**2
        self.c_Dz            = quad_params['c_Dz']  # drag coeff, N/(m/s)**2

        self.num_rotors      = quad_params['num_rotors']
        self.rotor_pos       = quad_params['rotor_pos']
        self.rotor_dir       = quad_params['rotor_directions']

        # Rotor parameters    
        self.rotor_speed_min = quad_params['rotor_speed_min'] # rad/s
        self.rotor_speed_max = quad_params['rotor_speed_max'] # rad/s

        self.k_eta           = quad_params['k_eta']     # thrust coeff, N/(rad/s)**2
        self.k_m             = quad_params['k_m']       # yaw moment coeff, Nm/(rad/s)**2
        self.k_d             = quad_params['k_d']       # rotor drag coeff, N/(m/s)
        self.k_z             = quad_params['k_z']       # induced inflow coeff N/(m/s)
        self.k_flap          = quad_params['k_flap']    # Flapping moment coefficient Nm/(m/s)

        # Motor parameters
        self.tau_m           = quad_params['tau_m']     # motor reponse time, seconds

        # You may define any additional constants you like including control gains.
        self.inertia = np.array([[self.Ixx, self.Ixy, self.Ixz],
                                 [self.Ixy, self.Iyy, self.Iyz],
                                 [self.Ixz, self.Iyz, self.Izz]]) # kg*m^2
        self.g = 9.81 # m/s^2

        # Gains  
        self.kp_pos = np.array([6.5,6.5,15])
        self.kd_pos = np.array([4.0, 4.0, 9])
        self.kp_att = 544
        self.kd_att = 46.64
        self.kp_vel = 0.1*self.kp_pos   # P gain for velocity controller (only used when the control abstraction is cmd_vel)

        # Linear map from individual rotor forces to scalar thrust and vector
        # moment applied to the vehicle.
        k = self.k_m/self.k_eta  # Ratio of torque to thrust coefficient. 

        # Below is an automated generation of the control allocator matrix. It assumes that all thrust vectors are aligned
        # with the z axis.
        self.f_to_TM = np.vstack((np.ones((1,self.num_rotors)),
                                  np.hstack([np.cross(self.rotor_pos[key],np.array([0,0,1])).reshape(-1,1)[0:2] for key in self.rotor_pos]), 
                                 (k * self.rotor_dir).reshape(1,-1)))
        self.TM_to_f = np.linalg.inv(self.f_to_TM)
    
    def update(self, t, state, flat_output):
        """
        This function receives the current time, true state, and desired flat
        outputs. It returns the command inputs.

        Inputs:
            t, present time in seconds
            state, a dict describing the present state with keys
                x, position, m
                v, linear velocity, m/s
                q, quaternion [i,j,k,w]
                w, angular velocity, rad/s
            flat_output, a dict describing the present desired flat outputs with keys
                x,        position, m
                x_dot,    velocity, m/s
                x_ddot,   acceleration, m/s**2
                x_dddot,  jerk, m/s**3
                x_ddddot, snap, m/s**4
                yaw,      yaw angle, rad
                yaw_dot,  yaw rate, rad/s

        Outputs:
            control_input, a dict describing the present computed control inputs with keys
                cmd_motor_speeds, rad/s
                cmd_motor_thrusts, N
                cmd_thrust, N 
                cmd_moment, N*m
                cmd_q, quaternion [i,j,k,w]
                cmd_w, angular rates in the body frame, rad/s
                cmd_v, velocity in the world frame, m/s
                cmd_acc, mass normalized thrust vector in the world frame, m/s/s.

                Not all keys are used, it depends on the control_abstraction selected when initializing the Multirotor object. 
        """
        cmd_motor_speeds = np.zeros((4,))
        cmd_thrust = 0
        cmd_moment = np.zeros((3,))
        cmd_q = np.zeros((4,))

        def normalize(x):
            """Return normalized vector."""
            return x / np.linalg.norm(x)

        def vee_map(S):
            """Return vector corresponding to given skew symmetric matrix."""
            return np.array([-S[1,2], S[0,2], -S[0,1]])

        # Get the desired force vector.
        pos_err  = state['x'] - flat_output['x']
        dpos_err = state['v'] - flat_output['x_dot']
        F_des = self.mass * (- self.kp_pos*pos_err
                             - self.kd_pos*dpos_err
                             + flat_output['x_ddot']
                             + np.array([0, 0, self.g]))

        # Desired thrust is force projects onto b3 axis.
        R = Rotation.from_quat(state['q']).as_matrix()
        b3 = R @ np.array([0, 0, 1])
        u1 = np.dot(F_des, b3)

        # Desired orientation to obtain force vector.
        b3_des = normalize(F_des)
        yaw_des = flat_output['yaw']
        c1_des = np.array([np.cos(yaw_des), np.sin(yaw_des), 0])
        b2_des = normalize(np.cross(b3_des, c1_des))
        b1_des = np.cross(b2_des, b3_des)
        R_des = np.stack([b1_des, b2_des, b3_des]).T

        # Orientation error.
        S_err = 0.5 * (R_des.T @ R - R.T @ R_des)
        att_err = vee_map(S_err)

        # Angular velocity error (this is oversimplified).
        w_des = np.array([0, 0, flat_output['yaw_dot']])
        w_err = state['w'] - w_des

        # Desired torque, in units N-m.
        u2 = self.inertia @ (-self.kp_att*att_err - self.kd_att*w_err) + np.cross(state['w'], self.inertia@state['w'])  # Includes compensation for wxJw component

        # Compute command body rates by doing PD on the attitude error. 
        cmd_w = -self.kp_att*att_err - self.kd_att*w_err

        # Compute motor speeds. Avoid taking square root of negative numbers.
        TM = np.array([u1, u2[0], u2[1], u2[2]])
        cmd_rotor_thrusts = self.TM_to_f @ TM
        cmd_motor_speeds = cmd_rotor_thrusts / self.k_eta
        cmd_motor_speeds = np.sign(cmd_motor_speeds) * np.sqrt(np.abs(cmd_motor_speeds))

        # Assign controller commands.
        cmd_thrust = u1                                             # Commanded thrust, in units N.
        cmd_moment = u2                                             # Commanded moment, in units N-m.
        cmd_q = Rotation.from_matrix(R_des).as_quat()               # Commanded attitude as a quaternion.
        cmd_v = -self.kp_vel*pos_err + flat_output['x_dot']         # Commanded velocity in world frame (if using cmd_vel control abstraction), in units m/s
        cmd_acc = F_des/self.mass                                   # Commanded acceleration in world frame (if using cmd_acc control abstraction)

        control_input = {'cmd_motor_speeds':cmd_motor_speeds,
                         'cmd_motor_thrusts':cmd_rotor_thrusts,
                         'cmd_thrust':cmd_thrust,
                         'cmd_moment':cmd_moment,
                         'cmd_q':cmd_q,
                         'cmd_w':cmd_w,
                         'cmd_v':cmd_v,
                         'cmd_acc': cmd_acc}
        
        return control_input


class BatchedSE3Control(object):
    def __init__(self, batch_params, num_drones, device, kp_pos=None, kd_pos=None, kp_att=None, kd_att=None):
        '''
        batch_params, BatchedMultirotorParams object 
        num_drones: int, number of drones in the batch
        device: torch.device("cpu") or torch.device("cuda")

        kp_pos: torch.Tensor of shape (num_drones, 3)
        kd_pos: torch.Tensor of shape (num_drones, 3)
        kp_att: torch.Tensor of shape (num_drones, 1)
        kd_att: torch.Tensor of shape (num_drones, 1)
        '''
        assert batch_params.device == device
        self.params = batch_params
        self.device = device
        # Quadrotor physical parameters

        # Gains
        if kp_pos is None:
            self.kp_pos = torch.tensor([6.5, 6.5, 15], device=self.device).repeat(num_drones, 1).double()
        else:
            self.kp_pos = kp_pos.to(self.device).double()
        if kd_pos is None:
            self.kd_pos = torch.tensor([4.0, 4.0, 9], device=self.device).repeat(num_drones, 1).double()
        else:
            self.kd_pos = kd_pos.to(self.device).double()
        if kp_att is None:
            self.kp_att = torch.tensor([544], device=device).repeat(num_drones, 1).double()
        else:
            self.kp_att = kp_att.to(self.device).double()
            if len(self.kp_att.shape) < 2:
                self.kp_att = self.kp_att.unsqueeze(-1)
        if kd_att is None:
            self.kd_att = torch.tensor([46.64], device=device).repeat(num_drones, 1).double()
        else:
            self.kd_att = kd_att.to(self.device).double()
            if len(self.kd_att.shape) < 2:
                self.kd_att = self.kd_att.unsqueeze(-1)

        self.kp_vel = 0.1 * self.kp_pos

    def normalize(self, x):
        return x / torch.norm(x, dim=-1, keepdim=True)

    def update(self, t, states, flat_outputs, idxs=None):
        '''
        Computes a batch of control outputs for the drones specified by idxs
        :param states: a dictionary of pytorch tensors containing the states of the quadrotors (expects double precision)
        :param flat_outputs: a dictionary of pytorch tensors containing the reference trajectories for each quad. (expects double precision)
        :param idxs: a list of which drones to update
        :return:
        '''
        if idxs is None:
            idxs = [i for i in range(states['x'].shape[0])]
        pos_err = states['x'][idxs].double() - flat_outputs['x'][idxs].double()
        dpos_err = states['v'][idxs].double() - flat_outputs['x_dot'][idxs].double()

        F_des = self.params.mass[idxs] * (-self.kp_pos[idxs] * pos_err
                             - self.kd_pos[idxs] * dpos_err
                             + flat_outputs['x_ddot'][idxs].double()
                             + torch.tensor([0, 0, self.params.g], device=self.device))


        R = roma.unitquat_to_rotmat(states['q'][idxs]).double()
        b3 = R @ torch.tensor([0.0, 0.0, 1.0], device=self.device).double()
        u1 = torch.sum(F_des * b3, dim=-1).double()

        b3_des = self.normalize(F_des)
        yaw_des = flat_outputs['yaw'][idxs].double()
        c1_des = torch.stack([torch.cos(yaw_des), torch.sin(yaw_des), torch.zeros_like(yaw_des)], dim=-1)
        b2_des = self.normalize(torch.cross(b3_des, c1_des, dim=-1))
        b1_des = torch.cross(b2_des, b3_des, dim=-1)
        R_des = torch.stack([b1_des, b2_des, b3_des], dim=-1)

        S_err = 0.5 * (R_des.transpose(-1, -2) @ R - R.transpose(-1, -2) @ R_des)
        att_err = torch.stack([-S_err[:, 1, 2], S_err[:, 0, 2], -S_err[:, 0, 1]], dim=-1)

        w_des = torch.stack([torch.zeros_like(yaw_des), torch.zeros_like(yaw_des), flat_outputs['yaw_dot'][idxs].double()], dim=-1).to(self.device)
        w_err = states['w'][idxs].double()- w_des

        Iw = self.params.inertia[idxs] @ states['w'][idxs].unsqueeze(-1).double()
        tmp = -self.kp_att[idxs] * att_err - self.kd_att[idxs] * w_err
        u2 = (self.params.inertia[idxs] @ tmp.unsqueeze(-1)).squeeze(-1) + torch.cross(states['w'][idxs].double(), Iw.squeeze(-1), dim=-1)

        TM = torch.cat([u1.unsqueeze(-1), u2], dim=-1)
        cmd_rotor_thrusts = (self.params.TM_to_f[idxs] @ TM.unsqueeze(1).transpose(-1, -2)).squeeze(-1)
        cmd_motor_speeds = cmd_rotor_thrusts / self.params.k_eta[idxs]
        cmd_motor_speeds = torch.sign(cmd_motor_speeds) * torch.sqrt(torch.abs(cmd_motor_speeds))

        cmd_q = roma.rotmat_to_unitquat(R_des)
        cmd_v = -self.kp_vel[idxs] * pos_err + flat_outputs['x_dot'][idxs].double()

        control_inputs = BatchedSE3Control._unpack_control(cmd_motor_speeds,
                                                           cmd_rotor_thrusts,
                                                           u1.unsqueeze(-1),
                                                           u2,
                                                           cmd_q,
                                                           -self.kp_att[idxs] * att_err - self.kd_att[idxs] * w_err,
                                                           cmd_v,
                                                           F_des/self.params.mass[idxs],
                                                           idxs,
                                                           states['x'].shape[0])

        return control_inputs

    @classmethod
    def _unpack_control(cls, cmd_motor_speeds, cmd_motor_thrusts,
                        u1, u2, cmd_q, cmd_w, cmd_v, cmd_acc, idxs, num_drones):
        device = cmd_motor_speeds.device
        # fill state with zeros, then replace with appropriate indexes.
        ctrl = {'cmd_motor_speeds': torch.zeros(num_drones, 4, dtype=torch.double, device=device),
                'cmd_motor_thrusts': torch.zeros(num_drones, 4, dtype=torch.double, device=device),
                 'cmd_thrust': torch.zeros(num_drones, 1, dtype=torch.double, device=device),
                 'cmd_moment': torch.zeros(num_drones, 3, dtype=torch.double, device=device),
                 'cmd_q': torch.zeros(num_drones, 4, dtype=torch.double, device=device),
                 'cmd_w': torch.zeros(num_drones, 3, dtype=torch.double, device=device),
                 'cmd_v': torch.zeros(num_drones, 3, dtype=torch.double, device=device),
                 'cmd_acc': torch.zeros(num_drones, 3, dtype=torch.double, device=device)}

        ctrl['cmd_motor_speeds'][idxs] = cmd_motor_speeds
        ctrl['cmd_motor_thrusts'][idxs] = cmd_motor_thrusts
        ctrl['cmd_thrust'][idxs] = u1
        ctrl['cmd_moment'][idxs] = u2
        ctrl['cmd_q'][idxs] = cmd_q
        ctrl['cmd_w'][idxs] = cmd_w
        ctrl['cmd_v'][idxs] = cmd_v
        ctrl['cmd_acc'][idxs] = cmd_acc
        return ctrl
