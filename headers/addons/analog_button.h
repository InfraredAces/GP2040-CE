#ifndef ANALOG_BUTTON_H
#define ANALOG_BUTTON_H

#include "gpaddon.h"
#include "GamepadEnums.h"
#include "BoardConfig.h"
#include "enums.pb.h"
#include <map>
#include "lib/AnalogButton/sma_filter.h"

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

#ifndef ANALOG_BUTTON_POLE_ORIENTATION
#define ANALOG_BUTTON_POLE_ORIENTATION 1
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
#define ANALOG_BUTTON_02_ACTION GpioAction::ANALOG_LS_DIRECTION_RIGHT
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

#define ANALOG_RESOLUTION 12

struct AnalogButton {
    uint16_t index;
    int pin = -1;
    GpioMappingInfo gpioMappingInfo;
    uint16_t rawValue = 0;
    uint16_t smaValue = 0;
    uint16_t restPosition = 0;
    uint16_t downPosition = (1 << ANALOG_RESOLUTION) - 1;
    uint16_t distance = 0;
    bool calibrated = false;
    bool pressed = false;
    SMAFilter filter = SMAFilter(SMA_FILTER_SAMPLE_EXPONENT);
};

struct AnalogChange {
    GpioAction gpioAction;
    uint16_t newValue;
    uint16_t lastValue;
};

/*
---------------------------
  Analog Button Addon
---------------------------
*/

#define NUM_ANALOG_BUTTONS 4
#define ANALOG_BUTTON_DEADZONE 20
#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

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
        long map(long x, long in_min, long in_max, long out_min, long out_max);
        void readButton(AnalogButton &button);
        void queueAnalogChange(GpioAction gpioAction, uint16_t newAnalogValue);
        void updateAnalogState();
        uint16_t getAverage();
        void updateButtonRange(AnalogButton &button);
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
        AnalogChange analogChanges[10] = {
            {GpioAction::ANALOG_LS_DIRECTION_UP, GAMEPAD_JOYSTICK_MID, GAMEPAD_JOYSTICK_MID},
            {GpioAction::ANALOG_LS_DIRECTION_DOWN, GAMEPAD_JOYSTICK_MID, GAMEPAD_JOYSTICK_MID},
            {GpioAction::ANALOG_LS_DIRECTION_LEFT, GAMEPAD_JOYSTICK_MID, GAMEPAD_JOYSTICK_MID},
            {GpioAction::ANALOG_LS_DIRECTION_RIGHT, GAMEPAD_JOYSTICK_MID, GAMEPAD_JOYSTICK_MID},
            {GpioAction::ANALOG_RS_DIRECTION_UP, GAMEPAD_JOYSTICK_MID, GAMEPAD_JOYSTICK_MID},
            {GpioAction::ANALOG_RS_DIRECTION_DOWN, GAMEPAD_JOYSTICK_MID, GAMEPAD_JOYSTICK_MID},
            {GpioAction::ANALOG_RS_DIRECTION_LEFT, GAMEPAD_JOYSTICK_MID, GAMEPAD_JOYSTICK_MID},
            {GpioAction::ANALOG_RS_DIRECTION_RIGHT, GAMEPAD_JOYSTICK_MID, GAMEPAD_JOYSTICK_MID},
            {GpioAction::ANALOG_TRIGGER_L2, GAMEPAD_TRIGGER_MID, GAMEPAD_TRIGGER_MID},
            {GpioAction::ANALOG_TRIGGER_R2, GAMEPAD_TRIGGER_MID, GAMEPAD_TRIGGER_MID},
        };
};

#endif