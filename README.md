# Raspberry Pi Pico W FreeRTOS Starter - In C

This is a simple blinky starter project for Raspberry Pi Pico W that uses FreeRTOS

## Important Setup

- Clone [Pico SDK](https://github.com/raspberrypi/pico-sdk) and inside the cloned SDK directory run `git submodule update --init` to init all submodules

- Clone [FreeRTOS Kernel](https://github.com/FreeRTOS/FreeRTOS-Kernel)

- Pico SDK should be present in the machine and it's path should be used as an environment variable as `PICO_SDK_PATH` pointing to the cloned Pico SDK dir

- FreeRTOS-Kernel should be present in the machine and it's path should be used as an environment variable as `FREERSTOS_KERNEL_PATH` pointing to the cloned FreeRTOS-Kernel dir

These environment variable should be used when calling `CMake` or defined in VSCode(**RECOMENDED**) using [this setup](https://www.youtube.com/watch?v=BAoTBg8MJJ4) that uses the `CMake Tools` extension.

## Project Rename

To rename the project simply open the root `CMakeLists.txt` and change `project(pico_freertos C CXX ASM)` to `project(your_project_name C CXX ASM)`.

Inspired by the [Learn Embedded Systems video series](https://www.youtube.com/watch?v=jCZxStjzGA8&list=PLEB5F4gTNK68IlRIJtcJ_2cW4dSdmreTw&index=14) on YouTube.
