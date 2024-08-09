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

bool AnalogButtonAddon::available()
{
    return Storage::getInstance().getAddonOptions().analogButtonOptions.enabled;
}
void AnalogButtonAddon::setup()
{
    stdio_init_all();

    const size_t num_adc_pins = NUM_ANALOG_BUTTONS;

    for (size_t i = 0; i < num_adc_pins; i++)
    {
        analogButtons[i].pin = buttonPins[i];
        analogButtons[i].gpioMappingInfo.action = buttonActions[i];
        printf("Pin: %2u ", analogButtons[i].pin);
        printGpioAction(analogButtons[i].gpioMappingInfo);
    }

    for (size_t i = 0; i < NUM_ANALOG_BUTTONS; i++)
    {
        if (isValidPin(analogButtons[i].pin))
        {
            adc_gpio_init(analogButtons[i].pin);
        }
    }
}

void AnalogButtonAddon::process()
{

    const AnalogButtonOptions &analogButtonOptions = Storage::getInstance().getAddonOptions().analogButtonOptions;
    Gamepad *gamepad = Storage::getInstance().GetGamepad();
    gamepad->hasAnalogTriggers = true;

    for (size_t i = 0; i < NUM_ANALOG_BUTTONS; i++)
    {
        if (isValidPin(analogButtons[i].pin))
        {
            readButton(analogButtons[i]);
            // printf("%4u ", analogButtons[i].rawValue);
            // printf("%4u ", analogButtons[i].smaValue);
            // printf("%4u ", analogButtons[i].restPosition);
            // printf("%4u ", analogButtons[i].downPosition);
            printf("%4u ", analogButtons[i].distance);

            // switch (analogButtons[i].gpioMappingInfo.action)
            // {
            // case GpioAction::ANALOG_LS_DIRECTION_UP:
            //     newValue = GAMEPAD_JOYSTICK_MAX * (0.5 + valueChange);
            //     break;
            // case GpioAction::ANALOG_LS_DIRECTION_DOWN:
            //     newValue = GAMEPAD_JOYSTICK_MAX * (0.5 - valueChange);
            //     break;
            // case GpioAction::ANALOG_LS_DIRECTION_LEFT:
            //     newValue = GAMEPAD_JOYSTICK_MAX * (0.5 + valueChange);
            //     break;
            // case GpioAction::ANALOG_LS_DIRECTION_RIGHT:
            //     newValue = GAMEPAD_JOYSTICK_MAX * (0.5 - valueChange);
            //     break;
            // case GpioAction::ANALOG_RS_DIRECTION_UP:
            //     newValue = GAMEPAD_JOYSTICK_MAX * (0.5 + valueChange);
            //     break;
            // case GpioAction::ANALOG_RS_DIRECTION_DOWN:
            //     newValue = GAMEPAD_JOYSTICK_MAX * (0.5 - valueChange);
            //     break;
            // case GpioAction::ANALOG_RS_DIRECTION_LEFT:
            //     newValue = GAMEPAD_JOYSTICK_MAX * (0.5 + valueChange);
            //     break;
            // case GpioAction::ANALOG_RS_DIRECTION_RIGHT:
            //     newValue = GAMEPAD_JOYSTICK_MAX * (0.5 - valueChange);
            //     break;
            // case GpioAction::ANALOG_TRIGGER_L2:
            //     newValue = GAMEPAD_TRIGGER_MAX * (0.5 + valueChange);
            //     break;
            // case GpioAction::ANALOG_TRIGGER_R2:
            //     newValue = GAMEPAD_TRIGGER_MAX * (0.5 + valueChange);
            //     break;
            // default:
            //     // RapidTrigger
            //     break;
            // }

            // queueAnalogChange(analogButtons[i].gpioMappingInfo.action, static_cast<uint16_t>(newValue));
        }
    }

    // updateAnalogState();

    printf("\n");
    // printf("%5u %5u %5u %5u %3u %3u\n", gamepad->state.lx, gamepad->state.ly, gamepad->state.rx, gamepad->state.ry, gamepad->state.lt, gamepad->state.rt);
}

void AnalogButtonAddon::printGpioAction(GpioMappingInfo gpioMappingInfo)
{
    switch (gpioMappingInfo.action)
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

void AnalogButtonAddon::readButton(AnalogButton &button)
{
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
}

void AnalogButtonAddon::queueAnalogChange(GpioAction gpioAction, uint16_t newAnalogValue)
{
    for (size_t i = 0; i < size(analogChanges); i++)
    {
        if (gpioAction == analogChanges[i].gpioAction && newAnalogValue != analogChanges[i].lastValue)
        {
            analogChanges[i].newValue = newAnalogValue;
        }
    }
}

void AnalogButtonAddon::updateAnalogState()
{
    Gamepad *gamepad = Storage::getInstance().GetGamepad();
    gamepad->hasAnalogTriggers = true;

    uint16_t newLSAnalogValueY = (analogChanges[0].newValue + analogChanges[1].newValue) / 2;
    uint16_t newLSAnalogValueX = (analogChanges[2].newValue + analogChanges[3].newValue) / 2;

    // printf("%5u ", analogChanges[0].newValue);
    // printf("%5u ", analogChanges[1].newValue);
    // printf("%5u ", analogChanges[2].newValue);
    // printf("%5u ", analogChanges[3].newValue);

    printf("%5u ", newLSAnalogValueY);
    printf("%5u ", newLSAnalogValueX);

    gamepad->state.lx = static_cast<uint16_t>(clamp(GAMEPAD_JOYSTICK_MAX * newLSAnalogValueX, GAMEPAD_JOYSTICK_MIN, GAMEPAD_JOYSTICK_MAX));
    gamepad->state.ly = static_cast<uint16_t>(clamp(GAMEPAD_JOYSTICK_MAX * newLSAnalogValueY, GAMEPAD_JOYSTICK_MIN, GAMEPAD_JOYSTICK_MAX));

    uint16_t newRSAnalogValueY = (analogChanges[4].newValue + analogChanges[5].newValue) / 2;
    uint16_t newRSAnalogValueX = (analogChanges[6].newValue + analogChanges[7].newValue) / 2;

    // printf("%5u ", analogChanges[4].newValue);
    // printf("%5u ", analogChanges[5].newValue);
    // printf("%5u ", analogChanges[6].newValue);
    // printf("%5u ", analogChanges[7].newValue);

    printf("%5u ", newRSAnalogValueY);
    printf("%5u ", newRSAnalogValueX);

    gamepad->state.rx = static_cast<uint16_t>(clamp(GAMEPAD_JOYSTICK_MAX * newRSAnalogValueX, GAMEPAD_JOYSTICK_MIN, GAMEPAD_JOYSTICK_MAX));
    gamepad->state.ry = static_cast<uint16_t>(clamp(GAMEPAD_JOYSTICK_MAX * newRSAnalogValueY, GAMEPAD_JOYSTICK_MIN, GAMEPAD_JOYSTICK_MAX));
}

uint16_t AnalogButtonAddon::getAverage()
{
    uint16_t values = 0;
    return values;
}

void AnalogButtonAddon::updateButtonRange(AnalogButton &button)
{
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
