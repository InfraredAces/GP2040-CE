#ifndef ANALOG_BUTTON_H
#define ANALOG_BUTTON_H

#include "gpaddon.h"
#include "GamepadEnums.h"
#include "BoardConfig.h"
#include "enums.pb.h"

#ifndef ANALOG_BUTTON_ENABLED
#define ANALOG_BUTTON_ENABLED 1
#endif

// All distances are measured in 0.1mm increments

#ifndef ANALOG_BUTTON_TOTAL_TRAVEL
#define ANALOG_BUTTON_TOTAL_TRAVEL 400
#endif

#ifndef ANALOG_BUTTON_ACTUATION_POINT
#define ANALOG_BUTTON_ACTUATION_POINT 150
#endif

// Distance that the switch need to move downward, when past the actuation point, to register as pressed
#ifndef ANALOG_BUTTON_PRESS_THRESHOLD
#define ANALOG_BUTTON_PRESS_THRESHOLD 20
#endif

// Distance that the switch need to move upward, when past the actuation point, to register as released
#ifndef ANALOG_BUTTON_RELEASE_THRESHOLD
#define ANALOG_BUTTON_RELEASE_THRESHOLD 55
#endif

#ifndef ANALOG_BUTTON_00_PIN
#define ANALOG_BUTTON_00_PIN 26
#endif

#ifndef ANALOG_BUTTON_00_ACTION
#define ANALOG_BUTTON_00_ACTION GpioAction::ANALOG_LS_DIRECTION_LEFT
#endif

#ifndef ANALOG_BUTTON_01_PIN
#define ANALOG_BUTTON_01_PIN 27
#endif

#ifndef ANALOG_BUTTON_01_ACTION
#define ANALOG_BUTTON_01_ACTION GpioAction::ANALOG_LS_DIRECTION_DOWN
#endif

#ifndef ANALOG_BUTTON_02_PIN
#define ANALOG_BUTTON_02_PIN 28
#endif

#ifndef ANALOG_BUTTON_02_ACTION
#define ANALOG_BUTTON_02_ACTION GpioAction::NONE
#endif

#ifndef ANALOG_BUTTON_03_PIN
#define ANALOG_BUTTON_03_PIN -1
#endif

#ifndef ANALOG_BUTTON_03_ACTION
#define ANALOG_BUTTON_03_ACTION GpioAction::NONE
#endif

/*
---------------------------
  Analog Button State
---------------------------
*/

struct AnalogButton {
    uint16_t index;
    int pin = -1;
    GpioMappingInfo gpioMappingInfo;
    uint16_t rawValue = 0;
    uint16_t smaValue = 0;
    uint16_t maxValue = 0;
    uint16_t restValue = 0;
    bool calibrated = false;
    bool pressed = false;
};

/*
---------------------------
  Analog Button Addon
---------------------------
*/

#define NUM_ANALOG_BUTTONS 4

#define AnalogButtonName "AnalogButton"

class AnalogButtonAddon : public GPAddon {
    public:
        virtual bool available();
        virtual void setup();
        virtual void preprocess() {}
        virtual void process();
        virtual std::string name() { return AnalogButtonName; }
    private:
        static void printGpioAction(GpioMappingInfo gpioMappingInfo);
        uint16_t readADCPin(int pin);
        void queueAnalogChange(GpioAction gpioAction, uint16_t analogValue, uint16_t lastAnalogValue);
        void updateAnalogState();
        uint16_t getAverage();
        AnalogButton analogButtons[NUM_ANALOG_BUTTONS];
        int buttonPins[NUM_ANALOG_BUTTONS] = {
            ANALOG_BUTTON_00_PIN, 
            ANALOG_BUTTON_01_PIN, 
            ANALOG_BUTTON_02_PIN, 
            ANALOG_BUTTON_03_PIN
        };
        GpioAction buttonActions[NUM_ANALOG_BUTTONS] = {
            ANALOG_BUTTON_00_ACTION,
            ANALOG_BUTTON_01_ACTION,
            ANALOG_BUTTON_02_ACTION,
            ANALOG_BUTTON_03_ACTION
        };  
};

#endif