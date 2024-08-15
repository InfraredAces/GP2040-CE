#ifndef ANALOG_BUTTON_H
#define ANALOG_BUTTON_H

#include "gpaddon.h"
#include "GamepadEnums.h"
#include "BoardConfig.h"
#include "enums.pb.h"
#include <map>
#include <queue>
#include "lib/AnalogButton/sma_filter.h"

#ifndef ANALOG_BUTTON_ENABLED
#define ANALOG_BUTTON_ENABLED 1
#endif

#ifndef ANALOG_BUTTON_TOTAL_TRAVEL
// uint16 measured in 0.1mm increments 
#define ANALOG_BUTTON_TOTAL_TRAVEL 400
#endif

#ifndef ANALOG_BUTTON_POLE_SENSOR_ORIENTATION
// -1 or 1, Orientation of magnet and sensor relative to one another, Flip if unexpected behavior occurs
#define ANALOG_BUTTON_POLE_SENSOR_ORIENTATION 1
#endif

#ifndef ANALOG_BUTTON_TRIGGER_MODE
#define ANALOG_BUTTON_TRIGGER_MODE AnalogTriggerType::RAPID_TRIGGER
#endif

#ifndef CONTINUOUS_RAPID_THRESHOLD
/// uint16 measured in 0.1mm increments, Distance from rest position for disabling rapid trigger state
#define CONTINUOUS_RAPID_THRESHOLD 0
#endif

#ifndef ANALOG_BUTTON_ACTUATION_POINT
// uint16 measured in 0.1mm increments 
#define ANALOG_BUTTON_ACTUATION_POINT 150
#endif

#ifndef ANALOG_BUTTON_PRESS_THRESHOLD
/// uint16 measured in 0.1mm increments, Downward distance required when ready to actuate switch
#define ANALOG_BUTTON_PRESS_THRESHOLD 20
#endif

#ifndef ANALOG_BUTTON_RELEASE_THRESHOLD
/// uint16 measured in 0.1mm increments, Upward distance required while pressed to deactivate switch
#define ANALOG_BUTTON_RELEASE_THRESHOLD 20
#endif

#ifndef ANALOG_BUTTON_ENFORCE_CIRCULARITY
#define ANALOG_BUTTON_ENFORCE_CIRCULARITY 1
#endif

#ifndef ANALOG_BUTTON_00_PIN
#define ANALOG_BUTTON_00_PIN 27
#endif

#ifndef ANALOG_BUTTON_00_ACTION
#define ANALOG_BUTTON_00_ACTION GpioAction::ANALOG_LS_DIRECTION_DOWN
#endif

#ifndef ANALOG_BUTTON_01_PIN
#define ANALOG_BUTTON_01_PIN 28
#endif

#ifndef ANALOG_BUTTON_01_ACTION
#define ANALOG_BUTTON_01_ACTION GpioAction::ANALOG_LS_DIRECTION_RIGHT
#endif

#ifndef ANALOG_BUTTON_02_PIN
#define ANALOG_BUTTON_02_PIN 26
#endif

#ifndef ANALOG_BUTTON_02_ACTION
#define ANALOG_BUTTON_02_ACTION GpioAction::ANALOG_LS_DIRECTION_LEFT
#endif

#ifndef ANALOG_BUTTON_03_PIN
#define ANALOG_BUTTON_03_PIN -1
#endif

#ifndef ANALOG_BUTTON_03_ACTION
#define ANALOG_BUTTON_03_ACTION GpioAction::ANALOG_LS_DIRECTION_UP
#endif

#ifndef ANALOG_BUTTON_0_PIN
#define ANALOG_BUTTON_0_PIN -1
#endif

#ifndef ANALOG_BUTTON_0_ACTION
#define ANALOG_BUTTON_0_ACTION GpioAction::NONE
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
    uint16_t restPosition = (1 << ANALOG_RESOLUTION) - 1;
    uint16_t downPosition = 0;
    uint16_t distance = 0;
    uint16_t localMax = ANALOG_BUTTON_TOTAL_TRAVEL;
    bool calibrated = false;
    bool pressed = false;
    bool inRapidTriggerZone = false;
    SMAFilter filter = SMAFilter(SMA_FILTER_SAMPLE_EXPONENT);
};

struct AnalogChange {
    GpioAction gpioAction;
    uint16_t restPosition;
    uint16_t downPosition;
    uint16_t newValue;
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
        static void printGpioAction(GpioAction gpioAction);
        long map(long x, long in_min, long in_max, long out_min, long out_max);
        void readButton(AnalogButton &button);
        void queueAnalogChange(AnalogButton button);
        void updateAnalogState();
        void scaleVector(float &x, float &y, float magnitudeMin, float magnitudeMax, float scaleMin, float scaleMax);
        uint16_t getAverage();
        void updateButtonRange(AnalogButton &button);
        void processDigitalButton(AnalogButton &button);
        void triggerButtonPress(AnalogButton button);
        void rapidTrigger(AnalogButton &button);
        void SOCDClean(SOCDMode socdMode);
        const SOCDMode getSOCDMode(const GamepadOptions &options);
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
        vector<int> analogActions {
            GpioAction::ANALOG_LS_DIRECTION_UP,
            GpioAction::ANALOG_LS_DIRECTION_DOWN,
            GpioAction::ANALOG_LS_DIRECTION_LEFT,
            GpioAction::ANALOG_LS_DIRECTION_RIGHT,
            GpioAction::ANALOG_RS_DIRECTION_UP,
            GpioAction::ANALOG_RS_DIRECTION_DOWN,
            GpioAction::ANALOG_RS_DIRECTION_LEFT,
            GpioAction::ANALOG_RS_DIRECTION_RIGHT,
            GpioAction::ANALOG_TRIGGER_L2,
            GpioAction::ANALOG_TRIGGER_R2
        };
        queue<AnalogChange> analogChangeQueue;
};

#endif