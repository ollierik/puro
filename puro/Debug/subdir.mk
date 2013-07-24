################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../AudioStorage.cpp \
../Buffer.cpp \
../Drop.cpp \
../Engine.cpp \
../Idea.cpp \
../Interpreter.cpp \
../MainFrame.cpp \
../Passage.cpp \
../Puro.cpp \
../Worker.cpp \
../maintest.cpp 

OBJS += \
./AudioStorage.o \
./Buffer.o \
./Drop.o \
./Engine.o \
./Idea.o \
./Interpreter.o \
./MainFrame.o \
./Passage.o \
./Puro.o \
./Worker.o \
./maintest.o 

CPP_DEPS += \
./AudioStorage.d \
./Buffer.d \
./Drop.d \
./Engine.d \
./Idea.d \
./Interpreter.d \
./MainFrame.d \
./Passage.d \
./Puro.d \
./Worker.d \
./maintest.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/oek/cpp/Puro/ICST" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


