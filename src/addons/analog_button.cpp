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
#define NUM_ADC_PINS 4

bool AnalogButtonAddon::available() {
    return Storage::getInstance().getAddonOptions().analogButtonOptions.enabled;
}

void AnalogButtonAddon::setup() {
    const AnalogButtonOptions &analogButtonOptions = Storage::getInstance().getAddonOptions().analogButtonOptions;
    stdio_init_all();

    for (size_t i = 0; i < NUM_ANALOG_BUTTONS; i++) {
        analogButtons[i].index = i;
        analogButtons[i].pin = analogButtonOptions.analogButtons[i].pin;
        analogButtons[i].gpioMappingInfo.action = analogButtonOptions.analogButtons[i].gpioAction;
    }

    for (size_t i = 0; i < NUM_ANALOG_BUTTONS; i++) {
        if (isValidPin(analogButtons[i].pin))
        {
            adc_gpio_init(analogButtons[i].pin);
        }
    }
} 

void AnalogButtonAddon::preprocess() {

}

void AnalogButtonAddon::process() {
    const AnalogButtonOptions &analogButtonOptions = Storage::getInstance().getAddonOptions().analogButtonOptions;
    Gamepad *gamepad = Storage::getInstance().GetGamepad();
    analogButtonDPadState = 0;

    for (size_t i = 0; i < NUM_ANALOG_BUTTONS; i++) {
        if (isValidPin(analogButtons[i].pin)) {
            // printButtonProp(analogButtons[i], "pin");

            readButton(analogButtons[i]);
            if (analogButtons[i].calibrated) {
                if(find(analogActions.begin(), analogActions.end(), analogButtons[i].gpioMappingInfo.action) != analogActions.end()) {
                    queueAnalogChange(analogButtons[i]);
                } else {
                    processDigitalButton(analogButtons[i]);
                }
            }   
        }
    }
    
    // printb(analogButtonDPadState);

    updateAnalogDPad();
    
    // printb(gamepad->state.dpad);

    updateAnalogState();

    // printGamepadState("analog");
    printf("\n");
}

void AnalogButtonAddon::bootProcess() {
    const AnalogButtonOptions &analogButtonOptions = Storage::getInstance().getAddonOptions().analogButtonOptions;
    Gamepad *gamepad = Storage::getInstance().GetGamepad();

    for (size_t i = 0; i < NUM_ANALOG_BUTTONS; i++) {
        if (isValidPin(analogButtons[i].pin)) {
            adc_select_input(analogButtons[i].pin - ADC_PIN_OFFSET);
        
            analogButtons[i].pressed = 3000 > adc_read();

            switch (analogButtons[i].gpioMappingInfo.action) {
            case BUTTON_PRESS_UP:
                gamepad->state.dpad |= analogButtons[i].pressed ? GAMEPAD_MASK_UP : 0;
                break;
            case BUTTON_PRESS_DOWN:
                gamepad->state.dpad |= analogButtons[i].pressed ? GAMEPAD_MASK_DOWN : 0;
                break;
            case BUTTON_PRESS_LEFT:
                gamepad->state.dpad |= analogButtons[i].pressed ? GAMEPAD_MASK_LEFT : 0;
                break;
            case BUTTON_PRESS_RIGHT:
                gamepad->state.dpad |= analogButtons[i].pressed ? GAMEPAD_MASK_RIGHT : 0;
                break;
            default:
                triggerButtonPress(analogButtons[i]);
                break;
            }
        }
    }
}

long AnalogButtonAddon::map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void AnalogButtonAddon::scaleVector(float &x, float &y, float magnitudeMin, float magnitudeMax, float scaleMin, float scaleMax) {
    float angle = atan2(y, x);
    float xy_magnitude = sqrt((x * x) + (y * y));
    float scaledMagnitude = (xy_magnitude - magnitudeMin) / (magnitudeMax - magnitudeMin);

    // printf("%1.4f ", xy_magnitude);
    
    x = x / xy_magnitude;
    y = y / xy_magnitude;
}

