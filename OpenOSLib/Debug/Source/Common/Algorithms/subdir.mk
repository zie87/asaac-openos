################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source/Common/Algorithms/CRC.c 

CC_SRCS += \
../Source/Common/Algorithms/DiffieHellman.cc \
../Source/Common/Algorithms/Primes.cc 

OBJS += \
./Source/Common/Algorithms/CRC.o \
./Source/Common/Algorithms/DiffieHellman.o \
./Source/Common/Algorithms/Primes.o 

CC_DEPS += \
./Source/Common/Algorithms/DiffieHellman.d \
./Source/Common/Algorithms/Primes.d 

C_DEPS += \
./Source/Common/Algorithms/CRC.d 


# Each subdirectory must supply rules for building sources it contributes
Source/Common/Algorithms/%.o: ../Source/Common/Algorithms/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Source/Common/Algorithms/%.o: ../Source/Common/Algorithms/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -Ddont_DEBUG -DDISABLE_PROCESS_ACCESS_RIGHTS -Ddont_DEBUG_APPLICATION -Ddont_DEBUG_OLI -Ddont_DEBUG_BUFFER -Ddont_DEBUG_SHM -DNO_SPORADIC_SERVER -Ddont_DEBUG_SCI -I"/home/gam/IDEFIX/OpenOSLib/Source/ProcessManagement" -I"/home/gam/IDEFIX/OpenOSLib/Source" -I"/home/gam/IDEFIX/OpenOSLib" -I"/home/gam/IDEFIX/AsaacInterface" -O0 -gstabs+ -g3 -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


