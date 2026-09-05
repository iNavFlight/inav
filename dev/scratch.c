...
#include "arm_math.h"
#define SAMPLES 128
const int samplingFreq = 1000;
...

static void pdAdcTask(void* parameters) {
 
  DEBUG_PRINT("FFT test function is running!");
  
  //Configure delay
  float delay = 500 / portTICK_PERIOD_MS;
  DEBUG_PRINT("Delay in ticks is %f \n", delay);
  const TickType_t xDelay =  delay;

  //Some parameters
  int peakFrequency;
  int f = 150;
  int A = 58;

  while (true) {
    	//Force Delay
   	 vTaskDelay(xDelay);

    	//Create input signal
   	for(int i=0; i<SAMPLES; i++){
   		mySine[i] = A*arm_sin_f32(2*PI*f*i/samplingFreq);
   	}

   	//Create RFFT instance
   	arm_rfft_fast_instance_f32 S;
    	arm_rfft_fast_init_f32(&S, SAMPLES);

    	//RFFT transform
    	arm_rfft_fast_f32(&S, mySine,rfft_output,0);
    	//Calculate magnitude of imaginary coefficients
    	arm_cmplx_mag_f32(rfft_output,test_output, SAMPLES/2);
    	//Set DC component to 0
    	test_output[0] = 0;
    	//Obtain peak frequency
    	arm_max_f32(test_output, SAMPLES/2, &maxvalue, &maxindex);

    	peakFrequency = maxindex * samplingFreq / SAMPLES;

    	DEBUG_PRINT("Peak frequency %d \n\r", peakFrequency);

    	DEBUG_PRINT("Max Value:[%ld]:%f Output=[",maxindex,2*maxvalue/SAMPLES);
    	DEBUG_PRINT("]\r\n");
  }
}