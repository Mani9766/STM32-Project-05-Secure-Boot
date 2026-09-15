################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Modules/Crypto/Src/sha256.c 

OBJS += \
./Modules/Crypto/Src/sha256.o 

C_DEPS += \
./Modules/Crypto/Src/sha256.d 


# Each subdirectory must supply rules for building sources it contributes
Modules/Crypto/Src/%.o Modules/Crypto/Src/%.su Modules/Crypto/Src/%.cyclo: ../Modules/Crypto/Src/%.c Modules/Crypto/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I"D:/Manisha Daigavhane/software/workspace/Github_Repositories/STM32-Project-05-Secure-Boot/Firmware/Modules/Crypto/Inc" -I"D:/Manisha Daigavhane/software/workspace/Github_Repositories/STM32-Project-05-Secure-Boot/Firmware/Modules/Flash/Inc" -I"D:/Manisha Daigavhane/software/workspace/Github_Repositories/STM32-Project-05-Secure-Boot/Firmware/Modules/Metadata/Inc" -I../Core/Inc -I"D:/Manisha Daigavhane/software/workspace/Github_Repositories/STM32-Project-05-Secure-Boot/Firmware/Tools" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Modules-2f-Crypto-2f-Src

clean-Modules-2f-Crypto-2f-Src:
	-$(RM) ./Modules/Crypto/Src/sha256.cyclo ./Modules/Crypto/Src/sha256.d ./Modules/Crypto/Src/sha256.o ./Modules/Crypto/Src/sha256.su

.PHONY: clean-Modules-2f-Crypto-2f-Src

