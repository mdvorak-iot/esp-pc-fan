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
