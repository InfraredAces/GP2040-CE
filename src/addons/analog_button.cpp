#include "addons/analog_button.h"
#include "config.pb.h"
#include "enums.pb.h"
#include "hardware/adc.h"
#include "helper.h"
#include "storagemanager.h"

#include <math.h>
#include <iostream>
#include "analog_button.h"

#define ADC_PIN_OFFSET 26

bool AnalogButtonAddon::available() {
    return Storage::getInstance().getAddonOptions().analogButtonOptions.enabled;
}
void AnalogButtonAddon::setup() {
    stdio_init_all();

    const size_t num_adc_pins = NUM_ANALOG_BUTTONS;

    for (size_t i = 0; i < num_adc_pins; i++)
    {
        analogButtons[i].pin = buttonPins[i];
        analogButtons[i].gpioMappingInfo.action = buttonActions[i];
        printf("Pin: %2u ", analogButtons[i].pin);
        printGpioAction(analogButtons[i].gpioMappingInfo.action);
    }

    for (size_t i = 0; i < NUM_ANALOG_BUTTONS; i++)
    {
        if (isValidPin(analogButtons[i].pin))
        {
            adc_gpio_init(analogButtons[i].pin);
        }
    }
}

void AnalogButtonAddon::process() {
    const AnalogButtonOptions &analogButtonOptions = Storage::getInstance().getAddonOptions().analogButtonOptions;
    Gamepad *gamepad = Storage::getInstance().GetGamepad();
    gamepad->hasAnalogTriggers = true;

    for (size_t i = 0; i < NUM_ANALOG_BUTTONS; i++)
    {
        if (isValidPin(analogButtons[i].pin))
        {
            readButton(analogButtons[i]);
            if (analogButtons[i].calibrated){
                if(find(analogActions.begin(), analogActions.end(), analogButtons[i].gpioMappingInfo.action) != analogActions.end()) {
                    queueAnalogChange(analogButtons[i]);
                } else {
                 processDigitalButton(analogButtons[i]);
            }

            }
            
        }
    }

    updateAnalogState();

    printf("\n");
}

void AnalogButtonAddon::printGpioAction(GpioAction gpioAction) {
    switch (gpioAction)
    {
    case GpioAction::ANALOG_LS_DIRECTION_UP:
        printf("ANALOG_LS_DIRECTION_UP\n");
        break;
    case GpioAction::ANALOG_LS_DIRECTION_DOWN:
        printf("ANALOG_LS_DIRECTION_DOWN\n");
        break;
    case GpioAction::ANALOG_LS_DIRECTION_LEFT:
        printf("ANALOG_LS_DIRECTION_LEFT\n");
        break;
    case GpioAction::ANALOG_LS_DIRECTION_RIGHT:
        printf("ANALOG_LS_DIRECTION_RIGHT\n");
        break;
    case GpioAction::ANALOG_RS_DIRECTION_UP:
        printf("ANALOG_RS_DIRECTION_UP\n");
        break;
    case GpioAction::ANALOG_RS_DIRECTION_DOWN:
        printf("ANALOG_RS_DIRECTION_DOWN\n");
        break;
    case GpioAction::ANALOG_RS_DIRECTION_LEFT:
        printf("ANALOG_RS_DIRECTION_LEFT\n");
        break;
    case GpioAction::ANALOG_RS_DIRECTION_RIGHT:
        printf("ANALOG_RS_DIRECTION_RIGHT\n");
        break;
    case GpioAction::ANALOG_TRIGGER_L2:
        printf("ANALOG_TRIGGER_L2\n");
        break;
    case GpioAction::ANALOG_TRIGGER_R2:
        printf("ANALOG_TRIGGER_R2\n");
        break;
        break;
    case GpioAction::NONE:
        printf("NONE\n");
        break;
    default:
        printf("OTHER/MISSING\n");
        break;
    }
}

