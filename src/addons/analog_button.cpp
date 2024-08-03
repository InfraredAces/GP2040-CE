#include "addons/analog_button.h"
#include "config.pb.h"
#include "enums.pb.h"
#include "hardware/adc.h"
#include "helper.h"
#include "storagemanager.h"

#include <math.h>

#define ADC_PIN_OFFSET 26

bool AnalogButtonAddon::available() {
    return Storage::getInstance().getAddonOptions().analogButtonOptions.enabled;
}
void AnalogButtonAddon::setup() {
    stdio_init_all();

    const size_t num_adc_pins = NUM_ANALOG_BUTTONS;

    for(size_t i = 0; i < num_adc_pins; i++) {
        analogButtons[i].index = i;
        analogButtons[i].pin = i + ADC_PIN_OFFSET;
    }

    for(size_t i = 0; i < NUM_ANALOG_BUTTONS; i++) {
        if(isValidPin(analogButtons[i].pin)) {
            adc_gpio_init(analogButtons[i].pin);
            analogButtons[i].restValue = adc_read();
        }
    }
}

void AnalogButtonAddon::process() {

    // const AnalogButtonOptions &analogButtonOptions = Storage::getInstance().getAddonOptions().analogButtonOptions;
    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    for(size_t i = 0; i < NUM_ANALOG_BUTTONS; i++) {
        if(isValidPin(analogButtons[i].pin)) {
            adc_select_input(analogButtons[i].pin - ADC_PIN_OFFSET);
            analogButtons[i].rawValue = adc_read();
            double value = (float)analogButtons[i].rawValue / (float)analogButtons[i].restValue - 1.0;
            printf("Pin %u: %4.2f\n", analogButtons[i].pin, value);
            gamepad->state.lx = static_cast<uint16_t>(GAMEPAD_JOYSTICK_MAX * (0.5 + clamp(value, 0.0, 0.5)));
        }
    }
}