void AnalogButtonAddon::readButton(AnalogButton &button) {
    const AnalogButtonOptions &analogButtonOptions = Storage::getInstance().getAddonOptions().analogButtonOptions;

    adc_select_input(button.pin - ADC_PIN_OFFSET);
    button.rawValue = adc_read();
    button.smaValue = button.filter(button.rawValue);

    if (analogButtonOptions.poleSensorOrientation == -1) {
        button.smaValue = (1 << ANALOG_RESOLUTION) - 1 - button.smaValue;
    }

    if (button.filter.initialized){
        updateButtonRange(button);
    }

    if (!button.calibrated) {
        button.distance = analogButtonOptions.totalTravel;
        return;
    }

    button.distance = constrain(map(button.smaValue, button.restPosition, button.downPosition, 0, analogButtonOptions.totalTravel), 0, analogButtonOptions.totalTravel);

    printGpioAction(button.gpioMappingInfo.action);
    printf("%4u ", button.smaValue);
    // printf("%4u ", button.restPosition);
    // printf("%4u ", button.downPosition);
    // printf("%1.2fmm ", (float)button.distance / 100.0);
}

void AnalogButtonAddon::updateButtonRange(AnalogButton &button) {
    const AnalogButtonOptions &analogButtonOptions = Storage::getInstance().getAddonOptions().analogButtonOptions;

    // Calculate the value with the deadzone in the positive and negative direction applied.
    uint16_t upperValue = button.smaValue - analogButtonOptions.analogDeadzone;
    uint16_t lowerValue = button.smaValue + analogButtonOptions.analogDeadzone;

    // If the read value with deadzone applied is bigger than the current down position, update it.
    if (button.downPosition < upperValue) {
        button.downPosition = upperValue;

    // If the read value with deadzone applied is lower than the current down position, update it. 
    // Make sure that the distance to the rest position is at least SENSOR_BOUNDARY_MIN_DISTANCE (scaled with travel distance @ 4.00mm) to prevent poor calibration/analog range resulting in "crazy behaviour".
    } else if (button.restPosition > lowerValue && button.downPosition - upperValue >= (analogButtonOptions.totalTravel / 2) * analogButtonOptions.totalTravel / analogButtonOptions.totalTravel) {
    // From here on, the down position has been set < rest position, therefore the key can be considered calibrated, allowing distance calculation.
        button.calibrated = true;

        button.restPosition = lowerValue;
    }

    // printButtonProp(button);

}

void AnalogButtonAddon::processDigitalButton(AnalogButton &button) {
    const AnalogButtonOptions &analogButtonOptions = Storage::getInstance().getAddonOptions().analogButtonOptions;
    Gamepad *gamepad = Storage::getInstance().GetGamepad();

    switch (analogButtonOptions.analogButtons[button.index].actuationOptions.triggerMode) {
        case AnalogTriggerType::STATIC_TRIGGER:
            if(button.distance > analogButtonOptions.analogButtons[button.index].actuationOptions.actuationPoint) {
                triggerButtonPress(button);
            }
            break;
        default:
            rapidTrigger(button);
            break;
    }
}

void AnalogButtonAddon::rapidTrigger(AnalogButton &button) {
    const AnalogButtonOptions &analogButtonOptions = Storage::getInstance().getAddonOptions().analogButtonOptions;


    // Reset RapidTrigger state if button leaves the rapid trigger zone
    switch(analogButtonOptions.analogButtons[button.index].actuationOptions.triggerMode) {
        // Reset once past actuation point
        case AnalogTriggerType::RAPID_TRIGGER:
            if (button.distance <= analogButtonOptions.analogButtons[button.index].actuationOptions.actuationPoint - analogButtonOptions.analogButtons[button.index].actuationOptions.releaseThreshold) {
                button.inRapidTriggerZone = false;
            }
            break;
        // Reset once fully released
        case AnalogTriggerType::CONTINUOUS_RAPID_TRIGGER:
            if (button.distance <= CONTINUOUS_RAPID_TRIGGER_THRESHOLD) {
                button.inRapidTriggerZone = false;
            }
            break;
    }

    // Press and set in RTZ once past actuation point
    if (button.distance >= analogButtonOptions.analogButtons[button.index].actuationOptions.actuationPoint && !button.inRapidTriggerZone) {
        button.inRapidTriggerZone = true;
        button.pressed = true;

    // Press if button is in RTZ and has moved sufficiently downwards to trigger again
    } else if (!button.pressed && button.inRapidTriggerZone && button.distance >= button.localMax + analogButtonOptions.analogButtons[button.index].actuationOptions.pressThreshold) {
        button.pressed = true;

    // Release button if either not in RTZ or is in RTZ, but has moved sufficiently upwards
    } else if (button.pressed && (!button.inRapidTriggerZone || button.distance <= button.localMax - analogButtonOptions.analogButtons[button.index].actuationOptions.releaseThreshold)) {
        button.pressed = false;
    }

    // Updates maximum distance for calculating whether button should trigger again after releasing 
    if ((button.pressed && button.distance > button.localMax) || (!button.pressed && button.distance < button.localMax)) {
        button.localMax = button.distance;
    }

    if(button.pressed) {
        triggerButtonPress(button);
    }
}

