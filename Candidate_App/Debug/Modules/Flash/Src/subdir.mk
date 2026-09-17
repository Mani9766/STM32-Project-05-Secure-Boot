################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Modules/Flash/Src/flash_storage.c 

OBJS += \
./Modules/Flash/Src/flash_storage.o 

C_DEPS += \
./Modules/Flash/Src/flash_storage.d 


# Each subdirectory must supply rules for building sources it contributes
Modules/Flash/Src/%.o Modules/Flash/Src/%.su Modules/Flash/Src/%.cyclo: ../Modules/Flash/Src/%.c Modules/Flash/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"D:/Manisha Daigavhane/software/workspace/Github_Repositories/STM32-Project-05-Secure-Boot/Candidate_App/Modules/Metadata/Inc" -I"D:/Manisha Daigavhane/software/workspace/Github_Repositories/STM32-Project-05-Secure-Boot/Candidate_App/Modules/Crypto/Inc" -I"D:/Manisha Daigavhane/software/workspace/Github_Repositories/STM32-Project-05-Secure-Boot/Candidate_App/Modules/Flash/Inc" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Modules-2f-Flash-2f-Src

clean-Modules-2f-Flash-2f-Src:
	-$(RM) ./Modules/Flash/Src/flash_storage.cyclo ./Modules/Flash/Src/flash_storage.d ./Modules/Flash/Src/flash_storage.o ./Modules/Flash/Src/flash_storage.su

.PHONY: clean-Modules-2f-Flash-2f-Src

