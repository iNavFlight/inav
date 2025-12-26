"""
Imports
"""
import numpy as np
import cvxopt
from scipy.linalg import block_diag
from typing import List
import torch

def cvxopt_solve_qp(P, q, G=None, h=None, A=None, b=None):
    """
    From https://scaron.info/blog/quadratic-programming-in-python.html . Infrastructure code for solving quadratic programs using CVXOPT. 
    The structure of the program is as follows: 

    min 0.5 xT P x + qT x
    s.t. Gx <= h
         Ax = b
    Inputs:
        P, numpy array, the quadratic term of the cost function
        q, numpy array, the linear term of the cost function
        G, numpy array, inequality constraint matrix
        h, numpy array, inequality constraint vector
        A, numpy array, equality constraint matrix
        b, numpy array, equality constraint vector
    Outputs:
        The optimal solution to the quadratic program
    """
    P = .5 * (P + P.T)  # make sure P is symmetric
    args = [cvxopt.matrix(P), cvxopt.matrix(q)]
    if G is not None:
        args.extend([cvxopt.matrix(G), cvxopt.matrix(h)])
    if A is not None:
        args.extend([cvxopt.matrix(A), cvxopt.matrix(b)])
    sol = cvxopt.solvers.qp(*args)
    if 'optimal' not in sol['status']:
        return None
    return np.array(sol['x']).reshape((P.shape[1],))

def H_fun(dt, k=7):
    """
    Computes the cost matrix for a single segment in a single dimension.
    *** Assumes that the decision variables c_i are e.g. x(t) = c_0 + c_1*t + c_2*t^2 + c_3*t^3 + c_4*t^4 + c_5*t^5 + .. + c_k*t^k
    Inputs:
        dt, scalar, the duration of the segment (t_(i+1) - t_i) 
        k, scalar, the order of the polynomial. 
    Outputs:
        H, numpy array, matrix containing the min snap cost function for that segment. Assumes the polynomial is at least order 5.
    """

    H = np.zeros((k+1,k+1))

    # TODO: implement automatic cost function based on the desired cost functional (snap or jerk) and the order of the polynomial, k. 

    seventh_order_cost = np.array([[576*dt, 1440*dt**2, 2880*dt**3, 5040*dt**4],
                                   [1440*dt**2, 4800*dt**3, 10800*dt**4, 20160*dt**5],
                                   [2880*dt**3, 10800*dt**4, 25920*dt**5, 50400*dt**6],
                                   [5040*dt**4, 20160*dt**5, 50400*dt**6, 100800*dt**7]])

    # Only take up to the (k+1) entries
    cost = seventh_order_cost[0:(k+1-4), 0:(k+1-4)]

    H[4:(k+1),4:(k+1)] = cost

    return H 

