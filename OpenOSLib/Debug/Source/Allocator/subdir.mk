################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../Source/Allocator/AllocatedArea.cc \
../Source/Allocator/Allocator.cc \
../Source/Allocator/LocalMemory.cc \
../Source/Allocator/SharedMemory.cc 

OBJS += \
./Source/Allocator/AllocatedArea.o \
./Source/Allocator/Allocator.o \
./Source/Allocator/LocalMemory.o \
./Source/Allocator/SharedMemory.o 

CC_DEPS += \
./Source/Allocator/AllocatedArea.d \
./Source/Allocator/Allocator.d \
./Source/Allocator/LocalMemory.d \
./Source/Allocator/SharedMemory.d 


# Each subdirectory must supply rules for building sources it contributes
Source/Allocator/%.o: ../Source/Allocator/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -Ddont_DEBUG -DDISABLE_PROCESS_ACCESS_RIGHTS -Ddont_DEBUG_APPLICATION -Ddont_DEBUG_OLI -Ddont_DEBUG_BUFFER -Ddont_DEBUG_SHM -DNO_SPORADIC_SERVER -Ddont_DEBUG_SCI -I"/home/gam/IDEFIX/OpenOSLib/Source/ProcessManagement" -I"/home/gam/IDEFIX/OpenOSLib/Source" -I"/home/gam/IDEFIX/OpenOSLib" -I"/home/gam/IDEFIX/AsaacInterface" -O0 -gstabs+ -g3 -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


