#ifndef PID_h
#define PID_h

#include "chtypes.h"

//Constants used in some of the functions below
#define PID_AUTOMATIC 1
#define PID_MANUAL 0
#define PID_DIRECT 0
#define PID_REVERSE 1
#define PID_ON_M 0
#define PID_ON_E 1


typedef struct {

    float kp;     // * (P)roportional Tuning Parameter
    float ki;     // * (I)ntegral Tuning Parameter
    float kd;     // * (D)erivative Tuning Parameter

    float dispKp; // * we'll hold on to the tuning parameters in user-entered 
    float dispKi; //   format for display purposes
    float dispKd; //

    int direction;
    int pOn;

    float *input;              // * Pointers to the Input, Output, and Setpoint variables
    float *output;             //   This creates a hard link between the variables and the 
    float *setPoint;           //   PID, freeing the user from having to constantly tell us
                               //   what these values are.  with pointers we'll just know.
    unsigned long lastTime;
    float outputSum;
    float lastInput;

    unsigned long sampleTime;
    float outMin;
    float outMax;

    bool inAuto;
    bool pOnE;

} pidc_t;


//commonly used functions **************************************************************************
void pid_create(pidc_t* p, float* Input, float* Output, float* Setpoint,  // * constructor.  links the PID to the Input, Output, and 
                float Kp, float Ki, float Kd, int POn, int Direction);    //   Setpoint.  Initial tuning parameters are also set here.
                                                                          //   (overload for specifying proportional mode)

void pid_setMode(pidc_t* p, int mode);                 // * sets PID to either Manual (0) or Auto (non-0)

bool pid_compute(pidc_t* p);               // * performs the PID calculation.  it should be
                                           //   called every time loop() cycles. ON/OFF and
                                           //   calculation frequency can be set using SetMode
                                           //   SetsampleTime respectively

void pid_setOutputLimits(pidc_t* p, float Min, float Max); // * clamps the output to a specific range. 0-255 by default, but
                                                           //   it's likely the user will want to change this depending on
                                                           //   the application



//available but not commonly used functions ********************************************************
void pid_setTunings(pidc_t* p, float Kp, float Ki, float Kd, int POn);  // * While most users will set the tunings once in the 
                                                                        //   constructor, this function gives the user the option
                                                                        //   of changing tunings during runtime for Adaptive control

void pid_setDirection(pidc_t* p, int Direction);  // * Sets the Direction, or "Action" of the controller. DIRECT
                                                  //   means the output will increase when error is positive. REVERSE
                                                  //   means the opposite.  it's very unlikely that this will be needed
                                                  //   once it is set in the constructor.
void pid_setSampleTime(pidc_t* p, int NewSampleTime);  // * sets the frequency, in Milliseconds, with which 
                                                       //   the PID calculation is performed.  default is 100

void pid_initialize(pidc_t* p);

#endif
