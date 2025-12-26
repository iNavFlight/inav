from rotorpy.vehicles.ardupilot_multirotor import Ardupilot
from rotorpy.vehicles.multirotor import Multirotor
from rotorpy.sensors.imu import Imu
from pymavlink import mavutil
import numpy as np
from typing import Tuple

import math

# Constants
R_EARTH = 6378137.0  # meters
rad2deg = 180.0 / np.pi
INT_MAX = 32767
INT_MIN = -32768

class SensorSource:
    """ The binary codes to signal which simulated data is being sent through mavlink

    Credit to Pegasus Simulator https://github.com/PegasusSimulator/PegasusSimulator
    
    Atribute:
        | ACCEL (int): mavlink binary code for the accelerometer (0b0000000000111 = 7)
        | GYRO (int): mavlink binary code for the gyroscope (0b0000000111000 = 56)
        | MAG (int): mavlink binary code for the magnetometer (0b0000111000000=448)
        | BARO (int): mavlink binary code for the barometer (0b1101000000000=6656)
        | DIFF_PRESS (int): mavlink binary code for the pressure sensor (0b0010000000000=1024)
    """

    ACCEL: int = 7    
    GYRO: int = 56          
    MAG: int = 448        
    BARO: int = 6656
    DIFF_PRESS: int = 1024

def _compute_hover_rotor_speeds(mass, k_eta, num_rotors, g=9.81):
    """Solve N·k_eta·ω² = m·g for ω and return an array length num_rotors."""
    omega = np.sqrt((mass * g) / (num_rotors * k_eta))
    return np.full(num_rotors, omega)

