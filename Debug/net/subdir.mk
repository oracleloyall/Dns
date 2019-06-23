################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../net/abstract.cpp \
../net/error.cpp \
../net/network.cpp \
../net/tcp.cpp \
../net/tcp_detail.cpp \
../net/udp.cpp \
../net/udp_detail.cpp 

OBJS += \
./net/abstract.o \
./net/error.o \
./net/network.o \
./net/tcp.o \
./net/tcp_detail.o \
./net/udp.o \
./net/udp_detail.o 

CPP_DEPS += \
./net/abstract.d \
./net/error.d \
./net/network.d \
./net/tcp.d \
./net/tcp_detail.d \
./net/udp.d \
./net/udp_detail.d 


# Each subdirectory must supply rules for building sources it contributes
net/%.o: ../net/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -std=c++11 -c -fmessage-length=0  -fPIC -Wall   -Werror   -DCO_DYNAMIC_LINK  -ldl -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


