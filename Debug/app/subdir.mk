################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../app/accl.c \
../app/cam.c \
../app/ctrl.c \
../app/ctrl3.c \
../app/delay.c \
../app/display.c \
../app/exposition.c \
../app/main.c \
../app/printf-stdarg.c 

OBJS += \
./app/accl.o \
./app/cam.o \
./app/ctrl.o \
./app/ctrl3.o \
./app/delay.o \
./app/display.o \
./app/exposition.o \
./app/main.o \
./app/printf-stdarg.o 

C_DEPS += \
./app/accl.d \
./app/cam.d \
./app/ctrl.d \
./app/ctrl3.d \
./app/delay.d \
./app/display.d \
./app/exposition.d \
./app/main.d \
./app/printf-stdarg.d 


# Each subdirectory must supply rules for building sources it contributes
app/%.o: ../app/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_MKL25Z128VLK4 -DCPU_MKL25Z128VLK4_cm0plus -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -D__REDLIB__ -I../app -I../bsp -I../ -I../startup -I../cmsis -I../FreeRTOS -I../FreeRTOS/include -I../FreeRTOS/portable/GCC/ARM_CM0 -I../TraceRecorder -I../TraceRecorder/config -I../TraceRecorder/include -I../TraceRecorder/streamports -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