class PX4Multirotor(Multirotor):
    """PX4 Multirotor Vehicle Model

    Args:
        quad_params (dict): Quadrotor parameters.
        initial_state (dict, optional): Initial state of the quadrotor.
        autopilot_controller (bool): Whether to use the autopilot controller or not.
    """
    def __init__(
        self,
        quad_params,
        initial_state=None,
        control_abstraction="cmd_motor_speeds",
        aero=True,
        enable_ground=True,
        mavlink_url="tcpin:localhost:4560",
        autopilot_controller=True,
        lockstep=True,
        integrator_kwargs=None
    ):
        integrator_kwargs = integrator_kwargs if integrator_kwargs is not None else {'method':'Radau', 'rtol':1e-3, 'atol':1e-6, 'max_step':0.01}
        # If no initial state passed, initialize to hover at origin
        if initial_state is None:
            initial_state = {
                'x': np.zeros(3),
                'v': np.zeros(3),
                'q': np.array([0, 0, 0, 1]),
                'w': np.zeros(3),
                'wind': np.zeros(3),
                'rotor_speeds': np.zeros(quad_params['num_rotors'])
            }
        super().__init__(
            quad_params=quad_params,
            initial_state=initial_state,
            control_abstraction=control_abstraction,
            aero=aero,
            enable_ground=enable_ground,
            integrator_kwargs=integrator_kwargs
        )
        # Simulated IMU (with noise)
        self.imu = Imu()
        self._enable_imu_noise = True  # Always add a bit of noise to avoid stale detection
        self.t = 0.0

        print("PX4Multirotor: Initializing MAVLink connection... on {}".format(mavlink_url))
        self.conn = mavutil.mavlink_connection(mavlink_url)
        self.conn.wait_heartbeat()
        print("PX4Multirotor: MAVLink connection established.")

        self._autopilot_controller = autopilot_controller
        self._lockstep_enabled = lockstep

    @staticmethod
    def enu_to_geodetic(
        east_m: float,
        north_m: float,
        up_m: float,
        lat0_deg: float = 40.0,
        lon0_deg: float = -74.3,
        alt0_m: float = 0.0,
    ) -> Tuple[float, float, float]:
        """
        Convert local ENU coordinates (meters) to geodetic latitude, longitude, and altitude (WGS-84)
        using the simple equirectangular approximation.

        Assumptions:
        - Small-area approximation (recommended within ~10–20 km of the reference).
        - Spherical Earth with WGS-84 semi-major radius.

        Args:
            lat0_deg (float): Reference latitude (degrees, geodetic).
            lon0_deg (float): Reference longitude (degrees, geodetic).
            alt0_m   (float): Reference altitude above mean sea level (meters).
            east_m   (float): Local ENU 'east' offset from reference (meters).
            north_m  (float): Local ENU 'north' offset from reference (meters).
            up_m     (float): Local ENU 'up' offset from reference (meters).

        Returns:
            (lat_deg, lon_deg, alt_msl_m):
                lat_deg   (float): Latitude in degrees (geodetic).
                lon_deg   (float): Longitude in degrees (geodetic).
                alt_msl_m (float): Altitude above mean sea level in meters.

        Notes:
            - This keeps longitude unwrapped; you can normalize to [-180, 180) if desired.
            - For larger areas or higher accuracy, use a full ENU↔ECEF↔LLA conversion.
        """
        # WGS-84 semi-major axis (meters)
        R = 6378137.0

        # Convert reference latitude to radians once
        lat0_rad = math.radians(lat0_deg)

        # Equirectangular projection back to geodetic
        lat_deg = lat0_deg + (north_m / R) * (180.0 / math.pi)
        lon_deg = lon0_deg + (east_m / (R * math.cos(lat0_rad))) * (180.0 / math.pi)

        # Altitude: ENU 'up' increases MSL altitude
        alt_msl_m = alt0_m + up_m

        # (Optional) normalize longitude to [-180, 180)
        if lon_deg >= 180.0 or lon_deg < -180.0:
            lon_deg = ((lon_deg + 180.0) % 360.0) - 180.0

        return lat_deg, lon_deg, alt_msl_m

    @staticmethod
    def geodetic_to_mavlink(lat_deg: float, lon_deg: float, alt_msl_m: float) -> Tuple[int, int, int]:
        """
        Convert geodetic coordinates to MAVLink integer fields.

        Args:
            lat_deg (float): Latitude in degrees.
            lon_deg (float): Longitude in degrees.
            alt_msl_m (float): Altitude above mean sea level in meters.

        Returns:
            lat_int (int): Latitude in 1E-7 degrees (MAVLink int32).
            lon_int (int): Longitude in 1E-7 degrees (MAVLink int32).
            alt_mm  (int): Altitude above MSL in millimeters (MAVLink int32).
        """
        lat_int = int(round(lat_deg * 1e7))
        lon_int = int(round(lon_deg * 1e7))
        alt_mm = int(round(alt_msl_m * 1000.0))
        return lat_int, lon_int, alt_mm

    def _fetch_latest_px4_control(self, blocking : bool = True):
        """Fetch the latest HIL_ACTUATOR_CONTROLS message from PX4 and update control inputs."""

        msg = self.conn.recv_match(type='HIL_ACTUATOR_CONTROLS', blocking=blocking, timeout=0.01)
        if msg is not None:
            return {'cmd_motor_speeds': [c * self.rotor_speed_max for c in msg.controls[:self.num_rotors]]}

    def _enu_to_ned_cmps(self, v_enu):
        v_n = float(v_enu[1])
        v_e = float(v_enu[0])
        v_d = float(-v_enu[2])
        return (
            int(np.round(v_n * 100.0)),
            int(np.round(v_e * 100.0)),
            int(np.round(v_d * 100.0)),
        )

    def _imu(self, state, statedot):
        meas = self.imu.measurement(state, statedot, with_noise=self._enable_imu_noise)
        a_flu = meas["accel"]
        omega_flu = meas["gyro"]
        # FLU -> FRD
        a_frd = np.array([a_flu[0], -a_flu[1], -a_flu[2]], dtype=float)
        omega_frd = np.array([omega_flu[0], -omega_flu[1], -omega_flu[2]], dtype=float)
        return a_frd, omega_frd

    def _send_hil_state_quaternion(self, state, statedot):
        """
        Send HIL_STATE_QUATERNION message to PX4.
        
        Args:
            state: Current vehicle state
            statedot: State derivative (from the `Multirotor.statedot` method)
        """
        # Convert cartesian ENU position to geodetic coordinates (latitude, longitude and height)
        lat_deg, lon_deg, height_meters = self.enu_to_geodetic(*state['x'])
        lat_e7, lon_e7, alt_mm = self.geodetic_to_mavlink(lat_deg, lon_deg, height_meters)
        
        # Convert quaternion from rotorpy to aerospace convention using ArduPilot's method
        quaternion_flu2ned = Ardupilot._quaternion_rotorpy_to_aerospace(state['q'])
        vx_cms, vy_cms, vz_cms = self._enu_to_ned_cmps(state['v'])

        # Send the ground truth acceleration in the state message (without imu noise)
        a_flu_gt = self.imu.measurement(state, statedot, with_noise=False)["accel"]
        a_frd_gt = np.array([a_flu_gt[0], -a_flu_gt[1], -a_flu_gt[2]], dtype=float)
        a_frd_mg = np.clip(np.round(a_frd_gt / 9.80665 * 1000.0), INT_MIN, INT_MAX).astype(np.int16)

        self.conn.mav.hil_state_quaternion_send(
            int(self.t * 1e6),
            quaternion_flu2ned,
            *tuple(state['w']),
            lat_e7, lon_e7, alt_mm,
            vx_cms, vy_cms, vz_cms,
            0, 0,  # Indicated airspeed, true airspeed
            int(a_frd_mg[0]), int(a_frd_mg[1]), int(a_frd_mg[2])
        )

    def _send_hil_sensor(self, state, statedot):
        """
        Send HIL_SENSOR message to PX4.
        
        Args:
            state: Current vehicle state
            statedot: State derivative (computed externally)
        """
        # Get IMU measurements
        a_ned, omega_ned = self._imu(state, statedot)

        # Only flag accel/gyro as updated (exclude mag and baro-related fields)
        updated_bitmask = SensorSource.ACCEL | SensorSource.GYRO

        self.conn.mav.hil_sensor_send(
            int(self.t * 1e6),
            *tuple(a_ned),
            *tuple(omega_ned),
            *(0.0, 0.0, 0.0),       # Magnetometer (body frame)
            0.0,                    # Abs pressure
            0,                      # Differential pressure
            0.0,                    # Altitude from pressure
            25,                     # Temperature
            fields_updated=updated_bitmask,
        )

    def step(self, state, control, t_step):        
        
        # Compute state derivative once for state and messages
        # and send both HIL messages
        statedot = self.statedot(state, control, 0.0)
        self._send_hil_state_quaternion(state, statedot)
        self._send_hil_sensor(state, statedot)

        # Use PX4 commands only if autopilot_controller is True
        if self._autopilot_controller:
            px4_control = self._fetch_latest_px4_control(blocking=self._lockstep_enabled)
            if px4_control is not None:
                control = px4_control
            else:
                control = {'cmd_motor_speeds': np.zeros(self.num_rotors)}

        else: # In this case we use the control provided by the external controller
            pass
        
        state = super().step(state, control, t_step)
        self.state = state
        self.t += t_step

        return state
