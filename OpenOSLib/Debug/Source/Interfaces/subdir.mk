################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../Source/Interfaces/APOS-Communication.cc \
../Source/Interfaces/APOS-Debugging.cc \
../Source/Interfaces/APOS-ErrorHandling.cc \
../Source/Interfaces/APOS-FileHandling.cc \
../Source/Interfaces/APOS-PowerConversion.cc \
../Source/Interfaces/APOS-Synchronization.cc \
../Source/Interfaces/APOS-ThreadManagement.cc \
../Source/Interfaces/APOS-TimeManagement.cc \
../Source/Interfaces/SMOS-BuiltInTestManagement.cc \
../Source/Interfaces/SMOS-CFMInformation.cc \
../Source/Interfaces/SMOS-CFMRecourcesManagement.cc \
../Source/Interfaces/SMOS-FaultManagement.cc \
../Source/Interfaces/SMOS-LoggingManagement.cc \
../Source/Interfaces/SMOS-NetworkConfiguration.cc \
../Source/Interfaces/SMOS-ProcessAndThreadManagement.cc \
../Source/Interfaces/SMOS-SecurityManagement.cc \
../Source/Interfaces/SMOS-TimeManagement.cc \
../Source/Interfaces/SMOS-VcConfiguration.cc 

OBJS += \
./Source/Interfaces/APOS-Communication.o \
./Source/Interfaces/APOS-Debugging.o \
./Source/Interfaces/APOS-ErrorHandling.o \
./Source/Interfaces/APOS-FileHandling.o \
./Source/Interfaces/APOS-PowerConversion.o \
./Source/Interfaces/APOS-Synchronization.o \
./Source/Interfaces/APOS-ThreadManagement.o \
./Source/Interfaces/APOS-TimeManagement.o \
./Source/Interfaces/SMOS-BuiltInTestManagement.o \
./Source/Interfaces/SMOS-CFMInformation.o \
./Source/Interfaces/SMOS-CFMRecourcesManagement.o \
./Source/Interfaces/SMOS-FaultManagement.o \
./Source/Interfaces/SMOS-LoggingManagement.o \
./Source/Interfaces/SMOS-NetworkConfiguration.o \
./Source/Interfaces/SMOS-ProcessAndThreadManagement.o \
./Source/Interfaces/SMOS-SecurityManagement.o \
./Source/Interfaces/SMOS-TimeManagement.o \
./Source/Interfaces/SMOS-VcConfiguration.o 

CC_DEPS += \
./Source/Interfaces/APOS-Communication.d \
./Source/Interfaces/APOS-Debugging.d \
./Source/Interfaces/APOS-ErrorHandling.d \
./Source/Interfaces/APOS-FileHandling.d \
./Source/Interfaces/APOS-PowerConversion.d \
./Source/Interfaces/APOS-Synchronization.d \
./Source/Interfaces/APOS-ThreadManagement.d \
./Source/Interfaces/APOS-TimeManagement.d \
./Source/Interfaces/SMOS-BuiltInTestManagement.d \
./Source/Interfaces/SMOS-CFMInformation.d \
./Source/Interfaces/SMOS-CFMRecourcesManagement.d \
./Source/Interfaces/SMOS-FaultManagement.d \
./Source/Interfaces/SMOS-LoggingManagement.d \
./Source/Interfaces/SMOS-NetworkConfiguration.d \
./Source/Interfaces/SMOS-ProcessAndThreadManagement.d \
./Source/Interfaces/SMOS-SecurityManagement.d \
./Source/Interfaces/SMOS-TimeManagement.d \
./Source/Interfaces/SMOS-VcConfiguration.d 


# Each subdirectory must supply rules for building sources it contributes
Source/Interfaces/%.o: ../Source/Interfaces/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -Ddont_DEBUG -DDISABLE_PROCESS_ACCESS_RIGHTS -Ddont_DEBUG_APPLICATION -Ddont_DEBUG_OLI -Ddont_DEBUG_BUFFER -Ddont_DEBUG_SHM -DNO_SPORADIC_SERVER -Ddont_DEBUG_SCI -I"/home/gam/IDEFIX/OpenOSLib/Source/ProcessManagement" -I"/home/gam/IDEFIX/OpenOSLib/Source" -I"/home/gam/IDEFIX/OpenOSLib" -I"/home/gam/IDEFIX/AsaacInterface" -O0 -gstabs+ -g3 -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