void AnalogButtonAddon::triggerButtonPress(AnalogButton button) {
    const AnalogButtonOptions &analogButtonOptions = Storage::getInstance().getAddonOptions().analogButtonOptions;
    Gamepad *gamepad = Storage::getInstance().GetGamepad();

    //TODO: Add SOCD Cleaning to DPad inputs
    switch (button.gpioMappingInfo.action) {
        case GpioAction::BUTTON_PRESS_UP:
            analogButtonDPadState |= GAMEPAD_MASK_UP;
            break;
        case GpioAction::BUTTON_PRESS_DOWN:
            analogButtonDPadState |= GAMEPAD_MASK_DOWN;
            break;
        case GpioAction::BUTTON_PRESS_LEFT:
            analogButtonDPadState |= GAMEPAD_MASK_LEFT;
            break;
        case GpioAction::BUTTON_PRESS_RIGHT:
            analogButtonDPadState |= GAMEPAD_MASK_RIGHT;
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

void AnalogButtonAddon::queueAnalogChange(AnalogButton button) {
    AnalogChange analogChange = {
        button.gpioMappingInfo.action,
        button.restPosition,
        button.downPosition,
        button.smaValue,
    };

    // printGpioAction(analogChange.gpioAction);
    // printButtonProp( button,"restPosition");
    // printButtonProp( button,"downPosition");
    // printButtonProp( button,"newValue");

    analogChangeQueue.push(analogChange);
}

void AnalogButtonAddon::updateAnalogState() {
    const AnalogButtonOptions &analogButtonOptions = Storage::getInstance().getAddonOptions().analogButtonOptions;
    Gamepad *gamepad = Storage::getInstance().GetGamepad();
    gamepad->hasAnalogTriggers = true;

    float joystickMid = (float)GAMEPAD_JOYSTICK_MID;
    float triggerMax = (float)GAMEPAD_TRIGGER_MAX;

    int16_t deltaLY = 0;
    int16_t deltaLX = 0;
    int16_t deltaRY = 0;
    int16_t deltaRX = 0;
    int16_t deltaLT = 0;
    int16_t deltaRT = 0;

    float deltaPercentLY = 0;
    float deltaPercentLX = 0;
    float deltaPercentRY = 0;
    float deltaPercentRX = 0;
    float deltaPercentLT = 0;
    float deltaPercentRT = 0;

    for (size_t i = 0; i < size(analogChangeQueue); i++) {
        GpioAction gpioAction = analogChangeQueue.front().gpioAction;
        int16_t restPosition = analogChangeQueue.front().restPosition;
        int16_t downPosition = analogChangeQueue.front().downPosition;
        int16_t newValue = analogChangeQueue.front().newValue;

        float deltaPercent = 0.0;

        if ((newValue - restPosition) > analogButtonOptions.analogDeadzone) {
            deltaPercent = constrain((float)(newValue - restPosition) / (float)(downPosition - restPosition), 0.0, 1.0);
            switch(gpioAction) {
                case GpioAction::ANALOG_LS_DIRECTION_UP:
                    deltaPercentLY -= deltaPercent;
                    break;
                case GpioAction::ANALOG_LS_DIRECTION_DOWN:
                    deltaPercentLY += deltaPercent;
                    break;
                case GpioAction::ANALOG_LS_DIRECTION_LEFT:
                    deltaPercentLX -= deltaPercent;
                    break;
                case GpioAction::ANALOG_LS_DIRECTION_RIGHT:
                    deltaPercentLX += deltaPercent;
                    break;
                case GpioAction::ANALOG_RS_DIRECTION_UP:
                    deltaPercentRY -= deltaPercent;
                    break;
                case GpioAction::ANALOG_RS_DIRECTION_DOWN:
                    deltaPercentRY += deltaPercent;
                    break;
                case GpioAction::ANALOG_RS_DIRECTION_LEFT:
                    deltaPercentRX -= deltaPercent;
                    break;
                case GpioAction::ANALOG_RS_DIRECTION_RIGHT:
                    deltaPercentRX += deltaPercent;
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

        analogChangeQueue.pop();
    }

    float magnitudeLXY = sqrt((deltaPercentLX * deltaPercentLX) + (deltaPercentLY * deltaPercentLY));
    float magnitudeRXY = sqrt((deltaPercentRX * deltaPercentRX) + (deltaPercentRY * deltaPercentRY));

    if (analogButtonOptions.enforceCircularity) {
        if (magnitudeLXY > 1.0) {
            scaleVector(deltaPercentLX, deltaPercentLY, 0.0, sqrt(2.0), 0.0, 1.0);
        } else if (magnitudeRXY > 1.0) {
            scaleVector(deltaPercentRX, deltaPercentRY, 0.0, sqrt(2.0), 0.0, 1.0);
        }
    }

    deltaLY = (int16_t)(deltaPercentLY * joystickMid);
    deltaLX = (int16_t)(deltaPercentLX * joystickMid);
    deltaRY = (int16_t)(deltaPercentRY * joystickMid);
    deltaRX = (int16_t)(deltaPercentRX * joystickMid);

    // printGamepadState("analog");

    gamepad->state.ly = constrain((int16_t)GAMEPAD_JOYSTICK_MID + deltaLY, GAMEPAD_JOYSTICK_MIN, GAMEPAD_JOYSTICK_MAX);
    gamepad->state.lx = constrain((int16_t)GAMEPAD_JOYSTICK_MID + deltaLX, GAMEPAD_JOYSTICK_MIN, GAMEPAD_JOYSTICK_MAX);
    gamepad->state.ry = constrain((int16_t)GAMEPAD_JOYSTICK_MID + deltaRY, GAMEPAD_JOYSTICK_MIN, GAMEPAD_JOYSTICK_MAX);
    gamepad->state.rx = constrain((int16_t)GAMEPAD_JOYSTICK_MID + deltaRX, GAMEPAD_JOYSTICK_MIN, GAMEPAD_JOYSTICK_MAX);
    gamepad->state.lt = constrain((int16_t)GAMEPAD_TRIGGER_MIN + deltaLT, GAMEPAD_TRIGGER_MIN, GAMEPAD_TRIGGER_MAX);
    gamepad->state.rt = constrain((int16_t)GAMEPAD_TRIGGER_MIN + deltaRT, GAMEPAD_TRIGGER_MIN, GAMEPAD_TRIGGER_MAX);

}

void AnalogButtonAddon::updateAnalogDPad() {
    Gamepad *gamepad = Storage::getInstance().GetGamepad();
    SOCDMode socdMode = gamepad->resolveSOCDMode(gamepad->getOptions());

    // printSOCDMode(socdMode);

    gamepad->state.dpad = runSOCDCleaner(socdMode, analogButtonDPadState);
}

//TODO: DEBUG - Remove once complete
void AnalogButtonAddon::printGamepadState(string property) {
    Gamepad *gamepad = Storage::getInstance().GetGamepad();

    if (property == "buttonMask")    {
        printf("%32u", gamepad->state.buttons);
    } else if (property == "lx")    {
        printf("%5u", gamepad->state.lx);
    } else if (property == "ly")    {
        printf("%5u", gamepad->state.ly);
    } else if (property == "rx")    {
        printf("%5u", gamepad->state.rx);
    } else if (property == "ry")    {
        printf("%5u", gamepad->state.ry);
    } else if (property == "lt")    {
        printf("%5u", gamepad->state.lt);
    } else if (property == "rt")    {
        printf("%5u", gamepad->state.rt);
    } else if(property == "analog") {
        printf("%5u %5u %5u %5u %5u %5u", gamepad->state.lx, gamepad->state.ly, gamepad->state.rx, gamepad->state.ry, gamepad->state.lt, gamepad->state.rt);
    }

    printf(" ");

}

void AnalogButtonAddon::printButtonProp(AnalogButton button, string property) {

    if(property == "index") {
        printf("%4u", button.index);
    } else if(property == "pin") {
        printf("%4u", button.pin);
    } else if(property == "gpioAction") {
        printGpioAction(button.gpioMappingInfo.action);
    } else if(property == "rawValue") {
        printf("%4u", button.rawValue);
    } else if(property == "smaValue") {
        printf("%4u", button.smaValue);
    } else if(property == "restPosition") {
        printf("%4u", button.restPosition);
    } else if(property == "downPosition") {
        printf("%4u", button.downPosition);
    } else if(property == "distance") {
        printf("%1.2fmm ", (float)button.distance / 100.0);
    } else if(property == "localMax") {
        printf("%4u", button.localMax);
    } else if(property == "calibrated") {
        printf("%4u", button.calibrated);
    } else if(property == "pressed") {
        printf("%4u", button.pressed);
    } else if(property == "inRapidTriggerZone") {
        printf("%4u", button.inRapidTriggerZone);
    } else {
        printGpioAction(button.gpioMappingInfo.action);
        printf("%4u ", button.rawValue);
        printf("%4u ", button.smaValue);
        printf("%4u ", button.restPosition);
        printf("%4u ", button.downPosition);
        printf("%1.2fmm ", (float)button.distance / 100.0);
        printf("%4u ", button.localMax);
        printf("%4u ", button.calibrated);
        printf("%4u ", button.pressed);
        printf("%4u ", button.pressed);
        printf("%4u", button.inRapidTriggerZone);
    }

    printf(" ");

}

void AnalogButtonAddon::printGpioAction(GpioAction gpioAction) {
    switch (gpioAction)
    {
    case GpioAction::ANALOG_LS_DIRECTION_UP:
        printf("LSU ");
        break;
    case GpioAction::ANALOG_LS_DIRECTION_DOWN:
        printf("LSD ");
        break;
    case GpioAction::ANALOG_LS_DIRECTION_LEFT:
        printf("LSL ");
        break;
    case GpioAction::ANALOG_LS_DIRECTION_RIGHT:
        printf("LSR ");
        break;
    case GpioAction::ANALOG_RS_DIRECTION_UP:
        printf("RSU ");
        break;
    case GpioAction::ANALOG_RS_DIRECTION_DOWN:
        printf("RSD ");
        break;
    case GpioAction::ANALOG_RS_DIRECTION_LEFT:
        printf("RS  ");
        break;
    case GpioAction::ANALOG_RS_DIRECTION_RIGHT:
        printf("RSR ");
        break;
    case GpioAction::ANALOG_TRIGGER_L2:
        printf("LT  ");
        break;
    case GpioAction::ANALOG_TRIGGER_R2:
        printf("RT  ");
        break;
        break;
    case GpioAction::NONE:
        printf("NON ");
        break;
    default:
        printf("O/M ");
        break;
    }

    printf(" ");
}

void AnalogButtonAddon::printSOCDMode(SOCDMode socdMode) {
        switch (socdMode)
    {
    case SOCD_MODE_UP_PRIORITY:
        printf("SOCD-U");
        break;
    case SOCD_MODE_NEUTRAL:
        printf("SOCD-N");
        break;
    case SOCD_MODE_SECOND_INPUT_PRIORITY:
        printf("SOCD-S");
        break;
    case SOCD_MODE_FIRST_INPUT_PRIORITY:
        printf("SOCD-F");
        break;
    case SOCD_MODE_BYPASS:
        printf("SOCD-B");
        break;
    default:
        printf("SOCD-?");
        break;
    }

    printf(" ");
}

void AnalogButtonAddon::printb(unsigned int v) {
    unsigned int mask=sizeof(v) * CHAR_BIT;
    while(mask) {
        printf("%d", (v&mask ? 1 : 0));
        mask >>= 1;
    }
    printf(" ");
}

