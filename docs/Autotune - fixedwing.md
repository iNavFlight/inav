# Airplane Autotune instructions

Airplane PIFF autotune is inspired by ArduPilot Plane firmware.

Getting a good set of roll/pitch/yaw PIFF parameters for your aircraft is essential for stable flight. To help with this it is highly recommended that you use the AUTOTUNE system described below.

## What AUTOTUNE does

AUTOTUNE is a modifier you enable on top of ANGLE, HORIZON or ACRO mode. It has no effect at all while flying in MANUAL mode - you can switch it on, but nothing will be tuned until you leave MANUAL. Where it is active, it uses the aircraft's response to your stick inputs to tune, for roll, pitch and yaw independently:

* **FeedForward (FF) gain** - in ANGLE, HORIZON or ACRO, whenever you give it enough hard stick input to learn from
* **Rate** (`roll_rate`/`pitch_rate`/`yaw_rate`) - only while flying in ACRO or HORIZON, and only if `fw_autotune_rate_adjustment` is not set to `FIXED` (see below). Rate is never adjusted while flying in ANGLE mode.

**AUTOTUNE never adjusts P or I gains, on any axis.** Tune P and I manually, either before or after using AUTOTUNE. This is a change from older INAV versions - if you're used to AUTOTUNE also setting P/I, that's no longer how it works.

In general the pilot needs to activate AUTOTUNE while in the air and then fly the plane for a few minutes. While flying the pilot needs to input as many sharp attitude changes as possible, on all three axes, so that the autotune code can learn how the aircraft responds. An axis that isn't given hard stick input won't be tuned - it's easy to spend a whole flight on hard rolls and pitches and forget yaw entirely.

## Before flying with AUTOTUNE

Before taking off you need to set up a few parameters for your airplane:

parameter | explanation
--------- | -----------
roll_rate | Starting roll rate for your airplane. Unless you set `fw_autotune_rate_adjustment` to `FIXED` (see below), AUTOTUNE will change this value while you fly - it's a starting point, not a ceiling it respects
pitch_rate | Starting pitch rate for your airplane. Same caveat as `roll_rate`
yaw_rate | Starting yaw rate for your airplane. Same caveat as `roll_rate`
fw_autotune_rate_adjustment | Controls whether/how AUTOTUNE changes `roll_rate`/`pitch_rate`/`yaw_rate` while tuning. `AUTO` (default) raises or lowers rates to match what the plane can achieve. `LIMIT` only ever lowers them, never above what you started with. `FIXED` leaves rates untouched - use this if you want AUTOTUNE to tune FeedForward only
fw_autotune_min_stick | Minimum stick deflection [%] (default 50) before AUTOTUNE starts recording a maneuver as tuning data
fw_autotune_max_rate_deflection | Target servo/mixer deflection [%] (default 90) AUTOTUNE aims for when calculating an achievable rate in `AUTO`/`LIMIT`
fw_p_level | Self-leveling strength. Bigger value means sharper response
fw_i_level | Self-leveling filtering. Usual value for airplanes is 1-5 Hz
max_angle_inclination_rll | Maximum roll angle in [0.1 deg] units
max_angle_inclination_pit | Maximum pitch angle in [0.1 deg] units
tpa_breakpoint | Cruise throttle (expected throttle that you would be flying most of the time)
tpa_rate | Amount of TPS curve to apply (usually should be in range 50-80 for most airplanes)

For most hobby-sized airplanes, starting roll/pitch rates in the range 70-120 deg/s (7-12 for `roll_rate` and `pitch_rate` values) work well. Small and agile flying wings can reach 180-200 deg/s. With the default `AUTO` setting, don't worry about getting this exactly right - AUTOTUNE will adjust it based on what your airplane actually does, down to a floor of 40 deg/s on roll/pitch and 10 deg/s on yaw, up to a ceiling of 720 deg/s.

Other things to check:

* It's highly recommended that you fly in MANUAL and trim your servo midpoints for stable flight
* Make sure you have center of gravity according to manual to your aircraft
* Check that your failsafe activates correctly (test on the ground with propeller off for safety)

## Flying in AUTOTUNE

Once you are all setup you can take off normally and switch to AUTOTUNE mode once you have gained altitude.

When you engage AUTOTUNE a few things will happen:

* The autotune system will monitor requested roll, pitch and yaw rates (as determined by your transmitter stick movements, once deflection passes `fw_autotune_min_stick`). When the demanded rate on an axis exceeds that threshold the autotune system will use the response of the aircraft to learn FeedForward (and, depending on `fw_autotune_rate_adjustment` and mode, Rate) for that axis
* Every 5 seconds the autotune system will store a snapshot of the current values. When you switch out of AUTOTUNE the last snapshot is restored - so if you exit within 5 seconds of enabling it, nothing changes
* You may find the plane is quite sluggish when you first enter AUTOTUNE. You will find that as the tune progresses this will get better. Make sure your flight area has plenty of room for slow large-radius turns.
* Don't land in AUTOTUNE mode - during landing airplane doesn't reach it full performance which may be read by autotune system as insufficient gains.

The key to a successful autotune is to input rapid movements with the transmitter sticks. You should only exercise one of roll, pitch or yaw at a time, and you should move the stick rapidly to the maximum deflection. Don't neglect yaw - it's easy to spend the whole flight on rolls and pitches and leave yaw untuned.

Fly in ACRO if you can - AUTOTUNE only learns Rate in modes where stick position maps directly to a commanded rotation rate (ACRO, and mostly HORIZON). In ANGLE mode the stick commands an angle instead, so AUTOTUNE will still tune FeedForward but will not touch Rate at all while you're in ANGLE. AUTOTUNE has no effect whatsoever in MANUAL mode - don't fly it there.

## Don't stop too early

The more you fly the better it will get. Let autotune analyze how your airplane behaves and figure decent tune for you. Once you feel that airplane is flying good in AUTOTUNE - keep flying well past that point to finalize the tune.

## Completing the tune

Once you have tuned reasonable PIFF parameters with AUTOTUNE you should complete the tune by switching out of AUTOTUNE to ANGLE or MANUAL and landing the airplane.

Note that AUTOTUNE mode doesn't automatically save parameters to EEPROM. You need to disarm and issue a [stick command](Controls.md) to save configuration parameters.
