import numpy as np
import sys
import torch
import math
import random

"""
Below are some default wind objects that might be useful inputs to the system.  
"""

class NoWind(object):
    """
    This wind profile is the trivial case of no wind. It will output 
    zero wind speed on all axes for all time. 
    Alternatively, you can use ConstantWind with wx=wy=wz=0. 
    """

    def __init__(self):
        """
        Inputs: 
            Nothing
        """
        self.wind = np.array([0, 0, 0])

    def update(self, t, position):
        """
        Given the present time and position of the multirotor, return the
        current wind speed on all three axes. 
        
        The wind should be expressed in the world coordinates.
        """
        return self.wind

class BatchedNoWind(object):
    '''
    Batched version of NoWind that prepends a batch dimension to the output.
    '''
    def __init__(self, num_drones):
        self.wind = torch.zeros(num_drones, 3)

    def update(self, t, position):
        return self.wind

class ConstantWind(object):
    """
    This wind profile is constant both spatially and temporally. 
    Wind speed is specified on each axis. 
    """

    def __init__(self, wx, wy, wz):
        """
        """
        self.wind = np.array([wx, wy, wz])

        
    def update(self, t, position):
        """
        Given the present time and position of the multirotor, return the
        current wind speed on all three axes. 
        
        The wind should be expressed in the world coordinates. 
        """

        return self.wind

class BatchedConstantWind(object):
    """
    Batched version of ConstantWind that prepends a batch dimension to the output.
    """
    def __init__(self, num_drones, wx, wy, wz):
        self.wind = torch.tensor([wx, wy, wz]).repeat(num_drones, 1)

    def update(self, t, position):
        return self.wind


class SinusoidWind(object):
    """
    Wind will vary subject to a sine function with appropriate amplitude, frequency, and phase offset.
    """

    def __init__(self, amplitudes=np.array([1,1,1]), frequencies=np.array([1,1,1]), phase=np.array([0,0,0])):
        """
        Inputs: 
            amplitudes := array of amplitudes on each axis
            frequencies := array of frequencies for the wind pattern on each axis
            phase := relative phase offset on each axis in seconds
        """
        self.Ax, self.Ay, self.Az = amplitudes[0], amplitudes[1], amplitudes[2]
        self.fx, self.fy, self.fz = frequencies[0], frequencies[1], frequencies[2]
        self.px, self.py, self.pz = phase[0], phase[1], phase[2]

    def update(self, t, position):
        """
        Given the present time and position of the multirotor, return the
        current wind speed on all three axes. 
        
        The wind should be expressed in the world coordinates. 
        """

        wind = np.array([self.Ax*np.sin(2*np.pi*self.fx*(t+self.px)),
                         self.Ay*np.sin(2*np.pi*self.fy*(t+self.py)),
                         self.Az*np.sin(2*np.pi*self.fz*(t+self.pz))])

        return wind

class BatchedSinusoidWind(object):
    """
    Batched version of SinusoidWind that handles a batch of UAV, each with
    their own amplitude, frequency, and phase on each axis.
    """

    def __init__(self, amplitudes, frequencies, phases):
        """
        Inputs:
            amplitudes := tensor of shape (num_drones, 3) for amplitude per UAV per axis
            frequencies := tensor of shape (num_drones, 3) for frequency per UAV per axis
            phases := tensor of shape (num_drones, 3) for phase per UAV per axis (in seconds)
        """
        self.A = amplitudes
        self.f = frequencies
        self.p = phases

    def update(self, t, position):
        """
        Inputs:
            t := scalar float time
            position := tensor of shape (num_drones, 3), not used in this wind model
        Returns:
            wind := tensor of shape (num_drones, 3)
        """

        t_tensor = torch.full_like(self.p, t)
        wind = self.A * torch.sin(2*math.pi*self.f*(t_tensor + self.p))
        return wind

