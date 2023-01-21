#ifndef __USBH_HUB_KEYBD_H
#define __USBH_HUB_KEYBD_H

/* Includes ------------------------------------------------------------------*/
#include "usbh_hid.h"
#include "usbh_hid_keybd.h"

USBH_StatusTypeDef USBH_HUB_KeybdInit(USBH_HandleTypeDef *phost);
HID_KEYBD_Info_TypeDef *USBH_HUB_GetKeybdInfo(USBH_HandleTypeDef *phost);
uint8_t USBH_HUB_GetASCIICode(HID_KEYBD_Info_TypeDef *info);


#endif /* __USBH_HUB_KEYBD_H */



