import os, sys, time
import socket

from rotorpy.environments import Environment
from rotorpy.vehicles.multirotor import Multirotor
from rotorpy.vehicles.hummingbird_params import quad_params
from rotorpy.sensors.imu import Imu

import numpy as np
from scipy.spatial.transform import Rotation as R
import src.sim_utils as sutil


quad_params['mass'] = 0.500           # kg
quad_params['Ixx']  = 10.0*3.65e-3    # kg*m^2
quad_params['Iyy']  = 10.0*3.68e-3    # kg*m^2
quad_params['Izz']  = 10.0*7.03e-3    # kg*m^2


class InavSimulate:
    
    def __init__(self, run_isolated=False):

        self.vehicle=Multirotor(quad_params)
        self.sim_state = self.vehicle.initial_state
        
        self.imu = Imu()
        self._enable_imu_noise = True  # Always add a bit of noise to avoid stale detection

        self.Nav = sutil.NavProj(
            46.631361,
            16.177206,
            185.0
        )
        
        
        self.cmd_motor_speeds = [
            0,0,0,0
        ]
        
        # Update once
        self.sim_state = self.vehicle.step(
            self.sim_state, {'cmd_motor_speeds': self.cmd_motor_speeds}, 0)
        
        self.state = {
            "trel": 0,
            
            "lat": self.Nav.lat0,
            "lon": self.Nav.lon0,
            "alt": self.Nav.alt0,
            "trk": 0,
            "hdg": 0,
            
            "posx": 0,
            "posy": 0,
            "posz": 0,
            "gvel": 0,
           
            "roll": 0,
            "pitch": 0,
            "yaw": 0,
            
            "ax": 0,
            "ay": 0,
            "az": 0,
            
            "p": 0,
            "q": 0,
            "r": 0,
            
            
            "ch1": 0,
            "ch2": 0,
            "ch3": 0,
            "ch4": 0,
            "ch5": 0,
            "ch6": 0,
            "ch7": 0,
            "ch8": 0,

        }
        
        self.msg = ""

        self.__moveDrone = False
        self.__isolatedRun = False # If no inav is online
        
        if run_isolated:
            self.__moveDrone = True
            self.__isolatedRun = True
                      
            self.cmd_motor_speeds[0] = 475 + 0 
            self.cmd_motor_speeds[1] = 475 + 0.1    
            self.cmd_motor_speeds[2] = 475 + 0.1   
            self.cmd_motor_speeds[3] = 475  
            
        self.__lastTime = None
        

    def __updateState(self, k, v, operation=""):
        
        if k not in self.state:
            raise RuntimeError (f"Key {k} not a member of self.state!")
        

        if operation == "":
            self.state[k] = v
        
        elif operation == "+":
            self.state[k] += v
            
        else:
            raise RuntimeError (f"Unknown operation: {operation}")
    
    
    def _imu(self, state, statedot):
        meas = self.imu.measurement(state, statedot, with_noise=self._enable_imu_noise)
        a_flu = meas["accel"]
        omega_flu = meas["gyro"]
        # FLU -> FRD
        a_frd = np.array([a_flu[0], -a_flu[1], -a_flu[2]], dtype=float)
        omega_frd = np.array([omega_flu[0], -omega_flu[1], -omega_flu[2]], dtype=float)
        return a_frd, omega_frd
    

    def update(self, trel:float):
        
        if self.__lastTime is None:
            self.__lastTime = 0.0
        
        dt  = trel - self.__lastTime
        
        a_ned, omega_ned = np.zeros(3,), np.zeros(3,)
        if self.__moveDrone or self.__isolatedRun:
            self.sim_state = self.vehicle.step(self.sim_state, {'cmd_motor_speeds': self.cmd_motor_speeds}, dt)
            a_ned, omega_ned = self._imu(self.sim_state, self.vehicle.s_dot)
            
        state = self.sim_state

        lat, lon, alt, trk = self.Nav.to_geodetic(
            state["x"][0], 
            state["x"][1],
            state["x"][2]
        )

        gvel = (state["v"][0]**2 + state["v"][1]**2)**0.5
                
        r = R.from_quat(state["q"])
        roll, pitch, yaw = r.as_euler('xyz', degrees=True)
        

        # Update state for comms 
        # --------------------------------------- #
        
        self.__updateState("trel",  trel)
        
        if not self.__isolatedRun:
            
            self.__updateState("ch3",  0)
            self.__updateState("ch5",  0)
            self.__updateState("ch6", -1)
            
            if trel > 9.0:
                self.__updateState("ch5",  1.0) # Arm
            
            if trel > 10.0:
                self.__updateState("ch3",  0.85) # Add power
                #self.__updateState("ch6",  0.00) # Angle mode
                #self.__updateState("ch6",  0.75) # Angle mode + Alt Hold
                self.__moveDrone = True
                
            if trel > 14.0:
                self.__updateState("ch6",  0.75) # Angle mode + Alt Hold + WP
                self.__updateState("ch2",  0.5) 
                
            if trel > 20.0:
                self.__updateState("ch2",  0.0) 
                
            if trel > 21.0:
                self.__updateState("ch6", -0.75)
                self.__updateState("ch7",  0.75) # WP Mode
                
                val = 0.2*np.sin(trel*0.05)
                #self.__updateState("ch1",  val)

            
        # States

        self.__updateState("lat", lat)
        self.__updateState("lon", lon)
        self.__updateState("alt", alt)
            
        # Don't update trk if too slow
        #
        if gvel > 1.0:
            self.__updateState("trk", trk)
            #self.__updateState("hdg", trk)
        
        self.__updateState("posx", state["x"][0])
        self.__updateState("posy", state["x"][1])
        self.__updateState("posz", state["x"][2])
        self.__updateState("gvel", gvel) 

        self.__updateState("roll", roll)
        self.__updateState("pitch", -pitch)
        self.__updateState("yaw", -yaw + 90.0)
        self.__updateState("hdg", -yaw + 90.0)
        self.__updateState("ax", a_ned[0] / 9.80665)
        self.__updateState("ay", -a_ned[1] / 9.80665)
        self.__updateState("az", -a_ned[2] / 9.80665)
        self.__updateState("p", omega_ned[0] * np.rad2deg(1.0))
        self.__updateState("q", omega_ned[1] * np.rad2deg(1.0))
        self.__updateState("r", omega_ned[2] * np.rad2deg(1.0))
    

        
        # Debug ------------------------- #

        #print("Time:", trel)
        #for k,v in state.items():
        #    if k != "q" and k != "rotor_speeds":
        #        sutil.print_vec(v, k)
        #
        #sutil.print_vec(np.array([roll, pitch, yaw]), "ypr")
        
        for k, v in self.state.items():
            print(f"{k:>12} = {v:12.6f}")

        for i, spd in enumerate(state["rotor_speeds"]):
            txt = f"motor{i}"
            print(f"{txt:>12} = {spd:12.6f}")
        print("# ----------------------------------- #")
        
        self.__lastTime = trel
        
        return self.state
    
    
    
    
    def rx(self, conn:socket.socket):
        
        data = conn.recv(1024)  # receive up to 1024 bytes
        if not data:
            return
        
        GAIN = 800.0
        
        line = data.decode("utf-8").strip() 
        vals = line.split(";")
        if len(vals) >= 4:
            self.cmd_motor_speeds[0] = GAIN * float(vals[3])   # Motor FL
            self.cmd_motor_speeds[1] = GAIN * float(vals[1])   # Motor FR
            self.cmd_motor_speeds[2] = GAIN * float(vals[0])   # Motor RR
            self.cmd_motor_speeds[3] = GAIN * float(vals[2])   # Motor RL
            
            self.cmd_motor_speeds[0] *= 1.000   
            self.cmd_motor_speeds[1] *= 1.000     
            self.cmd_motor_speeds[2] *= 1.001    
            self.cmd_motor_speeds[3] *= 1.001   
    
    
    def tx(self, conn:socket.socket):
        
        def appendToMsg(key):
            val = self.state[key]
            self.msg += f"{val:.6f};"
            
            
        self.msg = ""
        
        
        appendToMsg("trel")         #0        
        appendToMsg("lat")          #1
        appendToMsg("lon")          #2
        appendToMsg("alt")          #3
        appendToMsg("trk")          #4
        appendToMsg("hdg")          #5
        appendToMsg("posx")         #6
        appendToMsg("posy")         #7
        appendToMsg("posz")         #8
        appendToMsg("gvel")         #9
        appendToMsg("roll")         #10
        appendToMsg("pitch")        #11
        appendToMsg("yaw")          #12
        
        appendToMsg("ax")           #13
        appendToMsg("ay")           #14
        appendToMsg("az")           #15
        appendToMsg("p")            #16
        appendToMsg("q")            #17
        appendToMsg("r")            #18
        
        
        
        # Channels
        appendToMsg("ch1")          #19
        appendToMsg("ch2")          #20              
        appendToMsg("ch3")          #21
        appendToMsg("ch4")          #22
        appendToMsg("ch5")          #23
        appendToMsg("ch6")          #24
        appendToMsg("ch7")          #25
        appendToMsg("ch8")          #26
            

        # Finish line
        msg = self.msg.rsplit(";", 1)[0] + "\n"
        
        #print("msg:", msg)

        conn.sendall(msg.encode("utf-8"))