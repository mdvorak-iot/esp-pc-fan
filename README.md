# esp-pc-fan

![platformio build](https://github.com/mdvorak-iot/esp-pc-fan/workflows/platformio%20build/badge.svg)

Common 4-pin PWM fan controller.

## Usage

To reference this library by your project, add it as git submodule, using command

```shell
git submodule add https://github.com/mdvorak-iot/esp-pc-fan.git components/pc_fan
```

and include either of the header files

```c
#include <pc_fan_control.h>
#include <pc_fan_rpm.h>
```

For full example, see [pc_fan_example_main.c](./example/main/pc_fan_example_main.c).

### Sampling

When using sampling, ideal number of samples used for average depends on sampling interval, and desired response time to
changes. More samples provides best accuracy, but also react to RPM changes slowly.

For continuous monitoring, good values are 5 samples and sampling every 200 ms - that is, average on last second.

But if the readouts are in longer interval, e.g. 5 seconds, you may want to use just one sample, without any smoothing
at all. Experiment with the values to suit your use-case.

## 4-Wire Fan Pins

Standard pinout of PC PWM controlled fan.

![Fan Connector](./assets/connector_mb_4pin_header.png)

| Pin | Name    | Color Scheme 1 | Color Scheme 2 |
|-----|---------|----------------|----------------|
| 1   | GND     | Black          | Black          |
| 2   | +12VDC  | Yellow         | Red            |
| 3   | Sense   | Green          | Yellow         |
| 4   | Control | Blue           | Blue           |

This applies to 3-PIN fan as well, except voltage on PIN 2 is used to control the RPM, and PIN 4 is obviously missing.

## Development

Prepare [ESP-IDF development environment](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#get-started-get-prerequisites)
.

Configure example application with

```
cd example/
idf.py menuconfig
```

Flash it via (in the example dir)

```
idf.py build flash monitor
```

As an alternative, you can use [PlatformIO](https://docs.platformio.org/en/latest/core/installation.html) to build and
flash the example project.
