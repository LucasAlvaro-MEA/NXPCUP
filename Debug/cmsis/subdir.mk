################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cmsis/system_MKL25Z4.c 

OBJS += \
./cmsis/system_MKL25Z4.o 

C_DEPS += \
./cmsis/system_MKL25Z4.d 


# Each subdirectory must supply rules for building sources it contributes
cmsis/%.o: ../cmsis/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_MKL25Z128VLK4 -DCPU_MKL25Z128VLK4_cm0plus -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -D__REDLIB__ -I../app -I../bsp -I../ -I../startup -I../cmsis -I../FreeRTOS -I../FreeRTOS/include -I../FreeRTOS/portable/GCC/ARM_CM0 -I../TraceRecorder -I../TraceRecorder/config -I../TraceRecorder/include -I../TraceRecorder/streamports -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


