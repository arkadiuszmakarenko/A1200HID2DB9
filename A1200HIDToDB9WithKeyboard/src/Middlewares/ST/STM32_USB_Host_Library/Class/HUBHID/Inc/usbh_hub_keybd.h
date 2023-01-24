#ifndef __USBH_HUB_KEYBD_H
#define __USBH_HUB_KEYBD_H

/* Includes ------------------------------------------------------------------*/
#include "usbh_hid.h"
#include "usbh_hid_keybd.h"
#include "usbh_hub.h"

USBH_StatusTypeDef USBH_HUB_KeybdInit(USBH_HandleTypeDef *phost,HUB_Port_HandleTypeDef *port,HUB_Port_Interface_HandleTypeDef *Itf);
HID_KEYBD_Info_TypeDef *USBH_HUB_GetKeybdInfo(USBH_HandleTypeDef *phost);
uint8_t USBH_HUB_GetASCIICode(HID_KEYBD_Info_TypeDef *info);


#endif /* __USBH_HUB_KEYBD_H */



