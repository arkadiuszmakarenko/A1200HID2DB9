################################################################################
# MRS Version: {"version":"1.8.5","date":"2023/05/22"}
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/USB_Host/app_km.c \
../User/USB_Host/ch32v20x_usbfs_host.c \
../User/USB_Host/usb_gamepad.c \
../User/USB_Host/usb_hid_reportparser.c \
../User/USB_Host/usb_host_hid.c \
../User/USB_Host/usb_host_hub.c \
../User/USB_Host/usb_keyboard.c \
../User/USB_Host/usb_mouse.c 

OBJS += \
./User/USB_Host/app_km.o \
./User/USB_Host/ch32v20x_usbfs_host.o \
./User/USB_Host/usb_gamepad.o \
./User/USB_Host/usb_hid_reportparser.o \
./User/USB_Host/usb_host_hid.o \
./User/USB_Host/usb_host_hub.o \
./User/USB_Host/usb_keyboard.o \
./User/USB_Host/usb_mouse.o 

C_DEPS += \
./User/USB_Host/app_km.d \
./User/USB_Host/ch32v20x_usbfs_host.d \
./User/USB_Host/usb_gamepad.d \
./User/USB_Host/usb_hid_reportparser.d \
./User/USB_Host/usb_host_hid.d \
./User/USB_Host/usb_host_hub.d \
./User/USB_Host/usb_keyboard.d \
./User/USB_Host/usb_mouse.d 


# Each subdirectory must supply rules for building sources it contributes
User/USB_Host/%.o: ../User/USB_Host/%.c
	@	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"C:\Repo\A1200HID2DB9\A1200HIDToDB9WithKeyboard\Debug" -I"C:\Repo\A1200HID2DB9\A1200HIDToDB9WithKeyboard\User\USB_Host" -I"C:\Repo\A1200HID2DB9\A1200HIDToDB9WithKeyboard\Core" -I"C:\Repo\A1200HID2DB9\A1200HIDToDB9WithKeyboard\User" -I"C:\Repo\A1200HID2DB9\A1200HIDToDB9WithKeyboard\Peripheral\inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

