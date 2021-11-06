# Profiles

A profile is a set of configuration settings.

Currently three profiles are supported. The default profile is profile `1`.

## Changing profiles
### Stick Commands
Profiles can be selected using a GUI or the following stick combinations:

| Profile | Throttle | Yaw   | Pitch  | Roll   |
| ------- | -------- | ----- | ------ | ------ |
| 1       | Down     | Left  | Middle | Left   |
| 2       | Down     | Left  | Up     | Middle |
| 3       | Down     | Left  | Middle | Right  |

### CLI
The CLI `profile` command can also be used to change profiles:

```
profile <index>
```

### Programming (4.0.0 onwards)
You can change profiles using the programming frame work. This allows a lot of flexability in how you change profiles.

For example, using a simple switch on channel 15.

[![For example, using a simple switch](https://i.imgur.com/SS9CaaOl.png)](https://i.imgur.com/SS9CaaO.png)

Or using the speed to change profiles. In this example: 
- when lower than 25 cm/s (basically not flying), profiles are not effected.
- Below 2682 cm/s (60 mph | 97 Km/h) use Profile 1
- Above 5364 cm/s (120 mph | 193 Km/h) use Profile 3
- Between 2683 and 5364 cm/s, use Profile 2

[![Using speed to change profiles](https://i.imgur.com/WjkuhhWl.png)](https://i.imgur.com/WjkuhhW.png)

## Profile Contents
The values contained within a profile can be seen by using the CLI `dump profile` command.

e.g
```
# dump profile

# profile
profile 1

set mc_p_pitch = 40
set mc_i_pitch = 30
set mc_d_pitch = 23
set mc_cd_pitch = 60
set mc_p_roll = 40
set mc_i_roll = 30
set mc_d_roll = 23
set mc_cd_roll = 60
set mc_p_yaw = 85
set mc_i_yaw = 45
set mc_d_yaw = 0
set mc_cd_yaw = 60
set mc_p_level = 20
set mc_i_level = 15
set mc_d_level = 75
set fw_p_pitch = 5
set fw_i_pitch = 7
set fw_d_pitch = 0
set fw_ff_pitch = 50
set fw_p_roll = 5
set fw_i_roll = 7
set fw_d_roll = 0
set fw_ff_roll = 50
set fw_p_yaw = 6
set fw_i_yaw = 10
set fw_d_yaw = 0
set fw_ff_yaw = 60
set fw_p_level = 20
set fw_i_level = 5
set fw_d_level = 75
set max_angle_inclination_rll = 300
set max_angle_inclination_pit = 300
set dterm_lpf_hz = 110
set dterm_lpf_type = PT2
set dterm_lpf2_hz = 0
set dterm_lpf2_type = PT1
set yaw_lpf_hz = 0
set fw_iterm_throw_limit = 165
set fw_loiter_direction = RIGHT
set fw_reference_airspeed =  1500.000
set fw_turn_assist_yaw_gain =  1.000
set fw_turn_assist_pitch_gain =  1.000
set fw_iterm_limit_stick_position =  0.500
set fw_yaw_iterm_freeze_bank_angle = 0
set pidsum_limit = 500
set pidsum_limit_yaw = 350
set iterm_windup = 50
set rate_accel_limit_roll_pitch = 0
set rate_accel_limit_yaw = 10000
set heading_hold_rate_limit = 90
set nav_mc_pos_z_p = 50
set nav_mc_vel_z_p = 100
set nav_mc_vel_z_i = 50
set nav_mc_vel_z_d = 10
set nav_mc_pos_xy_p = 65
set nav_mc_vel_xy_p = 40
set nav_mc_vel_xy_i = 15
set nav_mc_vel_xy_d = 100
set nav_mc_vel_xy_ff = 40
set nav_mc_heading_p = 60
set nav_mc_vel_xy_dterm_lpf_hz =  2.000
set nav_mc_vel_xy_dterm_attenuation = 90
set nav_mc_vel_xy_dterm_attenuation_start = 10
set nav_mc_vel_xy_dterm_attenuation_end = 60
set nav_fw_pos_z_p = 40
set nav_fw_pos_z_i = 5
set nav_fw_pos_z_d = 10
set nav_fw_pos_xy_p = 75
set nav_fw_pos_xy_i = 5
set nav_fw_pos_xy_d = 8
set nav_fw_heading_p = 60
set nav_fw_pos_hdg_p = 30
set nav_fw_pos_hdg_i = 2
set nav_fw_pos_hdg_d = 0
set nav_fw_pos_hdg_pidsum_limit = 350
set mc_iterm_relax = RP
set mc_iterm_relax_cutoff = 15
set d_boost_min =  0.500
set d_boost_max =  1.250
set d_boost_max_at_acceleration =  7500.000
set d_boost_gyro_delta_lpf_hz = 80
set antigravity_gain =  1.000
set antigravity_accelerator =  1.000
set antigravity_cutoff_lpf_hz = 15
set pid_type = AUTO
set mc_cd_lpf_hz = 30
set fw_level_pitch_trim =  0.000
set smith_predictor_strength =  0.500
set smith_predictor_delay =  0.000
set smith_predictor_lpf_hz = 50
set fw_level_pitch_gain =  5.000
set thr_mid = 50
set thr_expo = 0
set tpa_rate = 0
set tpa_breakpoint = 1500
set fw_tpa_time_constant = 0
set rc_expo = 70
set rc_yaw_expo = 20
set roll_rate = 20
set pitch_rate = 20
set yaw_rate = 20
set manual_rc_expo = 70
set manual_rc_yaw_expo = 20
set manual_roll_rate = 100
set manual_pitch_rate = 100
set manual_yaw_rate = 100
set fpv_mix_degrees = 0
set rate_dynamics_center_sensitivity = 100
set rate_dynamics_end_sensitivity = 100
set rate_dynamics_center_correction = 10
set rate_dynamics_end_correction = 10
set rate_dynamics_center_weight = 0
set rate_dynamics_end_weight = 0

```