long AnalogButtonAddon::map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void AnalogButtonAddon::readButton(AnalogButton &button) {
    adc_select_input(button.pin - ADC_PIN_OFFSET);
    button.rawValue = adc_read();
    button.smaValue = button.filter(button.rawValue);

    if (ANALOG_BUTTON_POLE_ORIENTATION == -1) {
        button.smaValue = (1 << ANALOG_RESOLUTION) - 1 - button.smaValue;
    }

    if (button.filter.initialized){
        updateButtonRange(button);
    }

    if (!button.calibrated) {
        button.distance = ANALOG_BUTTON_TOTAL_TRAVEL;
        return;
    }

    button.distance = constrain(map(button.smaValue, button.downPosition, button.restPosition, 0, ANALOG_BUTTON_TOTAL_TRAVEL), 0, ANALOG_BUTTON_TOTAL_TRAVEL);

    printf("%1.2fmm ", (float)button.distance / 100.0);
}

void AnalogButtonAddon::queueAnalogChange(AnalogButton button) {
    AnalogChange analogChange = {
        button.gpioMappingInfo.action,
        button.restPosition,
        button.downPosition,
        button.smaValue,
    };
    analogChangeQueue.push(analogChange);
}

void AnalogButtonAddon::updateAnalogState() {
    Gamepad *gamepad = Storage::getInstance().GetGamepad();
    gamepad->hasAnalogTriggers = true;


    float joystickMin = (float)GAMEPAD_JOYSTICK_MIN;
    float joystickMid = (float)GAMEPAD_JOYSTICK_MID;
    float joystickMax = (float)GAMEPAD_JOYSTICK_MAX;
    float triggerMin = (float)GAMEPAD_TRIGGER_MIN;
    float triggerMid = (float)GAMEPAD_TRIGGER_MID;
    float triggerMax = (float)GAMEPAD_TRIGGER_MAX;

    int16_t deltaLY = 0;
    int16_t deltaLX = 0;
    int16_t deltaRY = 0;
    int16_t deltaRX = 0;
    int16_t deltaLT = 0;
    int16_t deltaRT = 0;

    for (size_t i = 0; i < size(analogChangeQueue); i++) {
        GpioAction gpioAction = analogChangeQueue.front().gpioAction;
        int16_t restPosition = analogChangeQueue.front().restPosition;
        int16_t downPosition = analogChangeQueue.front().downPosition;
        int16_t newValue = analogChangeQueue.front().newValue;

        float deltaPercent = 0.0;

        //TODO: Update logic to handle multiple buttons assigned to analog directions
        if ((newValue - downPosition) > ANALOG_BUTTON_DEADZONE) {
            deltaPercent = constrain((float)(newValue - downPosition) / (float)(restPosition - downPosition), 0.0, 1.0);
            switch(gpioAction) {
                case GpioAction::ANALOG_LS_DIRECTION_UP:
                    deltaLY -= (int16_t)(deltaPercent * joystickMid);
                    break;
                case GpioAction::ANALOG_LS_DIRECTION_DOWN:
                    deltaLY += (int16_t)(deltaPercent * joystickMid);
                    break;
                case GpioAction::ANALOG_LS_DIRECTION_LEFT:
                    deltaLX -= (int16_t)(deltaPercent * joystickMid);
                    break;
                case GpioAction::ANALOG_LS_DIRECTION_RIGHT:
                    deltaLX += (int16_t)(deltaPercent * joystickMid);
                    break;
                case GpioAction::ANALOG_RS_DIRECTION_UP:
                    deltaRY -= (int16_t)(deltaPercent * joystickMid);
                    break;
                case GpioAction::ANALOG_RS_DIRECTION_DOWN:
                    deltaRY += (int16_t)(deltaPercent * joystickMid);
                    break;
                case GpioAction::ANALOG_RS_DIRECTION_LEFT:
                    deltaRX -= (int16_t)(deltaPercent * joystickMid);
                    break;
                case GpioAction::ANALOG_RS_DIRECTION_RIGHT:
                    deltaRX += (int16_t)(deltaPercent * joystickMid);
                    break;
                case GpioAction::ANALOG_TRIGGER_L2:
                    if(deltaLT == 0) {
                        deltaLT = (int16_t)(deltaPercent * triggerMax);
                    } else if (deltaLT > 0) {
                        deltaLT = (int16_t)((deltaLT + (deltaPercent * triggerMax)) / 2.0);
                    }
                    break;
                case GpioAction::ANALOG_TRIGGER_R2:
                    if(deltaRT == 0) {
                        deltaRT = (int16_t)(deltaPercent * triggerMax);
                    } else if (deltaRT > 0) {
                        deltaRT = (int16_t)((deltaRT +  (deltaPercent * triggerMax)) / 2.0);
                    }
                    break;
                default:
                    break;
            }
        }

        gamepad->state.ly = constrain((int16_t)GAMEPAD_JOYSTICK_MID + deltaLY, GAMEPAD_JOYSTICK_MIN, GAMEPAD_JOYSTICK_MAX);
        gamepad->state.lx = constrain((int16_t)GAMEPAD_JOYSTICK_MID + deltaLX, GAMEPAD_JOYSTICK_MIN, GAMEPAD_JOYSTICK_MAX);
        gamepad->state.ry = constrain((int16_t)GAMEPAD_JOYSTICK_MID + deltaRY, GAMEPAD_JOYSTICK_MIN, GAMEPAD_JOYSTICK_MAX);
        gamepad->state.rx = constrain((int16_t)GAMEPAD_JOYSTICK_MID + deltaRX, GAMEPAD_JOYSTICK_MIN, GAMEPAD_JOYSTICK_MAX);
        gamepad->state.lt = constrain((int16_t)GAMEPAD_TRIGGER_MIN + deltaLT, GAMEPAD_TRIGGER_MIN, GAMEPAD_TRIGGER_MAX);
        gamepad->state.rt = constrain((int16_t)GAMEPAD_TRIGGER_MIN + deltaRT, GAMEPAD_TRIGGER_MIN, GAMEPAD_TRIGGER_MAX);

        analogChangeQueue.pop();

        // printf("%5u %5u %5u %5u %3u %3u", gamepad->state.lx, gamepad->state.ly, gamepad->state.rx, gamepad->state.ry, gamepad->state.lt, gamepad->state.rt);
    }
}

