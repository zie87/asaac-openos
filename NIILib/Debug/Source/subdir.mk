################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source/NII.c 

OBJS += \
./Source/NII.o 

C_DEPS += \
./Source/NII.d 


# Each subdirectory must supply rules for building sources it contributes
Source/%.o: ../Source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -Ddont_DEBUG_ -I"/home/gam/IDEFIX/AsaacInterface" -I"/home/gam/IDEFIX/AsaacCLib" -I"/home/gam/IDEFIX/NIILib/Source" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


