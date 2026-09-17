################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Modules/Metadata/Src/metadata.c 

OBJS += \
./Modules/Metadata/Src/metadata.o 

C_DEPS += \
./Modules/Metadata/Src/metadata.d 


# Each subdirectory must supply rules for building sources it contributes
Modules/Metadata/Src/%.o Modules/Metadata/Src/%.su Modules/Metadata/Src/%.cyclo: ../Modules/Metadata/Src/%.c Modules/Metadata/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"D:/Manisha Daigavhane/software/workspace/Github_Repositories/STM32-Project-05-Secure-Boot/Candidate_App/Modules/Metadata/Inc" -I"D:/Manisha Daigavhane/software/workspace/Github_Repositories/STM32-Project-05-Secure-Boot/Candidate_App/Modules/Crypto/Inc" -I"D:/Manisha Daigavhane/software/workspace/Github_Repositories/STM32-Project-05-Secure-Boot/Candidate_App/Modules/Flash/Inc" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Modules-2f-Metadata-2f-Src

clean-Modules-2f-Metadata-2f-Src:
	-$(RM) ./Modules/Metadata/Src/metadata.cyclo ./Modules/Metadata/Src/metadata.d ./Modules/Metadata/Src/metadata.o ./Modules/Metadata/Src/metadata.su

.PHONY: clean-Modules-2f-Metadata-2f-Src