def get_1d_constraints(keyframes, delta_t, m, k=7, vmax=5, vstart=0, vend=0):
    """
    Computes the constraint matrices for the min snap problem. 
    *** Assumes that the decision variables c_i are e.g. o(t) = c_0 + c_1*t + c_2*t^2 + ... c_(k)*t^(k)

    We impose the following constraints FOR EACH SEGMENT m: 
        1) x_m(0) = keyframe[i]             # position at t = 0
        2) x_m(dt) = keyframe[i+1]          # position at t = dt
        3) v_m(0) = v_start                 # velocity at t = 0
        4) v_m(dt) = v_end                  # velocity at t = dt
        5) v_m(dt) = v_(m+1)(0)             # velocity continuity for interior segments
        6) a_m(dt) = a_(m+1)(0)             # acceleration continuity for interior segments
        7) j_m(dt) = j_(m+1)(0)             # jerk continuity for interior segments
        8) s_m(dt) = s_(m+1)(0)             # snap continuity for interior segments
        9) v_m(dt/2) <= vmax                # velocity constraint at midpoint of each segment

    For the first and last segment we impose: 
        1) a_0(0) = 0                       # acceleration at start of the trajectory is 0
        2) j_0(0) = 0                       # jerk at start of the trajectory is 0
        3) a_N(dt) = 0                      # acceleration at the end of the trajectory is 0
        4) j_N(dt) = 0                      # jerk at the end of the trajectory is 0

    Inputs:
        keyframes, numpy array, a list of m waypoints IN ONE DIMENSION (x,y,z, or yaw)
        delta_t, numpy array, the times between keyframes computed apriori. 
        m, int, the number of segments.
        k, int, the degree of the polynomial. 
        vmax, float, max speeds imposed at the midpoint of each segment. 
        vstart, float, the starting speed of the quadrotor. 
        vend, float, the ending speed of the quadrotor. 
    Outputs:
        A, numpy array, matrix of equality constraints (left side). 
        b, numpy array, array of equality constraints (right side).
        G, numpy array, matrix of inequality constraints (left side). 
        h, numpy array, array of inequality constraints (right side).

    """

    # The constraint matrices to be filled out.
    A = []
    b = []
    G = []
    h = []

    for i in range(m): # for each segment...
        # Gets the segment duration
        dt = delta_t[i]

        # Position continuity at the beginning of the segment
        A.append([0]*(k+1)*i + [1] + [0]*(k) + [0]*(k+1)*(m-i-1))
        b.append(keyframes[i])

        # Position continuity at the end of the segment
        A.append([0]*(k+1)*i + [dt**j for j in range(k+1)] + [0]*(k+1)*(m-i-1))
        b.append(keyframes[i+1])

        # Intermediate smoothness constraints
        if i < (m-1): # we don't want to include the last segment for this loop

            A.append([0]*(k+1)*i + [0] + [-j*dt**(j-1) for j in range(1, k+1)] + [0] + [j*(0)**(j-1) for j in range(1, k+1)] + [0]*(k+1)*(m-i-2))  # Velocity
            b.append(0)
            A.append([0]*(k+1)*i + [0]*2 + [-(j-1)*j*dt**(j-2) for j in range(2, k+1)] + [0]*2 + [(j-1)*j*(0)**(j-2) for j in range(2, k+1)] + [0]*(k+1)*(m-i-2))  # Acceleration
            b.append(0)
            A.append([0]*(k+1)*i + [0]*3 + [-(j-2)*(j-1)*j*dt**(j-3) for j in range(3, k+1)] + [0]*3 + [(j-2)*(j-1)*j*(0)**(j-3) for j in range(3, k+1)] + [0]*(k+1)*(m-i-2))  # Jerk
            b.append(0)
            A.append([0]*(k+1)*i + [0]*4 + [-(j-3)*(j-2)*(j-1)*j*dt**(j-4) for j in range(4, k+1)] + [0]*4 + [(j-3)*(j-2)*(j-1)*j*(0)**(j-4) for j in range(4, k+1)] + [0]*(k+1)*(m-i-2))  # Snap
            b.append(0)

        # Inequality constraints 
        G.append([0]*(k+1)*i + [0] + [j*(0.5*dt)**(j-1) for j in range(1, k+1)] + [0]*(k+1)*(m-i-1))  # Velocity constraint at midpoint
        h.append(vmax)

    A.append([0] + [j*(0)**(j-1) for j in range(1, k+1)] + [0]*(k+1)*(m-1))              # Velocity at start
    b.append(vstart)
    A.append([0]*(k+1)*(m-1) + [0] + [j*(dt)**(j-1) for j in range(1, k+1)])              # Velocity at end
    b.append(vend)
    A.append([0]*2 + [(j-1)*j*(0)**(j-2) for j in range(2, k+1)] + [0]*(k+1)*(m-1))        # Acceleration = 0 at start
    b.append(0)
    A.append([0]*(k+1)*(m-1) + [0]*2 + [(j-1)*j*(dt)**(j-2) for j in range(2, k+1)])       # Acceleration = 0 at end
    b.append(0)
    A.append([0]*3 + [(j-2)*(j-1)*j*(0)**(j-3) for j in range(3, k+1)]  + [0]*(k+1)*(m-1))        # Jerk = 0 at start
    b.append(0)
    A.append([0]*(k+1)*(m-1) + [0]*3 + [(j-2)*(j-1)*j*(dt)**(j-3) for j in range(3, k+1)])       # Jerk = 0 at end
    b.append(0)

    # Convert to numpy arrays and ensure floats to work with cvxopt. 
    A = np.array(A).astype(float)
    b = np.array(b).astype(float)
    G = np.array(G).astype(float)
    h = np.array(h).astype(float)

    return (A, b, G, h)

