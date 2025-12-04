0#include "platform.h"
#include "drivers/io.h"
#include "drivers/time.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "rx/external_pwm.h"

static volatile uint16_t extPwmUs = 0;

void externalPwmInit(void)
{
    IO_t io = IOGetByTag(IO_TAG(PB10));
    IOInit(io, OWNER_RX, RESOURCE_INPUT, 0);
    IOConfigGPIO(io, IOCFG_IPD); // Input Pull-Down
}

void externalPwmUpdate(void) 
{
    static timeUs_t lastReadTime = 0;
    static bool lastState = false;
    static timeUs_t riseTime = 0;
    static uint16_t filteredUs = 0;
    static timeUs_t lastEdgeTime = 0;
    
    timeUs_t now = micros();
    
    if (now - lastReadTime < 10) {
        return;
    }
    lastReadTime = now;
    
    IO_t io = IOGetByTag(IO_TAG(PB10));
    bool currentState = IORead(io);
    
    // Обнаружение фронтов
    if (currentState && !lastState) {
        // Rising edge
        riseTime = now;
        lastEdgeTime = now;
    } 
    else if (!currentState && lastState && riseTime != 0) {
        // Falling edge
        timeUs_t pulseWidth = now - riseTime;
        
        // Фильтрация
        if (pulseWidth >= 600 && pulseWidth <= 2500) {
            filteredUs = (filteredUs * 3 + (uint16_t)pulseWidth) / 4;
            extPwmUs = filteredUs;
        }
        riseTime = 0;
        lastEdgeTime = now;
    }
    
    // СБРОС ПРИ ПРОПАЖЕ СИГНАЛА - если больше 50ms нет фронтов
    if (now - lastEdgeTime > 50000) { // 50ms
        extPwmUs = 0;
        filteredUs = 0;
    }
    
    lastState = currentState;
}

uint16_t getExternalPwmUs(void)
{
    return extPwmUs;
}
