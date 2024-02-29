################################################################################
# MRS Version: {"version":"1.8.5","date":"2023/05/22"}
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/ch32v20x_it.c \
../User/gamepad.c \
../User/gpio.c \
../User/keyboard.c \
../User/main.c \
../User/mouse.c \
../User/system_ch32v20x.c \
../User/tim.c \
../User/utils.c 

OBJS += \
./User/ch32v20x_it.o \
./User/gamepad.o \
./User/gpio.o \
./User/keyboard.o \
./User/main.o \
./User/mouse.o \
./User/system_ch32v20x.o \
./User/tim.o \
./User/utils.o 

C_DEPS += \
./User/ch32v20x_it.d \
./User/gamepad.d \
./User/gpio.d \
./User/keyboard.d \
./User/main.d \
./User/mouse.d \
./User/system_ch32v20x.d \
./User/tim.d \
./User/utils.d 


# Each subdirectory must supply rules for building sources it contributes
User/%.o: ../User/%.c
	@	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"C:\Repo\A1200HID2DB9\A1200HIDToDB9WithKeyboard\Debug" -I"C:\Repo\A1200HID2DB9\A1200HIDToDB9WithKeyboard\User\USB_Host" -I"C:\Repo\A1200HID2DB9\A1200HIDToDB9WithKeyboard\Core" -I"C:\Repo\A1200HID2DB9\A1200HIDToDB9WithKeyboard\User" -I"C:\Repo\A1200HID2DB9\A1200HIDToDB9WithKeyboard\Peripheral\inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

