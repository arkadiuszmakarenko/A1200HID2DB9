#ifndef __AMIGA_INCLUDED__
#define __AMIGA_INCLUDED__

#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include "main.h"
#include "usbh_hid_keybd.h"
#include "usbh_hid.h"
#include "usb_host.h"

#define KEY_PRESSED_MAX 6


typedef enum {
	NO_LED = 0,
	LED_CAPS_LOCK_ON,
	LED_NUM_LOCK_ON,
	LED_SCROLL_LOCK_ON,
	LED_CAPS_LOCK_OFF,
	LED_NUM_LOCK_OFF,
	LED_SCROLL_LOCK_OFF,
	LED_RESET_BLINK,
} led_status_t;

typedef enum {
	NUM_LOCK_LED = (1 << 0),
	CAPS_LOCK_LED = (1 << 1),
	SCROLL_LOCK_LED = (1 << 2),
} keyboard_led_t;


void amikb_process_irq();
void amikb_init();
void amikb_startup(void);
void amikb_process();
void amikb_notify(const char *notify);
void amikb_gpio_init(void);
void amikb_ready(int isready);
bool amikb_reset_check(void);
void amikb_reset(void);



extern ApplicationTypeDef Appli_state;
extern USBH_HandleTypeDef hUsbHostFS;
extern HID_KEYBD_Info_TypeDef *USBH_HID_GetKeybdInfo(USBH_HandleTypeDef *phost);
#endif