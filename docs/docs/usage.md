# GP2040-CE Usage

Select the button labels to be displayed in the usage guide: <label-selector></label-selector>

!> The v0.5+ release has some breaking changes to boards other than the Raspberry Pi Pico. If you are using such a board, do not update from an older version unless you have a backup .uf2 from a known good version, or you know what you're doing.

## Buttons

GP2040-CE uses generic button labeling for gamepad state, which is then converted to the appropriate input type before sending. This table provides a map of GP2040-CE buttons to the supported input types and layouts:

| GP2040-CE | XInput | Switch  | PS4      | PS3      | DirectInput | Arcade |
| --------- | ------ | ------- | -------- | -------- | ----------- | ------ |
| B1        | A      | B       | Cross    | Cross    | 2           | K1     |
| B2        | B      | A       | Circle   | Circle   | 3           | K2     |
| B3        | X      | Y       | Square   | Square   | 1           | P1     |
| B4        | Y      | X       | Triangle | Triangle | 4           | P2     |
| L1        | LB     | L       | L1       | L1       | 5           | P4     |
| R1        | RB     | R       | R1       | R1       | 6           | P3     |
| L2        | LT     | ZL      | L2       | L2       | 7           | K4     |
| R2        | RT     | ZR      | R2       | R2       | 8           | K3     |
| S1        | Back   | Minus   | Share    | Select   | 9           | Coin   |
| S2        | Start  | Plus    | Options  | Start    | 10          | Start  |
| L3        | LS     | LS      | L3       | L3       | 11          | LS     |
| R3        | RS     | RS      | R3       | R3       | 12          | RS     |
| A1        | Guide  | Home    | PS       | PS       | 13          | Home   |
| A2        | -      | Capture | Touchpad | -        | 14          | -      |

If your controller is missing dedicated buttons for any specific inputs, check the [Hotkeys page](hotkeys) on how to map a button combination that will emulate those missing inputs.

?> Unlike other controllers, Keyboard gets different keys for directional buttons. <br /> You can change the key mappings for Keyboard mode in [Webconfig mode > Configuration > Keyboard Mapping](web-configurator.md#keyboard-mapping)

| Direction | Keyboard    |
| --------- | ----------- |
| Up        | Up Arrow    |
| Down      | Down Arrow  |
| Left      | Left Arrow  |
| Right     | Right Arrow |

## Bootsel Mode

Bootsel Mode is the state of the board where firmware can be flashed onto the board. You will need to use this whenever there is an update or as part of the troubleshooting process.

You can boot into Bootsel Mode by either holding `["S1", "S2", "Up"]` buttons while plugging in the controller or by booting into the Web Configurator and then restarting in Bootsel Mode.

## Webconfig Mode

Webconfig Mode is the state of the board where built-in web browser-based configuration application is launched. From here, you can customize and configure your controller as needed.

You can boot into Webconfig Mode by holding the `["S2"]` button while plugging in the controller.

## Input Modes

GP2040-CE is compatible with a number of systems and input modes. To change the input mode, **hold one of the following buttons as the controller is plugged in:**

| Input Mode      | Button Held |
| :-------------- | :---------: |
| Nintendo Switch |  `["B1"]`   |
| XInput          |  `["B2"]`   |
| DirectInput/PS3 |  `["B2"]`   |
| PS4             |  `["B4"]`   |
| Keyboard        |  `["R2"]`   |

?> Input mode is saved across power cycles.

## Hotkeys

A number of useful hotkeys exist that change the functionality of GP2040 without needing to access the Web Configurator. Commonly used hotkeys include changing the directions to emulate a digital DPad, Left Analog Stick, Right Analog Stick or to change [Simultaneous Opposite Cardinal Direction (SOCD)](https://glossary.infil.net/?t=SOCD) cleaning modes. Different SOCD cleaning modes will ensure the controller obeys certain directional input rules when sending inputs to the computer or game console.

A number of hotkeys are enabled by default and if you are encountering issues with unexpected behavior on your controller, verify that you did not accidentally trigger a hotkey.

| Hotkeys Enabled By Default                        | Inputs                |
| :------------------------------------------------ | :-------------------- |
| [Home Button](hotkeys.md#home-button)             | `["S1","S2","Up"]`    |
| [Dpad Digital](hotkeys.md#dpad-digital)           | `["S1","S2","Down"]`  |
| [Dpad Left Analog](hotkeys.md#dpad-left-analog)   | `["S1","S2","Left"]`  |
| [Dpad Right Analog](hotkeys.md#dpad-right-analog) | `["S2","A1","Right"]` |
| [SOCD Up Priority](hotkeys.md#socd-up-priority)   | `["S2","A1","Up"]`    |
| [SOCD Neutral](hotkeys.md#socd-neutral)           | `["S2","A1","Down"]`  |
| [SOCD Last Wins](hotkeys.md#socd-last-win)        | `["S2","A1","Left"]`  |
| [SOCD Invert Y Axis](hotkeys.md#invert-y-axis)    | `["S2","A1","Right"]` |

?> Selected D-Pad mode and SOCD cleaning mode are saved across power cycles.

## Add-Ons and Additional Features

There are a number of add-ons that expand the functionality of GP2040-CE, such as [analog stick emulation](add-ons/analog) and [turbo functions](add-ons/turbo). Due to the large number of add-ons created by the community, they are located in a separate documentation page. Navigate to Add-Ons page for more information on the individual add-ons.