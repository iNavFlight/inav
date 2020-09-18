# INAV PID Controller

What you have to know about INAV PID/PIFF/PIDCD controllers:

1. INAV PID uses floating-point math
1. Rate/Angular Velocity controllers work in dps [degrees per second]
1. P, I, D and Multirotor CD gains are scaled like Betafligfht equivalents, but actual mechanics are different, and PID response might be different
1. Depending on platform type, different controllers are used
    1. Fixed-wing uses **PIFF**:
        1. Error is computed with a formula `const float rateError = pidState->rateTarget - pidState->gyroRate;`
        1. P-term with a formula `rateError * pidState->kP`
        1. Simple I-term without Iterm Relax. I-term limit based on stick position is used instead. I-term is no allowed to grow if stick (roll/pitch/yaw) is deflected above threshold defined in `fw_iterm_limit_stick_position`. `pidState->errorGyroIf += rateError * pidState->kI * dT;`
        1. No D-term
        1. FF-term (Feed Forward) is computed from the controller input with a formula `pidState->rateTarget * pidState->kFF`. Bear in mind, this is not a **FeedForward** from Betaflight!
    1. Multirotor uses **PIDCD**:
        1.  Error is computed with a formula `const float rateError = pidState->rateTarget - pidState->gyroRate;`
        1. P-term with a formula `rateError * pidState->kP`
        1. I-term
            1. Iterm Relax is used to dynamically attenuate I-term during fast stick movements
            1. I-term formula `pidState->errorGyroIf += (itermErrorRate * pidState->kI * antiWindupScaler * dT) + ((newOutputLimited - newOutput) * pidState->kT * antiWindupScaler * dT);`
            1. I-term can be limited when motor output is saturated
        1. D-term is computed only from gyro measurement
        1. There are 2 LPF filters on D-term 
        1. D-term can by boosted during fast maneuvers using D-Boost. D-Boost is an equivalent of Betaflight D_min
        1. **Control Derivative**, CD, or CD-term is a derivative computed from the setpoint that helps to boost PIDCD controller during fast stick movements. `newCDTerm = rateTargetDeltaFiltered * (pidState->kCD / dT);` It is an equivalent of Betaflight Feed Forward 