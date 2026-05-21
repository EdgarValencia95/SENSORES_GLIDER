################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../if_c/i2c_if.c \
../if_c/spi_if.c \
../if_c/system_clock_if.c \
../if_c/uart_if.c 

OBJS += \
./if_c/i2c_if.o \
./if_c/spi_if.o \
./if_c/system_clock_if.o \
./if_c/uart_if.o 

C_DEPS += \
./if_c/i2c_if.d \
./if_c/spi_if.d \
./if_c/system_clock_if.d \
./if_c/uart_if.d 


# Each subdirectory must supply rules for building sources it contributes
if_c/%.o if_c/%.su if_c/%.cyclo: ../if_c/%.c if_c/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F4 -DSTM32F405RGTx -c -I../Inc -I"C:/Users/son_e/STM32CubeIDE/workspace_2.1.1/PATHS/Material_Especialidad_2026-main/Material_Especialidad_2026-main/Clase_02_Chip_headers/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"C:/Users/son_e/STM32CubeIDE/workspace_2.1.1/PATHS/Material_Especialidad_2026-main/Material_Especialidad_2026-main/Clase_02_Chip_headers/Drivers/CMSIS/Include" -I"C:/Users/son_e/STM32CubeIDE/WS_glider/Plantilla_STM32F405RGT6/if_h" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-if_c

clean-if_c:
	-$(RM) ./if_c/i2c_if.cyclo ./if_c/i2c_if.d ./if_c/i2c_if.o ./if_c/i2c_if.su ./if_c/spi_if.cyclo ./if_c/spi_if.d ./if_c/spi_if.o ./if_c/spi_if.su ./if_c/system_clock_if.cyclo ./if_c/system_clock_if.d ./if_c/system_clock_if.o ./if_c/system_clock_if.su ./if_c/uart_if.cyclo ./if_c/uart_if.d ./if_c/uart_if.o ./if_c/uart_if.su

.PHONY: clean-if_c

