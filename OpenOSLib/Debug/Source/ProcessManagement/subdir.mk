################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../Source/ProcessManagement/Process.cc \
../Source/ProcessManagement/ProcessManager.cc \
../Source/ProcessManagement/Thread.cc \
../Source/ProcessManagement/ThreadManager.cc 

OBJS += \
./Source/ProcessManagement/Process.o \
./Source/ProcessManagement/ProcessManager.o \
./Source/ProcessManagement/Thread.o \
./Source/ProcessManagement/ThreadManager.o 

CC_DEPS += \
./Source/ProcessManagement/Process.d \
./Source/ProcessManagement/ProcessManager.d \
./Source/ProcessManagement/Thread.d \
./Source/ProcessManagement/ThreadManager.d 


# Each subdirectory must supply rules for building sources it contributes
Source/ProcessManagement/%.o: ../Source/ProcessManagement/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -Ddont_DEBUG -DDISABLE_PROCESS_ACCESS_RIGHTS -Ddont_DEBUG_APPLICATION -Ddont_DEBUG_OLI -Ddont_DEBUG_BUFFER -Ddont_DEBUG_SHM -DNO_SPORADIC_SERVER -Ddont_DEBUG_SCI -I"/home/gam/IDEFIX/OpenOSLib/Source/ProcessManagement" -I"/home/gam/IDEFIX/OpenOSLib/Source" -I"/home/gam/IDEFIX/OpenOSLib" -I"/home/gam/IDEFIX/AsaacInterface" -O0 -gstabs+ -g3 -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


