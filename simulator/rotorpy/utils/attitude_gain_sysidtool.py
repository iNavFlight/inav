import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import StateSpace, lsim
from matplotlib.widgets import Slider, TextBox
from rotorpy.vehicles.crazyfliebrushless_params import quad_params, d

""" 
The point of this script is to provide an interactive tool to gain insight into approximately how the lower level attitude gains
(kp_att and kd_att) affect the controller response (control effort, nat freq, damping ratio, settling time) for the closed loop attitude tracking 
using a simplified model of the attitude dynamics.
"""
def interactive_pd_homogeneous_fixed():
    def compute_response(kp, kd, x0, inertia):
        """ 
        Given set values for the gains, the initial angle error (rad), and inertia, simulate the 
        attitude dynamics using a linear second order system. 
        """ 
        if kp <= 0 or kd < 0 or inertia <= 0:
            return None, None, None, None, None

        wn = np.sqrt(kp)
        zeta = kd / (2 * wn)
        wd = wn * np.sqrt(1 - zeta**2) if zeta < 1 else 0

        # State-space: x1 = position, x2 = velocity
        A = np.array([[0, 1],
                      [-kp, -kd]])
        B = np.array([[0], [0]])  # No external input
        C = np.array([[1, 0]])
        D = np.array([[0]])

        sys = StateSpace(A, B, C, D)
        t = np.linspace(0, 0.5, 1000)
        u = np.zeros_like(t)  # no external input
        x0_vec = [x0, 0]  # position error, zero initial velocity

        _, y, x = lsim(sys, U=u, T=t, X0=x0_vec)

        return wn, zeta, wd, t, y.squeeze(), x

    def compute_settling_time(t, y, threshold=0.02):
        """
        Compute the time it takes for the response to settle based on the 2% rule.  
        """
        if len(y) == 0:
            return t[-1]
        steady_state = 0.0
        within_bounds = np.abs(y - steady_state) <= threshold * np.abs(y[0])
        for i in reversed(range(len(y))):
            if not within_bounds[i]:
                return t[i + 1] if i + 1 < len(t) else t[-1]
        return t[-1]

    def compute_control_effort(kp, kd, x, inertia):
        """ 
        Back out the expected control effort from this controller based on the angle and body rate errors. 
        """ 
        u = -inertia * (kp * x[:, 0] + kd * x[:, 1])
        return u

    # Initial values (start with nominal CFBL gains)
    kp0 = 2624.0
    kd0 = 360.0
    x0 = 0.785398  # 45 deg error in attitude
    inertia0 = quad_params['Ixx']  # Inertia in the roll axis
    max_moment = 2*quad_params['k_eta']*(quad_params['rotor_speed_max']**2)*d*0.7071  # Maximum roll moment from quad params. Assuming 2 motors are at max speed and the other 2 are off. 

    # Get the system response and characteristics based on the initial gains/error. 
    wn, zeta, wd, t, y, x = compute_response(kp0, kd0, x0, inertia0)
    settling_time = compute_settling_time(t, y)
    u = compute_control_effort(kp0, kd0, x, inertia0)

    # Plot setup
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 10))
    plt.subplots_adjust(bottom=0.3)

    response_line, = ax1.plot(t, y, lw=2, label="Homogeneous Response")
    st_line = ax1.axvline(settling_time, color='r', linestyle='--', label="Settling Time")
    info_text = ax1.text(0.02, 0.95, '', transform=ax1.transAxes, verticalalignment='top',
                         fontsize=10, bbox=dict(boxstyle="round", facecolor="white", alpha=0.7))
    ax1.set_title("PD Homogeneous Response (Initial Error)")
    ax1.set_xlabel("Time [s]")
    ax1.set_ylabel("Position")
    ax1.grid(True)
    ax1.legend(loc="lower right")

    effort_line, = ax2.plot(t, u, color='orange', lw=2, label="Control Effort")
    ax2.axhline(max_moment, color='gray', linestyle='--', label="Max Moment")
    ax2.axhline(-max_moment, color='gray', linestyle='--')
    ax2.set_title("Control Effort")
    ax2.set_xlabel("Time [s]")
    ax2.set_ylabel("Torque")
    ax2.grid(True)
    ax2.legend(loc="upper right")

    def update_info_text(wn, zeta, wd):
        info_text.set_text(
            f"Natural Freq (wn): {wn:.2f} rad/s\n"
            f"Damping Ratio (ζ): {zeta:.2f}\n"
            f"Damped Freq (wd): {wd:.2f} rad/s"
        )

    update_info_text(wn, zeta, wd)

    # Sliders and TextBox
    ax_kp = plt.axes([0.15, 0.22, 0.65, 0.03])
    ax_kd = plt.axes([0.15, 0.17, 0.65, 0.03])
    ax_x0 = plt.axes([0.15, 0.12, 0.65, 0.03])
    s_kp = Slider(ax_kp, 'kp', 100.0, 5000.0, valinit=kp0, valstep=1.0)
    s_kd = Slider(ax_kd, 'kd', 5.0, 1000.0, valinit=kd0, valstep=1.0)
    s_x0 = Slider(ax_x0, 'Initial Error', 0.01, 5.0, valinit=x0)

    ax_inertia = plt.axes([0.15, 0.05, 0.2, 0.035])
    box_inertia = TextBox(ax_inertia, 'Inertia', initial=str(inertia0))

    def update(val=None):
        """ 
        Update the plots whenever the sliders or text box are changed. 
        """ 
        try:
            inertia = float(box_inertia.text)
        except ValueError:
            inertia = inertia0

        kp = s_kp.val
        kd = s_kd.val
        x0 = s_x0.val

        wn, zeta, wd, t, y, x = compute_response(kp, kd, x0, inertia)
        if t is None:
            return

        u = compute_control_effort(kp, kd, x, inertia)
        settling_time = compute_settling_time(t, y)

        response_line.set_data(t, y)
        effort_line.set_data(t, u)
        st_line.set_xdata(settling_time)

        ax1.set_xlim([-0.05, t[-1]])
        ax1.set_ylim([min(y) - 0.1, max(y) + 0.1])
        ax2.set_xlim([-0.05, t[-1]])
        ax2.set_ylim([min(u) * 1.1, max(u) * 1.1])

        update_info_text(wn, zeta, wd)
        fig.canvas.draw_idle()

    s_kp.on_changed(update)
    s_kd.on_changed(update)
    s_x0.on_changed(update)
    box_inertia.on_submit(lambda val: update())

    plt.show()

if __name__ == "__main__":
    interactive_pd_homogeneous_fixed()