class LadderWind(object):
    """
    The wind will step up and down between a minimum and maximum speed for a specified duration. 
    Visualized below...

                     | | <- duration
     max ->          ---         ---
                  ---         ---
               ---         ---
     min -> ---         ---
            --------------------------> t
    
    ** Normally the wind will start at min and increase sequentially, but if the random flag is set true, 
       the wind will step to a random sublevel after each duration is up. 

    """

    def __init__(self, min=np.array([-1,-1,-1]), max=np.array([1,1,1]), duration=np.array([1,1,1]), Nstep=np.array([5,5,5]), random_flag=False):
        """
        Inputs:
            min := array of minimum wind speeds across each axis
            max := array of maximum wind speeds across each axis
            duration := array of durations for each step
            Nstep := array for the integer number of discretized steps between min and max across each axis
        """

        # Check the inputs for consistency, quit and raise a flag if the inputs aren't physically realizable
        if np.any(Nstep <= 0):
            print("LadderWind Error: The number of steps must be greater than or equal to 1")
            sys.exit(1)

        if np.any(max - min < 0):
            print("LadderWind Error: The max value must be greater than the min value.")
            sys.exit(1)

        self.random_flag = random_flag
        self.durx, self.dury, self.durz = duration[0], duration[1], duration[2]
        self.nx, self.ny, self.nz = Nstep[0], Nstep[1], Nstep[2]

        # Compute arrays of intermediate wind speeds for each axis
        self.wx_arr = np.linspace(min[0], max[0], self.nx)
        self.wy_arr = np.linspace(min[1], max[1], self.ny)
        self.wz_arr = np.linspace(min[2], max[2], self.nz)

        # Initialize the amplitude id.. these numbers are used to index the arrays above to get the appropriate wind speed on each axis
        if self.random_flag:
            self.xid = np.random.choice(self.nx)
            self.yid = np.random.choice(self.ny)
            self.zid = np.random.choice(self.nz)
        else:
            self.xid, self.yid, self.zid = 0, 0, 0

        # Initialize the timers... since we don't yet know the starting time, we'll set them in the first call
        self.timerx = None
        self.timery = None
        self.timerz = None

        # Initialize the winds
        self.wx, self.wy, self.wz = self.wx_arr[self.xid], self.wy_arr[self.yid], self.wz_arr[self.zid]

    def update(self, t, position):
        """
        Given the present time and position of the multirotor, return the
        current wind speed on all three axes. 
        
        The wind should be expressed in the world coordinates. 
        """
        if self.timerx is None:
            self.timerx, self.timery, self.timerz = t, t, t
        
        if (t - self.timerx) >= self.durx:
            if self.random_flag:
                self.xid = np.random.choice(self.nx)
            else:
                self.xid = (self.xid + 1) % self.nx
            self.wx = self.wx_arr[self.xid]
            self.timerx = t

        if (t - self.timery) >= self.dury:
            if self.random_flag:
                self.yid = np.random.choice(self.ny)
            else:
                self.yid = (self.yid + 1) % self.ny
            self.wy = self.wy_arr[self.yid]
            self.timery = t

        if (t - self.timerz) >= self.durz:
            if self.random_flag:
                self.zid = np.random.choice(self.nz)
            else:
                self.zid = (self.zid + 1) % self.nz
            self.wz = self.wz_arr[self.zid]
            self.timerz = t

        return np.array([self.wx, self.wy, self.wz])

