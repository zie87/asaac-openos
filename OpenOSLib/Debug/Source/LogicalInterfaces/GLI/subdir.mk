################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../Source/LogicalInterfaces/GLI/GLINode.cc \
../Source/LogicalInterfaces/GLI/GLISubNode.cc \
../Source/LogicalInterfaces/GLI/GLISuperNode.cc 

OBJS += \
./Source/LogicalInterfaces/GLI/GLINode.o \
./Source/LogicalInterfaces/GLI/GLISubNode.o \
./Source/LogicalInterfaces/GLI/GLISuperNode.o 

CC_DEPS += \
./Source/LogicalInterfaces/GLI/GLINode.d \
./Source/LogicalInterfaces/GLI/GLISubNode.d \
./Source/LogicalInterfaces/GLI/GLISuperNode.d 


# Each subdirectory must supply rules for building sources it contributes
Source/LogicalInterfaces/GLI/%.o: ../Source/LogicalInterfaces/GLI/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -Ddont_DEBUG -DDISABLE_PROCESS_ACCESS_RIGHTS -Ddont_DEBUG_APPLICATION -Ddont_DEBUG_OLI -Ddont_DEBUG_BUFFER -Ddont_DEBUG_SHM -DNO_SPORADIC_SERVER -Ddont_DEBUG_SCI -I"/home/gam/IDEFIX/OpenOSLib/Source/ProcessManagement" -I"/home/gam/IDEFIX/OpenOSLib/Source" -I"/home/gam/IDEFIX/OpenOSLib" -I"/home/gam/IDEFIX/AsaacInterface" -O0 -gstabs+ -g3 -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


