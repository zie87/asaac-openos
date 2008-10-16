################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../Source/Communication/CommunicationManager.cc \
../Source/Communication/GlobalVc.cc \
../Source/Communication/GlobalVcQueueCallback.cc \
../Source/Communication/LocalVc.cc \
../Source/Communication/OneWayCommunication.cc \
../Source/Communication/TwoWayCommunication.cc \
../Source/Communication/VcUpdateSignal.cc 

OBJS += \
./Source/Communication/CommunicationManager.o \
./Source/Communication/GlobalVc.o \
./Source/Communication/GlobalVcQueueCallback.o \
./Source/Communication/LocalVc.o \
./Source/Communication/OneWayCommunication.o \
./Source/Communication/TwoWayCommunication.o \
./Source/Communication/VcUpdateSignal.o 

CC_DEPS += \
./Source/Communication/CommunicationManager.d \
./Source/Communication/GlobalVc.d \
./Source/Communication/GlobalVcQueueCallback.d \
./Source/Communication/LocalVc.d \
./Source/Communication/OneWayCommunication.d \
./Source/Communication/TwoWayCommunication.d \
./Source/Communication/VcUpdateSignal.d 


# Each subdirectory must supply rules for building sources it contributes
Source/Communication/%.o: ../Source/Communication/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -Ddont_DEBUG -Ddont_DEBUG_APPLICATION -Ddont_DEBUG_OLI -Ddont_DEBUG_BUFFER -Ddont_DEBUG_SHM -DNO_SPORADIC_SERVER -Ddont_DEBUG_SCI -I"/root/workspaces/OpenOs/OpenOSLib/Source/ProcessManagement" -I"/root/workspaces/OpenOs/OpenOSLib/Source" -I"/root/workspaces/OpenOs/OpenOSLib" -I"/root/workspaces/OpenOs/AsaacInterface" -O0 -gstabs+ -g3 -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


