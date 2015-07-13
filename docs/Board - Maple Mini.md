# Board - Maple Mini

The Maple Mini is less than 6$ board that used with any gy-85 or gy-86 board will make fully featured flying machine. According to the hardware used you can have more or less input/output. 
If using PWM input gps and GY-88 the limitation is 6 channel input (Roll/pitch/throttle/yaw/aux1/aux2) and only 4 motors can be driven (so that limits the use for a quadcopter only)
if using PPM you can drive fully any flight machine using the below configuration
## Connections

### RC Input

INPUT

PA0  D3  - PWM1 / PPM
PA7  D4  - PWM2  - RC2/Motor5 (Motor6 if USE_VCP)
PA2  D5  - PWM3  - RC3/Motor6 (Motor7 if USE_VCP)
PA3  D10 - PWM4  - RC4/Motor7 (Motor8 if USE_VCP)
PA6  D11 - PWM5  - RC5/Motor8 (Motor9 if USE_VCP)
PA7  BUT - PWM6  - RC6/Motor9 (Motor10 if USE_VCP)
PB0  D15 - PWM7  - Motor1
PB1  D16 - PWM8  - Motor2
PA8  D24 - PWM9  - Motor3/(#NA if  USE_VCP)
PA11 D25 - PWM10 - #NA/(Motor3 if USE_VCP)
PB6  D26 - PWM11 - #NA/(Motor4 if USE_VCP)
PB7  D27 - PWM12 - Motor4/(Motor5 if USE_VCP)

UARTS1 (UARTS2 on the software)
Vin - +5v FDTI
D25 - UART1 RX / FDTI RX
D26 - UART1 TX / FDTI TX
GND

I2C Sensor Connection:

D0 - SDA
D1 - SCL


UARTS2 (UARTS3 on the software)
D5 - TX
D6 - RX




