################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
D:/RezalniSistemV2/Software/RezalniSistemV2/Drivers/BSP/stm32746g_discovery_qspi.c 

C_DEPS += \
./Drivers/BSP/stm32746g_discovery_qspi.d 

OBJS += \
./Drivers/BSP/stm32746g_discovery_qspi.o 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/stm32746g_discovery_qspi.o: D:/RezalniSistemV2/Software/RezalniSistemV2/Drivers/BSP/stm32746g_discovery_qspi.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F746xx -DDEBUG -c -I../../Core/Inc -I../../TouchGFX/App -I../../TouchGFX/target/generated -I../../TouchGFX/target -I../../Drivers/STM32F7xx_HAL_Driver/Inc -I../../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I../../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../../Drivers/CMSIS/Include -I../../Drivers/Components/ft5336 -I../../Drivers/BSP -I../../Middlewares/ST/touchgfx/framework/include -I../../TouchGFX/generated/fonts/include -I../../TouchGFX/generated/gui_generated/include -I../../TouchGFX/generated/images/include -I../../TouchGFX/generated/texts/include -I../../TouchGFX/gui/include -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Drivers/BSP/stm32746g_discovery_qspi.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

