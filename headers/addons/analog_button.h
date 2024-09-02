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
#define ANALOG_BUTTON_TRIGGER_MODE AnalogTriggerType::STATIC_TRIGGER
#endif

#ifndef CONTINUOUS_RAPID_TRIGGER_THRESHOLD
/// uint16 measured in 0.1mm increments, Distance from rest position for disabling rapid trigger state
#define CONTINUOUS_RAPID_TRIGGER_THRESHOLD 10
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
/// uint16 measured in 1% increments, Upward distance required while pressed to deactivate switch
#define ANALOG_BUTTON_RELEASE_THRESHOLD 20
#endif

#ifndef ANALOG_BUTTON_ENFORCE_CIRCULARITY
// -1 or 1, Will keep analog stick within a unit circle when enabled
#define ANALOG_BUTTON_ENFORCE_CIRCULARITY -1
#endif


#ifndef ANALOG_BUTTON_DEADZONE
/// uint16 measured in 1% increments, The % of total travel that should be applied in order for it to detect movement
#define ANALOG_BUTTON_DEADZONE 20
#endif

#ifndef ANALOG_BUTTON_00_PIN
#define ANALOG_BUTTON_00_PIN 29
#endif

#ifndef ANALOG_BUTTON_00_ACTION
#define ANALOG_BUTTON_00_ACTION GpioAction::ANALOG_LS_DIRECTION_UP
#endif

#ifndef ANALOG_BUTTON_01_PIN
#define ANALOG_BUTTON_01_PIN 27
#endif

#ifndef ANALOG_BUTTON_01_ACTION
#define ANALOG_BUTTON_01_ACTION GpioAction::ANALOG_LS_DIRECTION_DOWN
#endif

#ifndef ANALOG_BUTTON_02_PIN
#define ANALOG_BUTTON_02_PIN 26
#endif

#ifndef ANALOG_BUTTON_02_ACTION
#define ANALOG_BUTTON_02_ACTION GpioAction::ANALOG_LS_DIRECTION_LEFT
#endif

#ifndef ANALOG_BUTTON_03_PIN
#define ANALOG_BUTTON_03_PIN 28
#endif

#ifndef ANALOG_BUTTON_03_ACTION
#define ANALOG_BUTTON_03_ACTION GpioAction::ANALOG_LS_DIRECTION_RIGHT
#endif

#ifndef ANALOG_BUTTON_04_PIN
#define ANALOG_BUTTON_04_PIN -1
#endif

#ifndef ANALOG_BUTTON_04_ACTION
#define ANALOG_BUTTON_04_ACTION GpioAction::ANALOG_RS_DIRECTION_UP
#endif

#ifndef ANALOG_BUTTON_05_PIN
#define ANALOG_BUTTON_05_PIN -1
#endif

#ifndef ANALOG_BUTTON_05_ACTION
#define ANALOG_BUTTON_05_ACTION GpioAction::ANALOG_RS_DIRECTION_DOWN
#endif

#ifndef ANALOG_BUTTON_06_PIN
#define ANALOG_BUTTON_06_PIN -1
#endif

#ifndef ANALOG_BUTTON_06_ACTION
#define ANALOG_BUTTON_06_ACTION GpioAction::ANALOG_RS_DIRECTION_LEFT
#endif

#ifndef ANALOG_BUTTON_07_PIN
#define ANALOG_BUTTON_07_PIN -1
#endif

#ifndef ANALOG_BUTTON_07_ACTION
#define ANALOG_BUTTON_07_ACTION GpioAction::ANALOG_RS_DIRECTION_RIGHT
#endif

#ifndef ANALOG_BUTTON_08_PIN
#define ANALOG_BUTTON_08_PIN -1
#endif

#ifndef ANALOG_BUTTON_08_ACTION
#define ANALOG_BUTTON_08_ACTION GpioAction::BUTTON_PRESS_UP
#endif

#ifndef ANALOG_BUTTON_09_PIN
#define ANALOG_BUTTON_09_PIN -1
#endif

#ifndef ANALOG_BUTTON_09_ACTION
#define ANALOG_BUTTON_09_ACTION GpioAction::BUTTON_PRESS_DOWN
#endif

#ifndef ANALOG_BUTTON_10_PIN
#define ANALOG_BUTTON_10_PIN -1
#endif

#ifndef ANALOG_BUTTON_10_ACTION
#define ANALOG_BUTTON_10_ACTION GpioAction::BUTTON_PRESS_LEFT
#endif

#ifndef ANALOG_BUTTON_11_PIN
#define ANALOG_BUTTON_11_PIN -1
#endif

#ifndef ANALOG_BUTTON_11_ACTION
#define ANALOG_BUTTON_11_ACTION GpioAction::BUTTON_PRESS_RIGHT
#endif

#ifndef ANALOG_BUTTON_12_PIN
#define ANALOG_BUTTON_12_PIN -1
#endif

#ifndef ANALOG_BUTTON_12_ACTION
#define ANALOG_BUTTON_12_ACTION GpioAction::BUTTON_PRESS_B1
#endif

#ifndef ANALOG_BUTTON_13_PIN
#define ANALOG_BUTTON_13_PIN -1
#endif

