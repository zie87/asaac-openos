################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source/OAL/oal.c 

OBJS += \
./Source/OAL/oal.o 

C_DEPS += \
./Source/OAL/oal.d 


# Each subdirectory must supply rules for building sources it contributes
Source/OAL/%.o: ../Source/OAL/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


