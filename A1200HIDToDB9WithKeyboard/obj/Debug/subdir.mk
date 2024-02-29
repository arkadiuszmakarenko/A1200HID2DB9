################################################################################
# MRS Version: {"version":"1.8.5","date":"2023/05/22"}
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Debug/debug.c 

OBJS += \
./Debug/debug.o 

C_DEPS += \
./Debug/debug.d 


# Each subdirectory must supply rules for building sources it contributes
Debug/%.o: ../Debug/%.c
	@	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"C:\Repo\A1200HID2DB9\A1200HIDToDB9WithKeyboard\Debug" -I"C:\Repo\A1200HID2DB9\A1200HIDToDB9WithKeyboard\User\USB_Host" -I"C:\Repo\A1200HID2DB9\A1200HIDToDB9WithKeyboard\Core" -I"C:\Repo\A1200HID2DB9\A1200HIDToDB9WithKeyboard\User" -I"C:\Repo\A1200HID2DB9\A1200HIDToDB9WithKeyboard\Peripheral\inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

