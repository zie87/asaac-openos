################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source/Interfaces/MOS-BIT.c \
../Source/Interfaces/MOS-CFM.c \
../Source/Interfaces/MOS-Callback.c \
../Source/Interfaces/MOS-Communication.c \
../Source/Interfaces/MOS-Device.c \
../Source/Interfaces/MOS-Specific.c \
../Source/Interfaces/MOS-Timer.c 

OBJS += \
./Source/Interfaces/MOS-BIT.o \
./Source/Interfaces/MOS-CFM.o \
./Source/Interfaces/MOS-Callback.o \
./Source/Interfaces/MOS-Communication.o \
./Source/Interfaces/MOS-Device.o \
./Source/Interfaces/MOS-Specific.o \
./Source/Interfaces/MOS-Timer.o 

C_DEPS += \
./Source/Interfaces/MOS-BIT.d \
./Source/Interfaces/MOS-CFM.d \
./Source/Interfaces/MOS-Callback.d \
./Source/Interfaces/MOS-Communication.d \
./Source/Interfaces/MOS-Device.d \
./Source/Interfaces/MOS-Specific.d \
./Source/Interfaces/MOS-Timer.d 


# Each subdirectory must supply rules for building sources it contributes
Source/Interfaces/%.o: ../Source/Interfaces/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/gam/IDEFIX/AsaacInterface" -I"/home/gam/IDEFIX/AsaacCLib" -I"/home/gam/IDEFIX/NIILib/Source" -I"/home/gam/IDEFIX/MOSLib/Source" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