#ifndef ANALOG_BUTTON_13_ACTION
#define ANALOG_BUTTON_13_ACTION GpioAction::BUTTON_PRESS_B2
#endif

#ifndef ANALOG_BUTTON_14_PIN
#define ANALOG_BUTTON_14_PIN -1
#endif

#ifndef ANALOG_BUTTON_14_ACTION
#define ANALOG_BUTTON_14_ACTION GpioAction::BUTTON_PRESS_B3
#endif

#ifndef ANALOG_BUTTON_15_PIN
#define ANALOG_BUTTON_15_PIN -1
#endif

#ifndef ANALOG_BUTTON_15_ACTION
#define ANALOG_BUTTON_15_ACTION GpioAction::BUTTON_PRESS_B4
#endif

#ifndef ANALOG_BUTTON_16_PIN
#define ANALOG_BUTTON_16_PIN -1
#endif

#ifndef ANALOG_BUTTON_16_ACTION
#define ANALOG_BUTTON_16_ACTION GpioAction::BUTTON_PRESS_L1
#endif

#ifndef ANALOG_BUTTON_17_PIN
#define ANALOG_BUTTON_17_PIN -1
#endif

#ifndef ANALOG_BUTTON_17_ACTION
#define ANALOG_BUTTON_17_ACTION GpioAction::BUTTON_PRESS_R1
#endif

#ifndef ANALOG_BUTTON_18_PIN
#define ANALOG_BUTTON_18_PIN -1
#endif

#ifndef ANALOG_BUTTON_18_ACTION
#define ANALOG_BUTTON_18_ACTION GpioAction::ANALOG_TRIGGER_L2
#endif

#ifndef ANALOG_BUTTON_19_PIN
#define ANALOG_BUTTON_19_PIN -1
#endif

#ifndef ANALOG_BUTTON_19_ACTION
#define ANALOG_BUTTON_19_ACTION GpioAction::ANALOG_TRIGGER_R2
#endif

#ifndef ANALOG_BUTTON_20_PIN
#define ANALOG_BUTTON_20_PIN -1
#endif

#ifndef ANALOG_BUTTON_20_ACTION
#define ANALOG_BUTTON_20_ACTION GpioAction::BUTTON_PRESS_S1
#endif

#ifndef ANALOG_BUTTON_21_PIN
#define ANALOG_BUTTON_21_PIN -1
#endif

#ifndef ANALOG_BUTTON_21_ACTION
#define ANALOG_BUTTON_21_ACTION GpioAction::BUTTON_PRESS_S2
#endif

#ifndef ANALOG_BUTTON_22_PIN
#define ANALOG_BUTTON_22_PIN -1
#endif

#ifndef ANALOG_BUTTON_22_ACTION
#define ANALOG_BUTTON_22_ACTION GpioAction::BUTTON_PRESS_L3
#endif

#ifndef ANALOG_BUTTON_23_PIN
#define ANALOG_BUTTON_23_PIN -1
#endif

#ifndef ANALOG_BUTTON_23_ACTION
#define ANALOG_BUTTON_23_ACTION GpioAction::BUTTON_PRESS_R3
#endif

#ifndef ANALOG_BUTTON_24_PIN
#define ANALOG_BUTTON_24_PIN -1
#endif

#ifndef ANALOG_BUTTON_24_ACTION
#define ANALOG_BUTTON_24_ACTION GpioAction::BUTTON_PRESS_A1
#endif

#ifndef ANALOG_BUTTON_25_PIN
#define ANALOG_BUTTON_25_PIN -1
#endif

#ifndef ANALOG_BUTTON_25_ACTION
#define ANALOG_BUTTON_25_ACTION GpioAction::BUTTON_PRESS_A2
#endif

/*
---------------------------
  Analog Button State
---------------------------
*/

#define ANALOG_RESOLUTION 12

struct AnalogButton {
    uint16_t index = 0;
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

#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

#define AnalogButtonName "AnalogButton"

class AnalogButtonAddon : public GPAddon {
    public:
        virtual bool available();
        virtual void setup();
        virtual void preprocess();
        virtual void process();
        virtual void bootProcess();
        virtual std::string name() { return AnalogButtonName; }
    private:
        long map(long x, long in_min, long in_max, long out_min, long out_max);
        void scaleVector(float &x, float &y, float magnitudeMin, float magnitudeMax, float scaleMin, float scaleMax);
        
        void readButton(AnalogButton &button);
        void updateButtonRange(AnalogButton &button);
        void processDigitalButton(AnalogButton &button);
        void rapidTrigger(AnalogButton &button);
        void triggerButtonPress(AnalogButton button);
        void queueAnalogChange(AnalogButton button);
        void updateAnalogState();
        void updateAnalogDPad();
        AnalogButton analogButtons[NUM_ANALOG_BUTTONS];
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
        uint8_t analogButtonDPadState {0};

        //TODO: Remove once complete
        void printGamepadState(string property);
        void printButtonProp(AnalogButton button, string property = "all");
        void printGpioAction(GpioAction gpioAction);
        void printSOCDMode(SOCDMode socdMode);
        void printS(SOCDMode socdMode);
        void printb(unsigned int v);
};

#endif