################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../main.cc 

OBJS += \
./main.o 

CC_DEPS += \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/root/workspaces/OpenOs/OpenOSLib/Source" -I"/root/workspaces/OpenOs/OpenOSLib" -I"/root/workspaces/OpenOs/AsaacInterface" -O0 -g3 -gstabs+ -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


