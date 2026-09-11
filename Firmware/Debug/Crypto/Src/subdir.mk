################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Crypto/Src/sha256.c 

OBJS += \
./Crypto/Src/sha256.o 

C_DEPS += \
./Crypto/Src/sha256.d 


# Each subdirectory must supply rules for building sources it contributes
Crypto/Src/%.o Crypto/Src/%.su Crypto/Src/%.cyclo: ../Crypto/Src/%.c Crypto/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"D:/Manisha Daigavhane/software/workspace/Github_Repositories/STM32-Project-05-Secure-Boot/Firmware/Crypto/Inc" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Crypto-2f-Src

clean-Crypto-2f-Src:
	-$(RM) ./Crypto/Src/sha256.cyclo ./Crypto/Src/sha256.d ./Crypto/Src/sha256.o ./Crypto/Src/sha256.su

.PHONY: clean-Crypto-2f-Src

