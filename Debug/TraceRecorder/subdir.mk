################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../TraceRecorder/trcKernelPort.c \
../TraceRecorder/trcSnapshotRecorder.c \
../TraceRecorder/trcStreamingRecorder.c 

OBJS += \
./TraceRecorder/trcKernelPort.o \
./TraceRecorder/trcSnapshotRecorder.o \
./TraceRecorder/trcStreamingRecorder.o 

C_DEPS += \
./TraceRecorder/trcKernelPort.d \
./TraceRecorder/trcSnapshotRecorder.d \
./TraceRecorder/trcStreamingRecorder.d 


# Each subdirectory must supply rules for building sources it contributes
TraceRecorder/%.o: ../TraceRecorder/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_MKL25Z128VLK4 -DCPU_MKL25Z128VLK4_cm0plus -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -D__REDLIB__ -I../app -I../bsp -I../ -I../startup -I../cmsis -I../FreeRTOS -I../FreeRTOS/include -I../FreeRTOS/portable/GCC/ARM_CM0 -I../TraceRecorder -I../TraceRecorder/config -I../TraceRecorder/include -I../TraceRecorder/streamports -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