class MinSnap(object):
    """
    MinSnap generates a minimum snap trajectory for the quadrotor, following https://ieeexplore.ieee.org/document/5980409. 
    The trajectory is a piecewise 7th order polynomial (minimum degree necessary for snap optimality). 
    """
    def __init__(self, points, yaw_angles=None, yaw_rate_max=2*np.pi, 
                poly_degree=7, yaw_poly_degree=7,
                v_max=3, v_avg=1, v_start=[0, 0, 0], v_end=[0, 0, 0],
                verbose=True):
        """
        Waypoints and yaw angles compose the "keyframes" for optimizing over. 
        Inputs:
            points, numpy array of m 3D waypoints. 
            yaw_angles, numpy array of m yaw angles corresponding to each waypoint. 
            yaw_rate_max, the maximum yaw rate in rad/s
            v_avg, the average speed between waypoints, this is used to do the time allocation as well as impose constraints at midpoint of each segment. 
            v_start, the starting velocity vector given as an array [x_dot_start, y_dot_start, z_dot_start]
            v_end, the ending velocity vector given as an array [x_dot_end, y_dot_end, z_dot_end]
            verbose, determines whether or not the QP solver will output information. 
        """

        if poly_degree != 7 or yaw_poly_degree != 7:
            raise NotImplementedError("Oops, we haven't implemented cost functions for polynomial degree != 7 yet.")

        if yaw_angles is None:
            self.yaw = np.zeros((points.shape[0]))
        else:
            self.yaw = yaw_angles
        self.v_avg = v_avg

        cvxopt.solvers.options['show_progress'] = verbose

        # Compute the distances between each waypoint.
        seg_dist = np.linalg.norm(np.diff(points, axis=0), axis=1)
        seg_mask = np.append(True, seg_dist > 1e-1)
        self.points = points[seg_mask,:]

        self.null = False

        m = self.points.shape[0]-1  # Get the number of segments

        # Compute the derivatives of the polynomials
        self.x_dot_poly    = np.zeros((m, 3, poly_degree))
        self.x_ddot_poly   = np.zeros((m, 3, poly_degree-1))
        self.x_dddot_poly  = np.zeros((m, 3, poly_degree-2))
        self.x_ddddot_poly = np.zeros((m, 3, poly_degree-3))
        self.yaw_dot_poly = np.zeros((m, 1, yaw_poly_degree))
        self.yaw_ddot_poly = np.zeros((m, 1, yaw_poly_degree-1))
        
        # If two or more waypoints remain, solve min snap
        if self.points.shape[0] >= 2:

            ################## Time allocation
            self.delta_t = seg_dist/self.v_avg # Compute the segment durations based on the average velocity
            self.t_keyframes = np.concatenate(([0], np.cumsum(self.delta_t)))  # Construct time array which indicates when the quad should be at the i'th waypoint. 

            ################## Cost function
            # First get the cost segment for each matrix: 
            H_pos = [H_fun(self.delta_t[i], k=poly_degree) for i in range(m)]
            H_yaw = [H_fun(self.delta_t[i], k=yaw_poly_degree) for i in range(m)]

            # Now concatenate these costs using block diagonal form:
            P_pos = block_diag(*H_pos)
            P_yaw = block_diag(*H_yaw)

            # Lastly the linear term in the cost function is 0
            q_pos = np.zeros(((poly_degree+1)*m,1))
            q_yaw = np.zeros(((yaw_poly_degree+1)*m, 1))
            
            ################## Constraints for each axis
            (Ax,bx,Gx,hx) = get_1d_constraints(self.points[:,0], self.delta_t, m, k=poly_degree, vmax=v_max, vstart=v_start[0], vend=v_end[0])
            (Ay,by,Gy,hy) = get_1d_constraints(self.points[:,1], self.delta_t, m, k=poly_degree, vmax=v_max, vstart=v_start[1], vend=v_end[1])
            (Az,bz,Gz,hz) = get_1d_constraints(self.points[:,2], self.delta_t, m, k=poly_degree, vmax=v_max, vstart=v_start[2], vend=v_end[2])
            (Ayaw,byaw,Gyaw,hyaw) = get_1d_constraints(self.yaw, self.delta_t, m, k=yaw_poly_degree, vmax=yaw_rate_max)

            ################## Solve for x, y, z, and yaw

            ### Only in the fully constrained situation is there a unique minimum s.t. we can solve the system Ax = b.
            # c_opt_x = np.linalg.solve(Ax,bx)
            # c_opt_y = np.linalg.solve(Ay,by)
            # c_opt_z = np.linalg.solve(Az,bz)
            # c_opt_yaw = np.linalg.solve(Ayaw,byaw)

            ### Otherwise, in the underconstrained case or when inequality constraints are given we solve the QP. 
            c_opt_x = cvxopt_solve_qp(P_pos, q=q_pos, G=Gx, h=hx, A=Ax, b=bx)
            c_opt_y = cvxopt_solve_qp(P_pos, q=q_pos, G=Gy, h=hy, A=Ay, b=by)
            c_opt_z = cvxopt_solve_qp(P_pos, q=q_pos, G=Gz, h=hz, A=Az, b=bz)
            c_opt_yaw = cvxopt_solve_qp(P_yaw, q=q_yaw, G=Gyaw, h=hyaw, A=Ayaw, b=byaw)

            ################## Construct polynomials from c_opt
            self.x_poly = np.zeros((m, 3, (poly_degree+1)))
            self.yaw_poly = np.zeros((m, 1, (yaw_poly_degree+1)))
            for i in range(m):
                self.x_poly[i,0,:] = np.flip(c_opt_x[(poly_degree+1)*i:((poly_degree+1)*i+(poly_degree+1))])
                self.x_poly[i,1,:] = np.flip(c_opt_y[(poly_degree+1)*i:((poly_degree+1)*i+(poly_degree+1))])
                self.x_poly[i,2,:] = np.flip(c_opt_z[(poly_degree+1)*i:((poly_degree+1)*i+(poly_degree+1))])
                self.yaw_poly[i,0,:] = np.flip(c_opt_yaw[(yaw_poly_degree+1)*i:((yaw_poly_degree+1)*i+(yaw_poly_degree+1))])

            for i in range(m):
                for j in range(3):
                    self.x_dot_poly[i,j,:]    = np.polyder(self.x_poly[i,j,:], m=1)
                    self.x_ddot_poly[i,j,:]   = np.polyder(self.x_poly[i,j,:], m=2)
                    self.x_dddot_poly[i,j,:]  = np.polyder(self.x_poly[i,j,:], m=3)
                    self.x_ddddot_poly[i,j,:] = np.polyder(self.x_poly[i,j,:], m=4)
                self.yaw_dot_poly[i,0,:] = np.polyder(self.yaw_poly[i,0,:], m=1)
                self.yaw_ddot_poly[i,0,:] = np.polyder(self.yaw_poly[i,0,:], m=2)

        else:
            # Otherwise, there is only one waypoint so we just set everything = 0. 
            self.null = True
            m = 1
            self.T = np.zeros((m,))
            self.x_poly = np.zeros((m, 3, 6))
            self.x_poly[0,:,-1] = points[0,:]

    def update(self, t):
        """
        Given the present time, return the desired flat output and derivatives.

        Inputs
            t, time, s
        Outputs
            flat_output, a dict describing the present desired flat outputs with keys
                x,        position, m
                x_dot,    velocity, m/s
                x_ddot,   acceleration, m/s**2
                x_dddot,  jerk, m/s**3
                x_ddddot, snap, m/s**4
                yaw,      yaw angle, rad
                yaw_dot,  yaw rate, rad/s
        """
        x = np.zeros((3,))
        x_dot = np.zeros((3,))
        x_ddot = np.zeros((3,))
        x_dddot = np.zeros((3,))
        x_ddddot = np.zeros((3,))
        yaw    = 0
        yaw_dot = 0
        yaw_ddot = 0

        if self.null:
            # If there's only one waypoint
            x    = self.points[0,:]
            yaw = self.yaw[0]
        else:
            # Find interval index i and time within interval t.
            t = np.clip(t, self.t_keyframes[0], self.t_keyframes[-1])
            for i in range(self.t_keyframes.size-1):
                if self.t_keyframes[i] + self.delta_t[i] >= t:
                    break
            t = t - self.t_keyframes[i]

            # Evaluate polynomial.
            for j in range(3):
                x[j]        = np.polyval(       self.x_poly[i,j,:], t)
                x_dot[j]    = np.polyval(   self.x_dot_poly[i,j,:], t)
                x_ddot[j]   = np.polyval(  self.x_ddot_poly[i,j,:], t)
                x_dddot[j]  = np.polyval( self.x_dddot_poly[i,j,:], t)
                x_ddddot[j] = np.polyval(self.x_ddddot_poly[i,j,:], t)

            yaw = np.polyval(self.yaw_poly[i, 0, :], t)
            yaw_dot = np.polyval(self.yaw_dot_poly[i,0,:], t)
            yaw_ddot = np.polyval(self.yaw_ddot_poly[i,0,:], t)

        flat_output = { 'x':x, 'x_dot':x_dot, 'x_ddot':x_ddot, 'x_dddot':x_dddot, 'x_ddddot':x_ddddot,
                        'yaw':yaw, 'yaw_dot':yaw_dot, 'yaw_ddot':yaw_ddot}
        return flat_output


