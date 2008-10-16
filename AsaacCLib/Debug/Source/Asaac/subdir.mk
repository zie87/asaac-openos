################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source/Asaac/AsaacTime.c \
../Source/Asaac/AsaacTimeInterval.c 

OBJS += \
./Source/Asaac/AsaacTime.o \
./Source/Asaac/AsaacTimeInterval.o 

C_DEPS += \
./Source/Asaac/AsaacTime.d \
./Source/Asaac/AsaacTimeInterval.d 


# Each subdirectory must supply rules for building sources it contributes
Source/Asaac/%.o: ../Source/Asaac/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/root/workspaces/OpenOs/AsaacInterface" -I"/root/workspaces/OpenOs/AsaacCLib" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