void AnalogButtonAddon::updateButtonRange(AnalogButton &button) {
    // Calculate the value with the deadzone in the positive and negative direction applied.
    uint16_t upperValue = button.smaValue - ANALOG_BUTTON_DEADZONE;
    uint16_t lowerValue = button.smaValue + ANALOG_BUTTON_DEADZONE;

    // If the read value with deadzone applied is bigger than the current rest position, update it.
    if (button.restPosition < upperValue) {
        button.restPosition = upperValue;

    // If the read value with deadzone applied is lower than the current down position, update it. Make sure that the distance to the rest position
    // is at least SENSOR_BOUNDARY_MIN_DISTANCE (scaled with travel distance @ 4.00mm) to prevent poor calibration/analog range resulting in "crazy behaviour".
    } else if (button.downPosition > lowerValue && button.restPosition - lowerValue >= (ANALOG_BUTTON_TOTAL_TRAVEL / 2) * ANALOG_BUTTON_TOTAL_TRAVEL / ANALOG_BUTTON_TOTAL_TRAVEL) {
    // From here on, the down position has been set < rest position, therefore the key can be considered calibrated, allowing distance calculation.
        button.calibrated = true;

        button.downPosition = lowerValue;
    }
}

void AnalogButtonAddon::processDigitalButton(AnalogButton &button) {
    const AnalogButtonOptions &analogButtonOptions = Storage::getInstance().getAddonOptions().analogButtonOptions;
    Gamepad *gamepad = Storage::getInstance().GetGamepad();

    switch (ANALOG_BUTTON_TRIGGER_MODE) {
        case AnalogTriggerMode::STATIC_TRIGGER:
            if(button.distance > ANALOG_BUTTON_ACTUATION_POINT) {
                triggerButtonPress(button);
            }
            break;
        default:
            rapidTrigger(button);
            break;
    }
}

