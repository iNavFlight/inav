import numpy as np
from scipy.spatial.transform import Rotation
import copy


class WindEKF:
    """
    Wind EKF: 
        Given approximate dynamics near level flight, the wind EKF will produce an estimate of the local wind vector acting on the body. 
        It requires knowledge of the effective drag coefficient on each axis, which would be determined either from real flight or computed in simulation, and the mass of the vehicle. 
        The inputs to the filter are the mass normalized collective thrust and the body rates on each axis. 
        Measurements of body velocity, Euler angles, and acceleration are provided to the vehicle. 
        The filter estimates the Euler angles, body velocities, and wind velocities. 

        State space: 
            xhat = [phi, theta, psi, u, v, w, windx, windy, windz]
        Input space: 
            u = [T/m, p, q, r]
    """

    def __init__(self, quad_params, Q=np.diag(np.concatenate([0.5*np.ones(3),0.7*np.ones(3),0.1*np.ones(3)])),
                                    R=np.diag(np.concatenate([0.0005*np.ones(3),0.0010*np.ones(3),np.sqrt(100/2)*(0.38**2)*np.ones(3)])), 
                                    xhat0=np.array([0,0,0, 0.1,0.05,0.02, 1.5,1.5,1.5]), 
                                    P0=1*np.eye(9),
                                    dt=1/100):
        """
        Inputs:
            quad_params, dict with keys specified in quadrotor_params.py
            Q, the process noise covariance
            R, the measurement noise covariance
            x0, the initial filter state
            P0, the initial state covariance
            dt, the time between predictions
        """
        # Quadrotor physical parameters.
        # Inertial parameters
        self.mass            = quad_params['mass'] # kg

        # Frame parameters
        self.c_Dx            = quad_params['c_Dx']  # drag coeff, N/(m/s)**2
        self.c_Dy            = quad_params['c_Dy']  # drag coeff, N/(m/s)**2
        self.c_Dz            = quad_params['c_Dz']  # drag coeff, N/(m/s)**2

        self.g = 9.81 # m/s^2

        # Filter parameters
        self.Q = Q
        self.R = R
        self.xhat = xhat0
        self.P = P0

        self.innovation = np.zeros((9,))

        self.dt = dt

        # Initialize the Jacobians at starting position and assuming hover thrust.  
        self.computeJacobians(self.xhat, np.array([self.g, 0, 0, 0]))

    def step(self, ground_truth_state, controller_command, imu_measurement, mocap_measurement):
        """
        This will perform both a propagate and update step in one for the sake of readability in other parts of the code.
        """
        self.propagate(ground_truth_state, controller_command)
        self.update(ground_truth_state, controller_command, imu_measurement, mocap_measurement)

        return self.pack_results()

    def propagate(self, ground_truth_state, controller_command):
        """
        Propagate occurs whenever an action u is taken.
        Inputs:
            ground_truth_state, the ground truth state is mainly there if it's necessary to compute certain portions of the state, e.g., actual thrust produced by the rotors. 
            controller_command, the controller command taken, this has to be converted to the appropriate control vector u depending on the filter model. 

        Outputs:
            xhat, the current state estimate after propagation
            P, the current covariance matrix after propagation
        """

        # Extract the appropriate u vector based on the controller commands.
        uk = self.construct_control_vector(ground_truth_state, controller_command)

        # First propagate the dynamics using the process model
        self.xhat = self.process_model(self.xhat, uk)

        # Update the covariance matrix using the linearized version of the dynamics
        self.computeJacobians(self.xhat, uk)
        self.P = self.A@self.P@(self.A.T) + self.Q

        return self.pack_results

    def update(self, ground_truth_state, controller_command, imu_measurement, mocap_measurement):
        """
        Update the estimate based on new sensor measurments. 
        Inputs:
            ground_truth_state, the ground truth state is mainly there if it's necessary to compute certain portions of the state, e.g., actual thrust produced by the rotors. 
            controller_command, the controller command taken, this has to be converted to the appropriate control vector u depending on the filter model. 
            imu_measurement, contains measurements from an inertial measurement unit. These measurements are noisy, potentially biased, and potentially off-axis. The measurement
                        is specific acceleration, i.e., total force minus gravity. 
            mocap_measurement, provides noisy measurements of pose and twist. 
        
        Outputs:
            xhat, the current state estimate after measurement update
            P, the current covariance matrix after measurement update
        """

        # Extract the appropriate u vector based on the controller commands.
        uk = self.construct_control_vector(ground_truth_state, controller_command)

        # Construct the measurement vector yk
        orientation = Rotation.from_quat(copy.deepcopy(mocap_measurement['q']))
        euler_angles = orientation.as_euler('zyx', degrees=False)  # Get Euler angles from current orientation
        inertial_speed = mocap_measurement['v']
        body_speed = (orientation.as_matrix()).T@inertial_speed
        yk = np.array([euler_angles[0],                 # phi
                       euler_angles[1],                 # theta
                       euler_angles[2],                 # psi
                       body_speed[0],                   # vx
                       body_speed[1],                   # vy
                       body_speed[2],                   # vz
                       imu_measurement['accel'][0],     # body x acceleration
                       imu_measurement['accel'][1],     # body y acceleration
                       imu_measurement['accel'][2]      # body z acceleration
                       ])
        
        # First linearize the measurement model. 
        self.computeJacobians(self.xhat, uk)

        # Now compute the Kalman gain
        K = self.P@(self.C.T)@np.linalg.inv(self.C@self.P@(self.C.T) + self.R)

        # Next compute the posteriori distribution
        self.innovation = yk - self.measurement_model(self.xhat, uk)
        self.xhat = self.xhat + K@self.innovation
        self.P = (np.eye(self.xhat.shape[0]) - K@self.C)@self.P

        return self.pack_results()

    def process_model(self, xk, uk):
        """
        Process model
        """

        va = np.sqrt((xk[3]-xk[6])**2 + (xk[4]-xk[7])**2 + (xk[5]-xk[8])**2)  # Compute the norm of the airspeed vector

        # The process model is integrated using forward Euler. 
        xdot = np.array([uk[1] + xk[0]*xk[1]*uk[2] + xk[2]*uk[3], 
                        uk[2] - xk[0]*uk[3], 
                        xk[0]*uk[2] + uk[3], 
                        -self.c_Dx/self.mass*(xk[3]-xk[6])*va + self.g*xk[1] + xk[4]*uk[3] - xk[5]*uk[1],
                        -self.c_Dy/self.mass*(xk[4]-xk[7])*va - self.g*xk[0] + xk[5]*uk[1] - xk[3]*uk[3], 
                        uk[0] - self.c_Dz/self.mass*(xk[5]-xk[8])*va - self.g + xk[3]*uk[2] - xk[4]*uk[1], 
                        0,
                        0,
                        0])

        xkp1 = xk + xdot*self.dt

        return xkp1
    
    def measurement_model(self, xk, uk):
        """
        Measurement model
        """

        h = np.zeros(xk.shape)

        va = np.sqrt((xk[3]-xk[6])**2 + (xk[4]-xk[7])**2 + (xk[5]-xk[8])**2)  # Compute the norm of the airspeed vector

        h[0:3] = np.hstack((np.eye(3), np.zeros((3,6))))@(xk)
        h[3:6] = np.hstack((np.zeros((3,3)), np.eye(3), np.zeros((3,3))))@(xk)

        h[6:] = np.array([-self.c_Dx/self.mass*(xk[3]-xk[6])*va, 
                          -self.c_Dy/self.mass*(xk[4]-xk[7])*va,
                          uk[0]-self.c_Dz/self.mass*(xk[5]-xk[8])*va])

        return h

    def computeJacobians(self, x, u):
        """
        Compute the Jacobians of the process and measurement model at the operating points x and u.
        """

        va = np.sqrt((x[3]-x[6])**2 + (x[4]-x[7])**2 + (x[5]-x[8])**2)  # Compute the norm of the airspeed vector

        # Partial derivatives of va for chain rule
        dvadu = (x[3]-x[6])/va
        dvadv = (x[4]-x[7])/va
        dvadw = (x[5]-x[8])/va
        dvadwx = -dvadu
        dvadwy = -dvadv
        dvadwz = -dvadw

        kx = self.c_Dx/self.mass
        ky = self.c_Dy/self.mass
        kz = self.c_Dz/self.mass

        vax = x[3] - x[6]
        vay = x[4] - x[7]
        vaz = x[5] - x[8]

        self.A = np.array([[x[1]*u[2], x[0]*u[2] + u[3], 0, 0, 0, 0, 0, 0, 0],
                           [-u[3], 0, 0, 0, 0, 0, 0, 0, 0],
                           [ u[2], 0, 0, 0, 0, 0, 0, 0, 0],
                           [0, self.g, 0, -kx*(dvadu*vax + va), u[3] - kx*(dvadv*vax), -u[1] - kx*(dvadw*vax), -kx*(dvadwx*vax-va), -kx*(dvadwy*vax), -kx*(dvadwz*vax)],
                           [-self.g, 0, 0, -ky*(dvadu*vay)-u[3], -ky*(dvadv*vay + va), u[1]-ky*(dvadw*vay), -ky*(dvadwx*vay), -ky*(dvadv*vay - va), -ky*(dvadw*vay)],
                           [0, 0, 0, u[2] - kz*(dvadu*vaz), -u[1] - kz*(dvadv*vaz), -kz*(dvadw*vaz + va), -kz*(dvadwx*vaz), -kz*(dvadwy*vaz), -kz*(dvadwz*vaz - va)],
                           [0, 0, 0, 0, 0, 0, 0, 0, 0],
                           [0, 0, 0, 0, 0, 0, 0, 0, 0],
                           [0, 0, 0, 0, 0, 0, 0, 0, 0]])

        self.C = np.array([[1, 0, 0, 0, 0, 0, 0, 0, 0],
                           [0, 1, 0, 0, 0, 0, 0, 0, 0],
                           [0, 0, 1, 0, 0, 0, 0, 0, 0],
                           [0, 0, 0, 1, 0, 0, 0, 0, 0],
                           [0, 0, 0, 0, 1, 0, 0, 0, 0],
                           [0, 0, 0, 0, 0, 1, 0, 0, 0],
                           [0, 0, 0, -kx*(dvadu*vax + va),  -kx*(dvadv*vax), -kx*(dvadw*vax), -kx*(dvadwx*vax-va), -kx*(dvadwy*vax), -kx*(dvadwz*vax)],
                           [0, 0, 0, -ky*(dvadu*vay), -ky*(dvadv*vay + va), -ky*(dvadw*vay), -ky*(dvadwx*vay), -ky*(dvadv*vay - va), -ky*(dvadw*vay)],
                           [0, 0, 0, -kz*(dvadu*vaz), -kz*(dvadv*vaz), -kz*(dvadw*vaz + va), -kz*(dvadwx*vaz), -kz*(dvadwy*vaz), -kz*(dvadwz*vaz - va)]])

        return

    def construct_control_vector(self, ground_truth_state, controller_command):
        """
        Constructs control vector
        """
        uk = np.array([controller_command['cmd_thrust']/self.mass,    # Compute mass normalized thrust from the command thrust, note that this is not the actual thrust...
                       ground_truth_state['w'][0],                      # Body rate in x axis
                       ground_truth_state['w'][1],                      # Body rate in y axis
                       ground_truth_state['w'][2]]                      # Body rate in z axis
                       )

        return uk

    def pack_results(self):
        # return {'euler_est': self.xhat[0:3], 'v_est': self.xhat[3:6], 'wind_est': self.xhat[6:9],
        #          'covariance': self.P, 'innovation': self.innovation}
        return {'filter_state': self.xhat, 'covariance': self.P}