class BatchedMinSnap:
    '''
    This is a batched version of the MinSnap trajectory class above.
    It is designed to act as a wrapper around existing MinSnap objects, so if you want to generate minsnap trajectories, use the MinSnap class
        and then pass a list of resulting objects to this class.

    Simultaneously samples multiple MinSnap reference trajectories at a given time using vectorized ops.
    If you have a lot of trajectories, this is a lot faster iterating through each of them. For this class,
    the trajectories must have the same number of spline segments and same polynomial degree.
    '''
    def __init__(self, list_of_trajectories: List, device):
        delta_ts = []
        t_keyframes = []
        xs = []
        x_dots = []
        x_ddots = []
        yaws = []
        yaw_dots = []
        for i, traj in enumerate(list_of_trajectories):
            delta_ts.append(traj.delta_t)
            t_keyframes.append(traj.t_keyframes)
            xs.append(traj.x_poly)
            x_dots.append(traj.x_dot_poly)
            x_ddots.append(traj.x_ddot_poly)
            yaws.append(traj.yaw_poly)
            yaw_dots.append(traj.yaw_dot_poly)
            assert(traj.x_poly.shape[-1] == 8)
            if i == 0:
                num_segments = len(traj.t_keyframes)
            assert(len(traj.t_keyframes) == num_segments)

        self.delta_ts = np.array(delta_ts)
        self.t_keyframes = np.array(t_keyframes)
        self.t_keyframes_summed = np.array(t_keyframes)
        self.t_keyframes_summed[...,:-1] += self.delta_ts
        self.xs = torch.from_numpy(np.array(xs)).to(device, non_blocking=True)
        self.x_dots = torch.from_numpy(np.array(x_dots)).to(device, non_blocking=True)
        self.x_ddots = torch.from_numpy(np.array(x_ddots)).to(device, non_blocking=True)
        self.yaws = torch.from_numpy(np.array(yaws)).to(device, non_blocking=True)
        self.yaw_dots = torch.from_numpy(np.array(yaw_dots)).to(device, non_blocking=True)


        self.device = device

        self.num_trajs = len(list_of_trajectories)

    def batch_polyval(self, polynomial_coeffs, ts_powers):
        '''
        Evaluate multiple polynomials of the same degree at once
        polynomial_coeffs: torch.Tensor of shape (batch_dim, num_axes, poly_degree)
        ts: torch.Tensor of shape (batch_dim, poly_degree)
        '''
        result = torch.sum(polynomial_coeffs * ts_powers, dim=-1)
        return result

    def update(self, t: np.ndarray):
        '''
        Evaluates trajectory [i] at time t[i], and returns the flat_outputs in the same format as RotorPy.
        Outputs:
            flat_output, a dict describing the present desired flat outputs with same keys as `MinSnap`. Entries in the dictionary are 
                torch tensors of shape (num_trajs, 3) for x, x_dot, x_ddot, and (num_trajs,) for yaw and yaw_dot.
        '''
        segment_idxs = torch.zeros(self.num_trajs).int()
        ts = torch.zeros(self.num_trajs)
        t_tmp = np.clip(t, self.t_keyframes[:,0], self.t_keyframes[:,-1])
        for i in range(self.num_trajs):
            for j in range(self.t_keyframes.shape[1]):
                if self.t_keyframes_summed[i,j] >= t_tmp[i]:
                    segment_idxs[i] = j
                    ts[i] = t_tmp[i] - self.t_keyframes[i,j]
                    break
        segment_idxs = segment_idxs.to(self.device, non_blocking=True).long()
        ts = ts.to(self.device, non_blocking=True)


        ts_expd = torch.cat([(ts**i).unsqueeze(-1) for i in reversed(range(0, 8))], dim=-1).unsqueeze(1)
        batch_idxs = torch.arange(self.num_trajs, device=self.device)
        x_out = self.batch_polyval(self.xs[batch_idxs.long(),segment_idxs], ts_expd).double()
        x_dot_out = self.batch_polyval(self.x_dots[batch_idxs.long(),segment_idxs], ts_expd[...,1:]).double()
        x_ddot_out = self.batch_polyval(self.x_ddots[batch_idxs.long(),segment_idxs], ts_expd[...,2:]).double()
        yaw_out = self.batch_polyval(self.yaws[batch_idxs.long(),segment_idxs], ts_expd).squeeze(-1).double()
        yaw_dots_out = self.batch_polyval(self.yaw_dots[batch_idxs.long(),segment_idxs], ts_expd[...,1:]).squeeze(-1).double()

        flat_output = {'x':x_out, 'x_dot':x_dot_out, 'x_ddot':x_ddot_out,
                       'yaw':yaw_out, 'yaw_dot':yaw_dots_out}
        return flat_output


