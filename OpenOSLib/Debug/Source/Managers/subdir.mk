################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../Source/Managers/AllocatorManager.cc \
../Source/Managers/EventManager.cc \
../Source/Managers/FileManager.cc \
../Source/Managers/FileNameGenerator.cc \
../Source/Managers/PrivateIdManager.cc \
../Source/Managers/SemaphoreManager.cc \
../Source/Managers/SignalManager.cc \
../Source/Managers/TimeManager.cc 

OBJS += \
./Source/Managers/AllocatorManager.o \
./Source/Managers/EventManager.o \
./Source/Managers/FileManager.o \
./Source/Managers/FileNameGenerator.o \
./Source/Managers/PrivateIdManager.o \
./Source/Managers/SemaphoreManager.o \
./Source/Managers/SignalManager.o \
./Source/Managers/TimeManager.o 

CC_DEPS += \
./Source/Managers/AllocatorManager.d \
./Source/Managers/EventManager.d \
./Source/Managers/FileManager.d \
./Source/Managers/FileNameGenerator.d \
./Source/Managers/PrivateIdManager.d \
./Source/Managers/SemaphoreManager.d \
./Source/Managers/SignalManager.d \
./Source/Managers/TimeManager.d 


# Each subdirectory must supply rules for building sources it contributes
Source/Managers/%.o: ../Source/Managers/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -Ddont_DEBUG -DDISABLE_PROCESS_ACCESS_RIGHTS -Ddont_DEBUG_APPLICATION -Ddont_DEBUG_OLI -Ddont_DEBUG_BUFFER -Ddont_DEBUG_SHM -DNO_SPORADIC_SERVER -Ddont_DEBUG_SCI -I"/home/gam/IDEFIX/OpenOSLib/Source/ProcessManagement" -I"/home/gam/IDEFIX/OpenOSLib/Source" -I"/home/gam/IDEFIX/OpenOSLib" -I"/home/gam/IDEFIX/AsaacInterface" -O0 -gstabs+ -g3 -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


