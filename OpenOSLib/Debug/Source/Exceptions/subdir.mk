################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../Source/Exceptions/ASAAC_Exception.cc 

OBJS += \
./Source/Exceptions/ASAAC_Exception.o 

CC_DEPS += \
./Source/Exceptions/ASAAC_Exception.d 


# Each subdirectory must supply rules for building sources it contributes
Source/Exceptions/%.o: ../Source/Exceptions/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -Ddont_DEBUG -Ddont_DEBUG_APPLICATION -Ddont_DEBUG_OLI -Ddont_DEBUG_BUFFER -Ddont_DEBUG_SHM -DNO_SPORADIC_SERVER -Ddont_DEBUG_SCI -I"/root/workspaces/OpenOs/OpenOSLib/Source/ProcessManagement" -I"/root/workspaces/OpenOs/OpenOSLib/Source" -I"/root/workspaces/OpenOs/OpenOSLib" -I"/root/workspaces/OpenOs/AsaacInterface" -O0 -gstabs+ -g3 -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


