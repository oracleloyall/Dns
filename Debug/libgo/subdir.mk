################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../libgo/block_object.cpp \
../libgo/co_mutex.cpp \
../libgo/co_rwmutex.cpp \
../libgo/config.cpp \
../libgo/debugger.cpp \
../libgo/error.cpp \
../libgo/processer.cpp \
../libgo/scheduler.cpp \
../libgo/sleep_wait.cpp \
../libgo/task.cpp \
../libgo/thread_pool.cpp \
../libgo/timer.cpp 

OBJS += \
./libgo/block_object.o \
./libgo/co_mutex.o \
./libgo/co_rwmutex.o \
./libgo/config.o \
./libgo/debugger.o \
./libgo/error.o \
./libgo/processer.o \
./libgo/scheduler.o \
./libgo/sleep_wait.o \
./libgo/task.o \
./libgo/thread_pool.o \
./libgo/timer.o 

CPP_DEPS += \
./libgo/block_object.d \
./libgo/co_mutex.d \
./libgo/co_rwmutex.d \
./libgo/config.d \
./libgo/debugger.d \
./libgo/error.d \
./libgo/processer.d \
./libgo/scheduler.d \
./libgo/sleep_wait.d \
./libgo/task.d \
./libgo/thread_pool.d \
./libgo/timer.d 


# Each subdirectory must supply rules for building sources it contributes
libgo/%.o: ../libgo/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -std=c++11 -c -fmessage-length=0  -fPIC -Wall   -Werror   -DCO_DYNAMIC_LINK  -ldl -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


