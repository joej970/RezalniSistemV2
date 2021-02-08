################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Middlewares/FreeRTOS/subdir.mk
-include Drivers/STM32F7xx_HAL_Driver/subdir.mk
-include Drivers/Components/subdir.mk
-include Drivers/CMSIS/subdir.mk
-include Drivers/BSP/subdir.mk
-include Application/User/gui/subdir.mk
-include Application/User/generated/subdir.mk
-include Application/User/TouchGFX/target/generated/subdir.mk
-include Application/User/TouchGFX/target/subdir.mk
-include Application/User/TouchGFX/App/subdir.mk
-include Application/User/Startup/subdir.mk
-include Application/User/Core/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

BUILD_ARTIFACT_NAME := CuttingSystemV2
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX := 
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME).$(BUILD_ARTIFACT_EXTENSION)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
CuttingSystemV2.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
CuttingSystemV2.list \

OBJCOPY_HEX += \
CuttingSystemV2.hex \

OBJCOPY_BIN += \
CuttingSystemV2.bin \


# All Target
all: main-build

# Main-build Target
main-build: CuttingSystemV2.elf secondary-outputs

# Tool invocations
CuttingSystemV2.elf: $(OBJS) $(USER_OBJS) D:\RezalniSistemV2\Software\RezalniSistemV2\STM32CubeIDE\STM32F746NGHX_FLASH.ld
	arm-none-eabi-g++ -o "CuttingSystemV2.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m7 -T"D:\RezalniSistemV2\Software\RezalniSistemV2\STM32CubeIDE\STM32F746NGHX_FLASH.ld" --specs=nosys.specs -Wl,-Map="CuttingSystemV2.map" -Wl,--gc-sections -static -LC:/Dev/STM32F746G-DISCO/test_update/Middlewares/ST/touchgfx/lib/core/cortex_m7/gcc -LC:/Dev/STM32F746G-DISCO/test_update/CuttingSystemV2/Middlewares/ST/touchgfx/lib/core/cortex_m7/gcc -LC:/Dev/STM32F746G-DISCO/CuttingSystemV2/CuttingSystemV2/Middlewares/ST/touchgfx/lib/core/cortex_m7/gcc -LD:/RezalniSistemV2/Software/RezalniSistemV2/Middlewares/ST/touchgfx/lib/core/cortex_m7/gcc --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -lstdc++ -lsupc++ -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

CuttingSystemV2.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "CuttingSystemV2.list"
	@echo 'Finished building: $@'
	@echo ' '

CuttingSystemV2.hex: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "CuttingSystemV2.hex"
	@echo 'Finished building: $@'
	@echo ' '

CuttingSystemV2.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "CuttingSystemV2.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents fail-specified-linker-script-missing warn-no-linker-script-specified
.SECONDARY:

-include ../makefile.targets
