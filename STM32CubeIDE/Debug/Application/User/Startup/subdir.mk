################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Application/User/Startup/startup_stm32f746nghx.s 

S_DEPS += \
./Application/User/Startup/startup_stm32f746nghx.d 

OBJS += \
./Application/User/Startup/startup_stm32f746nghx.o 


# Each subdirectory must supply rules for building sources it contributes
Application/User/Startup/startup_stm32f746nghx.o: ../Application/User/Startup/startup_stm32f746nghx.s
	arm-none-eabi-gcc -mcpu=cortex-m7 -g3 -c -x assembler-with-cpp -MMD -MP -MF"Application/User/Startup/startup_stm32f746nghx.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