if __name__=="__main__":

    import matplotlib.pyplot as plt
    from matplotlib import cm

    waypoints = np.array([[0.00, 0.00, 0.00],
                          [1.00, 0.00, 0.25],
                          [1.00, 1.00, 0.50],
                          [0.00, 1.00, 1.00],
                          [0.00, 2.00, 1.25],
                          [2.00, 2.00, 1.50]
                          ])
    yaw_angles = np.array([0, np.pi/2, 0, np.pi/4, 3*np.pi/2, 0])
    v_avg = 2                                                       # Average velocity, used for time allocation
    v_start = [0, 0, 0]                                             # Start (x,y,z) velocity
    v_end = [0, 0, 0]                                               # End (x,y,z) velocity.

    traj = MinSnap(waypoints, yaw_angles, v_max=5, v_avg=v_avg, v_start=v_start, v_end=v_end, verbose=True)
    t_keyframes = traj.t_keyframes

    N = 1000
    time = np.linspace(0,1.1*t_keyframes[-1],N)
    x = np.zeros((N, 3))
    xdot = np.zeros((N,3))
    xddot = np.zeros((N,3))
    xdddot = np.zeros((N,3))
    xddddot = np.zeros((N,3))
    yaw = np.zeros((N,))
    yaw_dot = np.zeros((N,))

    for i in range(N):
        flat = traj.update(time[i])

        x[i,:] = flat['x']
        xdot[i,:] = flat['x_dot']
        xddot[i,:] = flat['x_ddot']
        xdddot[i,:] = flat['x_dddot']
        xddddot[i,:] = flat['x_ddddot']

        yaw[i] = flat['yaw']
        yaw_dot[i] = flat['yaw_dot']

    t_keyframes = traj.t_keyframes

    (fig, axes) = plt.subplots(nrows=5, ncols=1, sharex=True, num="Translational Flat Outputs")
    ax = axes[0]
    ax.plot(time, x[:,0], 'r-', label="X")
    ax.plot(time, x[:,1], 'g-', label="Y")
    ax.plot(time, x[:,2], 'b-', label="Z")
    ax.plot(t_keyframes, waypoints, 'ko')
    ax.legend()
    ax.set_ylabel("x")
    ax = axes[1]
    ax.plot(time, xdot[:,0], 'r-', label="X")
    ax.plot(time, xdot[:,1], 'g-', label="Y")
    ax.plot(time, xdot[:,2], 'b-', label="Z")
    ax.set_ylabel("xdot")
    ax = axes[2]
    ax.plot(time, xddot[:,0], 'r-', label="X")
    ax.plot(time, xddot[:,1], 'g-', label="Y")
    ax.plot(time, xddot[:,2], 'b-', label="Z")
    ax.set_ylabel("xddot")
    ax = axes[3]
    ax.plot(time, xdddot[:,0], 'r-', label="X")
    ax.plot(time, xdddot[:,1], 'g-', label="Y")
    ax.plot(time, xdddot[:,2], 'b-', label="Z")
    ax.set_ylabel("xdddot")
    ax = axes[4]
    ax.plot(time, xddddot[:,0], 'r-', label="X")
    ax.plot(time, xddddot[:,1], 'g-', label="Y")
    ax.plot(time, xddddot[:,2], 'b-', label="Z")
    ax.set_ylabel("xddddot")
    ax.set_xlabel("Time, s")

    (fig, axes) = plt.subplots(nrows=2, ncols=1, sharex=True, num="Yaw vs Time")
    ax = axes[0]
    ax.plot(time, yaw, 'k', label="yaw")
    ax.plot(t_keyframes, yaw_angles, 'ro')
    ax.set_ylabel("yaw")
    ax = axes[1]
    ax.plot(time, yaw_dot, 'k', label="yaw")
    ax.set_ylabel("yaw dot")
    ax.set_xlabel("Time, s")

    speed = np.sqrt(xdot[:,0]**2 + xdot[:,1]**2)
    fig = plt.figure(num="XY Trajectory")
    ax = fig.add_subplot(projection='3d')
    s = ax.scatter(x[:,0], x[:,1], x[:,2], c=cm.winter(speed/speed.max()), s=4, label="Flat Output")
    ax.plot(waypoints[:,0], waypoints[:,1], waypoints[:,2], 'ko', markersize=10, label="Waypoints")
    ax.grid()
    ax.legend()

    plt.show()
