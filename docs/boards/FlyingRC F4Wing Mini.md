# Board - FLYINGRCF4WINGMINI

This is a budget-friendly flight controller, typically sold in the US$16-40 range. As with many entry-level boards, component tolerances and quality control may vary compared with higher-cost hardware, so users should test critical functions carefully before flying. The board has been advertised as INAV-compatible, but it was not originally submitted through the official INAV target process; current support exists thanks to community work, especially the contribution from dixi83.

FlyingRC is a small hardware vendor focused on affordable flight-controller and RC electronics products. More information about their work and open-source activity is available on their GitHub page: https://github.com/FlyingRC-Official

Hardware issues have been reported on these flight controllers. They are also missing many features. Unlike most other _wing_ flight controllers. This is not an all in one solution. It requires an external power source for servos. So is not as small or light as it first appears.

> [!WARNING]
> We recommend you only use this flight controller on very light aircraft that you will keep within line-of-sight distances. Reliability of the hardware is far from guaranteed. So fitting to a larger, heavier aircraft adds unnecessary safety risks. Also, there are essential features missing for other types of flights. Please keep this for small park fliers only, if used at all.
>
> Also, if you insist on buying one of these. Make sure it's from a somewhere selling it at $16US. Spending $40US on this is a waste of money. You can get better FCs for around that money.

## Specifications
| | |
|-----|-----|
| MCU | STM32F405RTG6 |
| Gyro | ICM-42605 / BMI270 |
| Baro | SPL06 |
| UARTS | 1, 2 (RX only - SBUS), 4 (DJI), 5 |
| PWM | Six + One (S12 used for LED control) |
| I<sup>2</sup>C | 1 |
| Size | 27.9 x 20.3 x 11.2 mm |
| Weight | 2.8g |

> [!NOTE]
> There is conflicting information for the power this FC can handle. There are 2 specs provided:
> | | |
> |---|---| 
> | Voltmeter | 2.5-30V |
> | Power input | 5v |
> 
> There is no ADC for "voltmeter" input. So potentially this FC can run at 1S to 6S. However there are only 2 LDO regulators on the FC itself. 

## Notable missing features
* Current sensor on earlier hardware revisions (V4 adds a backside ADC current-sensor pad)
* Blackbox recording
* Analogue OSD
* PINIO (for VTX power switching etc)
* ADCs (external current sensor, airspeed sensor, rssi, etc)
* On-board power rail for servos
* Filtered power for video

All the above can be found on the Matek F405-WMO. Which is a $45US flight controller. Which is 32 x 22 x 12.7 mm and 9g, and has a definite input voltage range of 2S to 6S, and able to handle up to 132A.
