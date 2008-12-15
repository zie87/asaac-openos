################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../Source/IPC/BlockingScope.cc \
../Source/IPC/Event.cc \
../Source/IPC/EventTable.cc \
../Source/IPC/MessageQueue.cc \
../Source/IPC/Mutex.cc \
../Source/IPC/ProtectedScope.cc \
../Source/IPC/Semaphore.cc \
../Source/IPC/SimpleCommandInterface.cc \
../Source/IPC/SpinLock.cc \
../Source/IPC/Trigger.cc 

OBJS += \
./Source/IPC/BlockingScope.o \
./Source/IPC/Event.o \
./Source/IPC/EventTable.o \
./Source/IPC/MessageQueue.o \
./Source/IPC/Mutex.o \
./Source/IPC/ProtectedScope.o \
./Source/IPC/Semaphore.o \
./Source/IPC/SimpleCommandInterface.o \
./Source/IPC/SpinLock.o \
./Source/IPC/Trigger.o 

CC_DEPS += \
./Source/IPC/BlockingScope.d \
./Source/IPC/Event.d \
./Source/IPC/EventTable.d \
./Source/IPC/MessageQueue.d \
./Source/IPC/Mutex.d \
./Source/IPC/ProtectedScope.d \
./Source/IPC/Semaphore.d \
./Source/IPC/SimpleCommandInterface.d \
./Source/IPC/SpinLock.d \
./Source/IPC/Trigger.d 


# Each subdirectory must supply rules for building sources it contributes
Source/IPC/%.o: ../Source/IPC/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -Ddont_DEBUG -DDISABLE_PROCESS_ACCESS_RIGHTS -Ddont_DEBUG_APPLICATION -Ddont_DEBUG_OLI -Ddont_DEBUG_BUFFER -Ddont_DEBUG_SHM -DNO_SPORADIC_SERVER -Ddont_DEBUG_SCI -I"/home/gam/IDEFIX/OpenOSLib/Source/ProcessManagement" -I"/home/gam/IDEFIX/OpenOSLib/Source" -I"/home/gam/IDEFIX/OpenOSLib" -I"/home/gam/IDEFIX/AsaacInterface" -O0 -gstabs+ -g3 -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


