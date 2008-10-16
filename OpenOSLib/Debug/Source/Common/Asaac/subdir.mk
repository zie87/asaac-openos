################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../Source/Common/Asaac/CharacterSequence.cc \
../Source/Common/Asaac/TimeInterval.cc \
../Source/Common/Asaac/TimeStamp.cc 

OBJS += \
./Source/Common/Asaac/CharacterSequence.o \
./Source/Common/Asaac/TimeInterval.o \
./Source/Common/Asaac/TimeStamp.o 

CC_DEPS += \
./Source/Common/Asaac/CharacterSequence.d \
./Source/Common/Asaac/TimeInterval.d \
./Source/Common/Asaac/TimeStamp.d 


# Each subdirectory must supply rules for building sources it contributes
Source/Common/Asaac/%.o: ../Source/Common/Asaac/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -Ddont_DEBUG -Ddont_DEBUG_APPLICATION -Ddont_DEBUG_OLI -Ddont_DEBUG_BUFFER -Ddont_DEBUG_SHM -DNO_SPORADIC_SERVER -Ddont_DEBUG_SCI -I"/root/workspaces/OpenOs/OpenOSLib/Source/ProcessManagement" -I"/root/workspaces/OpenOs/OpenOSLib/Source" -I"/root/workspaces/OpenOs/OpenOSLib" -I"/root/workspaces/OpenOs/AsaacInterface" -O0 -gstabs+ -g3 -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


