#include "addons/analog_button.h"
#include "config.pb.h"
#include "enums.pb.h"
#include "hardware/adc.h"
#include "helper.h"
#include "storagemanager.h"

#include <math.h>
#include "analog_button.h"

#define ADC_PIN_OFFSET 26

bool AnalogButtonAddon::available() {
    return Storage::getInstance().getAddonOptions().analogButtonOptions.enabled;
}
void AnalogButtonAddon::setup() {
    stdio_init_all();

    const size_t num_adc_pins = NUM_ANALOG_BUTTONS;

    // TODO: Debug GpioAction::ANALOG_DIRECTION_RIGHT not mapping to with analogButtons[i].gpioMappingInfo.action
    for(size_t i = 0; i < num_adc_pins; i++) {
        analogButtons[i].pin = buttonPins[i];
        analogButtons[i].gpioMappingInfo.action = buttonActions[i];
        printf("Pin: %2u ", analogButtons[i].pin);
        printGpioAction(analogButtons[i].gpioMappingInfo);
    }

    for(size_t i = 0; i < NUM_ANALOG_BUTTONS; i++) {
        if(isValidPin(analogButtons[i].pin)) {
            adc_gpio_init(analogButtons[i].pin);
            analogButtons[i].restValue = adc_read();
        }
    }
}

void AnalogButtonAddon::process() {

    const AnalogButtonOptions &analogButtonOptions = Storage::getInstance().getAddonOptions().analogButtonOptions;
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    gamepad->hasAnalogTriggers = true;

    for(size_t i = 0; i < NUM_ANALOG_BUTTONS; i++) {
        if(isValidPin(analogButtons[i].pin)) {
            adc_select_input(analogButtons[i].pin - ADC_PIN_OFFSET);
            analogButtons[i].rawValue = adc_read();
            // printf("%4u ", analogButtons[i].rawValue);

            double value = (float)analogButtons[i].rawValue / (float)analogButtons[i].restValue - 1.0;

            switch (analogButtons[i].gpioMappingInfo.action)
            {
            case GpioAction::ANALOG_DIRECTION_UP:
                gamepad->state.ly = static_cast<uint16_t>(GAMEPAD_JOYSTICK_MAX * (0.5 - clamp(value, 0.0, 0.5)));
                break;
            case GpioAction::ANALOG_DIRECTION_DOWN:
                gamepad->state.ly = static_cast<uint16_t>(GAMEPAD_JOYSTICK_MAX * (0.5 + clamp(value, 0.0, 0.5)));
                break;
            case GpioAction::ANALOG_DIRECTION_LEFT:
                gamepad->state.lx = static_cast<uint16_t>(GAMEPAD_JOYSTICK_MAX * (0.5 - clamp(value, 0.0, 0.5)));
                break;
            case GpioAction::ANALOG_DIRECTION_RIGHT:
                gamepad->state.ly = static_cast<uint16_t>(GAMEPAD_JOYSTICK_MAX * (0.5 + clamp(value, 0.0, 0.5)));
                break;
            case GpioAction::ANALOG_TRIGGER_L2:
                gamepad->state.lt = (GAMEPAD_TRIGGER_MAX * (0.5 + clamp(value, 0.0, 0.5)));
                break;
            case GpioAction::ANALOG_TRIGGER_R2:
                gamepad->state.rt = (GAMEPAD_TRIGGER_MAX * (0.5 + clamp(value, 0.0, 0.5)));
                break;
            default:
                break;
            }
        }
    }

    // printf("\n");
    printf("%5u %5u %5u %5u %3u %3u\n", gamepad->state.lx, gamepad->state.ly, gamepad->state.rx, gamepad->state.ry, gamepad->state.lt, gamepad->state.rt);
}

void AnalogButtonAddon::printGpioAction(GpioMappingInfo gpioMappingInfo) {
    switch (gpioMappingInfo.action)
    {
    case GpioAction::ANALOG_DIRECTION_UP:
        printf("ANALOG_DIRECTION_UP\n");
        break;
    case GpioAction::ANALOG_DIRECTION_DOWN:
        printf("ANALOG_DIRECTION_DOWN\n");
        break;
    case GpioAction::ANALOG_DIRECTION_LEFT:
        printf("ANALOG_DIRECTION_LEFT\n");
        break;
    case GpioAction::BUTTON_PRESS_RIGHT:
        printf("BUTTON_PRESS_RIGHT\n");
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

void AnalogButtonAddon::queueAnalogChange(uint16_t analogInput, uint16_t analogValue, uint16_t lastAnalogValue) {
    
}

void AnalogButtonAddon::updateAnalogState() {

}

uint16_t AnalogButtonAddon::getAverage() {
    uint16_t values = 0;
    return values;
}

