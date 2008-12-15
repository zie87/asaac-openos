################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include Source/Wrapper/subdir.mk
-include Source/ProcessManagement/subdir.mk
-include Source/subdir.mk
-include Source/OAL/subdir.mk
-include Source/Managers/subdir.mk
-include Source/LogicalInterfaces/PCS/subdir.mk
-include Source/LogicalInterfaces/OLI/subdir.mk
-include Source/LogicalInterfaces/GLI/subdir.mk
-include Source/Interfaces/subdir.mk
-include Source/IPC/subdir.mk
-include Source/FaultManagement/subdir.mk
-include Source/Exceptions/subdir.mk
-include Source/Debug/subdir.mk
-include Source/Communication/subdir.mk
-include Source/Common/Parameter/subdir.mk
-include Source/Common/Asaac/subdir.mk
-include Source/Common/Algorithms/subdir.mk
-include Source/Common/subdir.mk
-include Source/Allocator/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: libOpenOS.so

# Tool invocations
libOpenOS.so: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++ -L"/home/gam/IDEFIX/MOSLib/Debug" -shared -o"libOpenOS.so" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C++_DEPS)$(CC_DEPS)$(C_DEPS)$(CPP_DEPS)$(LIBRARIES)$(CXX_DEPS)$(C_UPPER_DEPS) libOpenOS.so
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
