################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../Source/FaultManagement/ErrorHandler.cc \
../Source/FaultManagement/FaultManager.cc \
../Source/FaultManagement/LoggingManager.cc 

OBJS += \
./Source/FaultManagement/ErrorHandler.o \
./Source/FaultManagement/FaultManager.o \
./Source/FaultManagement/LoggingManager.o 

CC_DEPS += \
./Source/FaultManagement/ErrorHandler.d \
./Source/FaultManagement/FaultManager.d \
./Source/FaultManagement/LoggingManager.d 


# Each subdirectory must supply rules for building sources it contributes
Source/FaultManagement/%.o: ../Source/FaultManagement/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -Ddont_DEBUG -Ddont_DEBUG_APPLICATION -Ddont_DEBUG_OLI -Ddont_DEBUG_BUFFER -Ddont_DEBUG_SHM -DNO_SPORADIC_SERVER -Ddont_DEBUG_SCI -I"/root/workspaces/OpenOs/OpenOSLib/Source/ProcessManagement" -I"/root/workspaces/OpenOs/OpenOSLib/Source" -I"/root/workspaces/OpenOs/OpenOSLib" -I"/root/workspaces/OpenOs/AsaacInterface" -O0 -gstabs+ -g3 -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


