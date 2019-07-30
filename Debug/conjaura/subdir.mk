################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../conjaura/colour.c \
../conjaura/data.c \
../conjaura/data_stream.c \
../conjaura/globals.c \
../conjaura/panel_config.c 

OBJS += \
./conjaura/colour.o \
./conjaura/data.o \
./conjaura/data_stream.o \
./conjaura/globals.o \
./conjaura/panel_config.o 

C_DEPS += \
./conjaura/colour.d \
./conjaura/data.d \
./conjaura/data_stream.d \
./conjaura/globals.d \
./conjaura/panel_config.d 


# Each subdirectory must supply rules for building sources it contributes
conjaura/%.o: ../conjaura/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -mfloat-abi=soft '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32G070xx -I"G:/STM32/conjaura_pimcu/Inc" -I"G:/STM32/conjaura_pimcu/conjaura" -I"G:/STM32/conjaura_pimcu/Drivers/STM32G0xx_HAL_Driver/Inc" -I"G:/STM32/conjaura_pimcu/Drivers/STM32G0xx_HAL_Driver/Inc/Legacy" -I"G:/STM32/conjaura_pimcu/Drivers/CMSIS/Device/ST/STM32G0xx/Include" -I"G:/STM32/conjaura_pimcu/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


