################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/i2c_driver.c \
../Src/main.c \
../Src/spi_driver.c \
../Src/syscalls.c \
../Src/sysmem.c \
../Src/system_clock_driver.c \
../Src/uart_driver.c 

OBJS += \
./Src/i2c_driver.o \
./Src/main.o \
./Src/spi_driver.o \
./Src/syscalls.o \
./Src/sysmem.o \
./Src/system_clock_driver.o \
./Src/uart_driver.o 

C_DEPS += \
./Src/i2c_driver.d \
./Src/main.d \
./Src/spi_driver.d \
./Src/syscalls.d \
./Src/sysmem.d \
./Src/system_clock_driver.d \
./Src/uart_driver.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F4 -DSTM32F405RGTx -c -I../Inc -I"C:/Users/son_e/STM32CubeIDE/workspace_2.1.1/PATHS/Material_Especialidad_2026-main/Material_Especialidad_2026-main/Clase_02_Chip_headers/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"C:/Users/son_e/STM32CubeIDE/workspace_2.1.1/PATHS/Material_Especialidad_2026-main/Material_Especialidad_2026-main/Clase_02_Chip_headers/Drivers/CMSIS/Include" -I"C:/Users/son_e/STM32CubeIDE/WS_glider/Plantilla_STM32F405RGT6/if_h" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/i2c_driver.cyclo ./Src/i2c_driver.d ./Src/i2c_driver.o ./Src/i2c_driver.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/spi_driver.cyclo ./Src/spi_driver.d ./Src/spi_driver.o ./Src/spi_driver.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su ./Src/system_clock_driver.cyclo ./Src/system_clock_driver.d ./Src/system_clock_driver.o ./Src/system_clock_driver.su ./Src/uart_driver.cyclo ./Src/uart_driver.d ./Src/uart_driver.o ./Src/uart_driver.su

.PHONY: clean-Src

