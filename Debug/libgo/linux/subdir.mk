################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../libgo/linux/fd_context.cpp \
../libgo/linux/hook_signal.cpp \
../libgo/linux/io_wait.cpp \
../libgo/linux/linux_glibc_hook.cpp 

OBJS += \
./libgo/linux/fd_context.o \
./libgo/linux/hook_signal.o \
./libgo/linux/io_wait.o \
./libgo/linux/linux_glibc_hook.o 

CPP_DEPS += \
./libgo/linux/fd_context.d \
./libgo/linux/hook_signal.d \
./libgo/linux/io_wait.d \
./libgo/linux/linux_glibc_hook.d 


# Each subdirectory must supply rules for building sources it contributes
libgo/linux/%.o: ../libgo/linux/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -std=c++11 -c -fmessage-length=0  -fPIC -Wall   -Werror   -DCO_DYNAMIC_LINK  -ldl -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


