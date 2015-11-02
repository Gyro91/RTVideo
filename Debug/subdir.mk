################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../Draw.o \
../Periodicity.o \
../RTVideo.o 

C_SRCS += \
../Draw.c \
../Periodicity.c \
../RTVideo.c 

OBJS += \
./Draw.o \
./Periodicity.o \
./RTVideo.o 

C_DEPS += \
./Draw.d \
./Periodicity.d \
./RTVideo.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


