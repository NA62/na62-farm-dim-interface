################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/FarmStarter.cpp \
../src/MessageQueueConnector.cpp \
../src/MonitorDimServer.cpp \
../src/MyCommandHandler.cpp \
../src/na62-farm-dim-interface.cpp 

OBJS += \
./src/FarmStarter.o \
./src/MessageQueueConnector.o \
./src/MonitorDimServer.o \
./src/MyCommandHandler.o \
./src/na62-farm-dim-interface.o 

CPP_DEPS += \
./src/FarmStarter.d \
./src/MessageQueueConnector.d \
./src/MonitorDimServer.d \
./src/MyCommandHandler.d \
./src/na62-farm-dim-interface.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