class BatchedLadderWind(object):
    """
    Batched version of LadderWind. Each drone has its own wind ladder across all axes.
    """

    def __init__(self, min_vals, max_vals, durations, nsteps, random_flag=False):
        """
        Inputs:
            min_vals := tensor of shape (num_drones, 3)
            max_vals := tensor of shape (num_drones, 3)
            durations := tensor of shape (num_drones, 3)
            nsteps := tensor of shape (num_drones, 3), integer number of steps between min and max
        """
        self.random_flag = random_flag
        self.num_drones = min_vals.shape[0]  # Get number of UAVs

        if torch.any(nsteps <= 0):
            raise ValueError("LadderWind Error: The number of steps must be greater than or equal to 1")

        if torch.any(max_vals - min_vals < 0):
            raise ValueError("LadderWind Error: The max value must be greater than the min value.")

        self.durations = durations  # shape (num_drones, 3)
        self.nsteps = nsteps        # shape (num_drones, 3)

        # Precompute the ladder values for each axis per drone
        self.wx_ladders = [torch.linspace(min_vals[i, 0], max_vals[i, 0], int(nsteps[i, 0])) for i in range(self.num_drones)]
        self.wy_ladders = [torch.linspace(min_vals[i, 1], max_vals[i, 1], int(nsteps[i, 1])) for i in range(self.num_drones)]
        self.wz_ladders = [torch.linspace(min_vals[i, 2], max_vals[i, 2], int(nsteps[i, 2])) for i in range(self.num_drones)]

        # Initialize step indices
        if random_flag:
            self.xids = torch.tensor([random.randint(0, len(ladder) - 1) for ladder in self.wx_ladders])
            self.yids = torch.tensor([random.randint(0, len(ladder) - 1) for ladder in self.wy_ladders])
            self.zids = torch.tensor([random.randint(0, len(ladder) - 1) for ladder in self.wz_ladders])
        else:
            self.xids = torch.zeros(self.num_drones, dtype=torch.long)
            self.yids = torch.zeros(self.num_drones, dtype=torch.long)
            self.zids = torch.zeros(self.num_drones, dtype=torch.long)

        # Timers (initialized on first update)
        self.timers_initialized = False
        self.timerx = torch.zeros(self.num_drones)
        self.timery = torch.zeros(self.num_drones)
        self.timerz = torch.zeros(self.num_drones)

    def update(self, t, position):
        """
        t := scalar float time
        position := tensor of shape (num_drones, 3), unused
        Returns:
            wind := tensor of shape (num_drones, 3)
        """
        if not self.timers_initialized:
            self.timerx[:] = t
            self.timery[:] = t
            self.timerz[:] = t
            self.timers_initialized = True

        # Update indices if needed
        for i in range(self.num_drones):
            # X-axis
            if (t - self.timerx[i]) >= self.durations[i, 0]:
                if self.random_flag:
                    self.xids[i] = random.randint(0, len(self.wx_ladders[i]) - 1)
                else:
                    self.xids[i] = (self.xids[i] + 1) % len(self.wx_ladders[i])
                self.timerx[i] = t

            # Y-axis
            if (t - self.timery[i]) >= self.durations[i, 1]:
                if self.random_flag:
                    self.yids[i] = random.randint(0, len(self.wy_ladders[i]) - 1)
                else:
                    self.yids[i] = (self.yids[i] + 1) % len(self.wy_ladders[i])
                self.timery[i] = t

            # Z-axis
            if (t - self.timerz[i]) >= self.durations[i, 2]:
                if self.random_flag:
                    self.zids[i] = random.randint(0, len(self.wz_ladders[i]) - 1)
                else:
                    self.zids[i] = (self.zids[i] + 1) % len(self.wz_ladders[i])
                self.timerz[i] = t

        # Assemble wind vectors
        wx = torch.stack([self.wx_ladders[i][self.xids[i]] for i in range(self.num_drones)])
        wy = torch.stack([self.wy_ladders[i][self.yids[i]] for i in range(self.num_drones)])
        wz = torch.stack([self.wz_ladders[i][self.zids[i]] for i in range(self.num_drones)])

        return torch.stack([wx, wy, wz], dim=1)

if __name__ == "__main__":

    # ----- NoWind -----
    print("\n--- NoWind ---")
    wind = NoWind()
    print(wind.update(0, np.array([0, 0, 0])))

    print("\n--- BatchedNoWind ---")
    batched_wind = BatchedNoWind(num_drones=3)
    print(batched_wind.update(0, torch.zeros(3, 3)))

    # ----- ConstantWind -----
    print("\n--- ConstantWind ---")
    wind = ConstantWind(wx=1, wy=1, wz=1)
    print(wind.update(0, np.array([0, 0, 0])))

    # ----- SinusoidWind -----
    print("\n--- SinusoidWind ---")
    wind = SinusoidWind(amplitudes=np.array([1, 2, 3]), frequencies=np.array([0.5, 1.0, 1.5]), phase=np.array([0.5, 0.5, 0]))
    print(wind.update(1.0, np.array([0, 0, 0])))

    print("\n--- BatchedSinusoidWind ---")
    B = 3
    batched_wind = BatchedSinusoidWind(
        amplitudes=torch.tensor([[1.0, 2.0, 3.0]] * B),
        frequencies=torch.tensor([[0.5, 1.0, 1.5]] * B),
        phases=torch.tensor([[0.5, 0.5, 0.0]] * B),
    )
    print(batched_wind.update(1.0, torch.zeros(B, 3)))

    # ----- LadderWind -----
    print("\n--- LadderWind ---")
    wind = LadderWind(min=np.array([0, 0, 0]), max=np.array([1, 1, 1]), duration=np.array([0.5, 0.5, 0.5]), Nstep=np.array([5, 5, 5]), random_flag=False)
    print(wind.update(0.0, np.array([0, 0, 0])))
    print(wind.update(0.6, np.array([0, 0, 0])))

    print("\n--- BatchedLadderWind ---")
    B = 3
    batched_wind = BatchedLadderWind(
        min_vals=torch.tensor([[0.0, 0.0, 0.0]] * B),
        max_vals=torch.tensor([[1.0, 1.0, 1.0]] * B),
        durations=torch.tensor([[0.5, 0.5, 0.5]] * B),
        nsteps=torch.tensor([[5, 5, 5]] * B),
        random_flag=False,
    )
    print(batched_wind.update(0.0, torch.zeros(B, 3)))
    print(batched_wind.update(0.6, torch.zeros(B, 3)))