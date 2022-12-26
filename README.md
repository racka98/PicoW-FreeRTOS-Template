# Raspberry Pi Pico W FreeRTOS Starter - In C

This is a simple blinky starter project for Raspberry Pi Pico W that uses FreeRTOS

## Important Configs

- Pico SDK should be present in the machine and it's path should be used as an environment variable as `PICO_SDK_PATH`
- FreeRTOS-Kernel should be present in the machine and it's path should be used as an environment variable as `FREERSTOS_KERNEL_PATH`

These environment variable should be used when calling `CMake` or defined in VSCode(**RECOMENDED**) using [this setup](https://www.youtube.com/watch?v=BAoTBg8MJJ4) that uses the `CMake Tools` extension.

Inspired by the [Learn Embedded Systems video series](https://www.youtube.com/watch?v=jCZxStjzGA8&list=PLEB5F4gTNK68IlRIJtcJ_2cW4dSdmreTw&index=14) on YouTube.
