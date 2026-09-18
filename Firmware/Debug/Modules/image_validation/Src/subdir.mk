################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Modules/image_validation/Src/image_validation.c 

OBJS += \
./Modules/image_validation/Src/image_validation.o 

C_DEPS += \
./Modules/image_validation/Src/image_validation.d 


# Each subdirectory must supply rules for building sources it contributes
Modules/image_validation/Src/%.o Modules/image_validation/Src/%.su Modules/image_validation/Src/%.cyclo: ../Modules/image_validation/Src/%.c Modules/image_validation/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I"D:/Manisha Daigavhane/software/workspace/Github_Repositories/STM32-Project-05-Secure-Boot/Firmware/Modules/Crypto/Inc" -I"D:/Manisha Daigavhane/software/workspace/Github_Repositories/STM32-Project-05-Secure-Boot/Firmware/Modules/image_validation/Inc" -I"D:/Manisha Daigavhane/software/workspace/Github_Repositories/STM32-Project-05-Secure-Boot/Firmware/Modules/Flash/Inc" -I"D:/Manisha Daigavhane/software/workspace/Github_Repositories/STM32-Project-05-Secure-Boot/Firmware/Modules/Metadata/Inc" -I../Core/Inc -I"D:/Manisha Daigavhane/software/workspace/Github_Repositories/STM32-Project-05-Secure-Boot/Firmware/Tools" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Modules-2f-image_validation-2f-Src

clean-Modules-2f-image_validation-2f-Src:
	-$(RM) ./Modules/image_validation/Src/image_validation.cyclo ./Modules/image_validation/Src/image_validation.d ./Modules/image_validation/Src/image_validation.o ./Modules/image_validation/Src/image_validation.su

.PHONY: clean-Modules-2f-image_validation-2f-Src

