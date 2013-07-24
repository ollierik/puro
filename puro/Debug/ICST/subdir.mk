################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../ICST/AudioAnalysis.cpp \
../ICST/AudioFile.cpp \
../ICST/AudioSynth.cpp \
../ICST/BlkDsp.cpp \
../ICST/Chart.cpp \
../ICST/Neuro.cpp \
../ICST/SpecMath.cpp \
../ICST/fftoourad.cpp \
../ICST/fftoouraf.cpp 

OBJS += \
./ICST/AudioAnalysis.o \
./ICST/AudioFile.o \
./ICST/AudioSynth.o \
./ICST/BlkDsp.o \
./ICST/Chart.o \
./ICST/Neuro.o \
./ICST/SpecMath.o \
./ICST/fftoourad.o \
./ICST/fftoouraf.o 

CPP_DEPS += \
./ICST/AudioAnalysis.d \
./ICST/AudioFile.d \
./ICST/AudioSynth.d \
./ICST/BlkDsp.d \
./ICST/Chart.d \
./ICST/Neuro.d \
./ICST/SpecMath.d \
./ICST/fftoourad.d \
./ICST/fftoouraf.d 


# Each subdirectory must supply rules for building sources it contributes
ICST/%.o: ../ICST/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/oek/cpp/Puro/ICST" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