void AnalogButtonAddon::rapidTrigger(AnalogButton &button) {
    // Reset RapidTrigger state if button leaves the rapid trigger zone

    switch(ANALOG_BUTTON_TRIGGER_MODE) {
        case AnalogTriggerMode::RAPID_TRIGGER:
            if (button.distance <= ANALOG_BUTTON_ACTUATION_POINT - ANALOG_BUTTON_RELEASE_THRESHOLD) {
                button.inRapidTriggerZone = false;
            }
            break;
        case AnalogTriggerMode::CONTINUOUS_RAPID_TRIGGER:
            if (button.distance <= CONTINUOUS_RAPID_THRESHOLD) {
                button.inRapidTriggerZone = false;
            }
            break;
    }

    if (button.distance >= ANALOG_BUTTON_ACTUATION_POINT && !button.inRapidTriggerZone) {
        button.inRapidTriggerZone = true;
        button.pressed = true;
    } else if (!button.pressed && button.inRapidTriggerZone && button.distance >= button.localMax + ANALOG_BUTTON_PRESS_THRESHOLD) {
        button.pressed = true;
    } else if (button.pressed && (!button.inRapidTriggerZone || button.distance <= button.localMax - ANALOG_BUTTON_RELEASE_THRESHOLD)) {
        button.pressed = false;
    }

    if ((button.pressed && button.distance > button.localMax) || (!button.pressed && button.distance < button.localMax)) {
        button.localMax = button.distance;
    }

    if(button.pressed) {
        printf("pressed ");
        triggerButtonPress(button);
    } else {
        printf("ready   ");
    }
}

void AnalogButtonAddon::triggerButtonPress(AnalogButton button) {
    const AnalogButtonOptions &analogButtonOptions = Storage::getInstance().getAddonOptions().analogButtonOptions;
    Gamepad *gamepad = Storage::getInstance().GetGamepad();

    //TODO: Add SOCD Cleaning to DPad inputs
    const SOCDMode socdMode = getSOCDMode(gamepad->getOptions());

    switch (button.gpioMappingInfo.action) {
        case GpioAction::BUTTON_PRESS_UP:
            gamepad->state.dpad |= GAMEPAD_MASK_UP;
            break;
        case GpioAction::BUTTON_PRESS_DOWN:
            gamepad->state.dpad |= GAMEPAD_MASK_DOWN;
            break;
        case GpioAction::BUTTON_PRESS_LEFT:
            gamepad->state.dpad |= GAMEPAD_MASK_LEFT;
            break;
        case GpioAction::BUTTON_PRESS_RIGHT:
            gamepad->state.dpad |= GAMEPAD_MASK_RIGHT;
            break;
        case GpioAction::BUTTON_PRESS_B1:
            gamepad->state.buttons |= GAMEPAD_MASK_B1;
            break;
        case GpioAction::BUTTON_PRESS_B2:
            gamepad->state.buttons |= GAMEPAD_MASK_B2;
            break;
        case GpioAction::BUTTON_PRESS_B3:
            gamepad->state.buttons |= GAMEPAD_MASK_B3;
            break;
        case GpioAction::BUTTON_PRESS_B4:
            gamepad->state.buttons |= GAMEPAD_MASK_B4;
            break;
        case GpioAction::BUTTON_PRESS_L1:
            gamepad->state.buttons |= GAMEPAD_MASK_L1;
            break;
        case GpioAction::BUTTON_PRESS_R1:
            gamepad->state.buttons |= GAMEPAD_MASK_R1;
            break;
        case GpioAction::BUTTON_PRESS_L2:
            gamepad->state.buttons |= GAMEPAD_MASK_L2;
            break;
        case GpioAction::BUTTON_PRESS_R2:
            gamepad->state.buttons |= GAMEPAD_MASK_R2;
            break;
        case GpioAction::BUTTON_PRESS_S1:
            gamepad->state.buttons |= GAMEPAD_MASK_S1;
            break;
        case GpioAction::BUTTON_PRESS_S2:
            gamepad->state.buttons |= GAMEPAD_MASK_S2;
            break;
        case GpioAction::BUTTON_PRESS_L3:
            gamepad->state.buttons |= GAMEPAD_MASK_L3;
            break;
        case GpioAction::BUTTON_PRESS_R3:
            gamepad->state.buttons |= GAMEPAD_MASK_R3;
            break;
        case GpioAction::BUTTON_PRESS_A1:
            gamepad->state.buttons |= GAMEPAD_MASK_A1;
            break;
        case GpioAction::BUTTON_PRESS_A2:
            gamepad->state.buttons |= GAMEPAD_MASK_A2;
            break;
        case GpioAction::BUTTON_PRESS_FN:
            gamepad->state.buttons |= AUX_MASK_FUNCTION;
            break;
        default:
            break;
    }
}

void AnalogButtonAddon::SOCDClean(SOCDMode socdMode) { 
}

const SOCDMode AnalogButtonAddon::getSOCDMode(const GamepadOptions& options) {
    return Gamepad::resolveSOCDMode(options);
}