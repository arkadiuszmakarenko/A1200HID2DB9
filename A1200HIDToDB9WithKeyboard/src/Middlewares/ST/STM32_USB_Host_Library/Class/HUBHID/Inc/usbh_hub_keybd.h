#ifndef __USBH_HUB_KEYBD_H
#define __USBH_HUB_KEYBD_H

/* Includes ------------------------------------------------------------------*/
#include "usbh_hid.h"
#include "usbh_hid_keybd.h"
#include "usbh_hub.h"
#include "usbh_hid_parser.h"

USBH_StatusTypeDef USBH_HUB_KeybdInit(HUB_Port_Interface_HandleTypeDef *Itf);
HID_KEYBD_Info_TypeDef *USBH_HUB_GetKeybdInfo(HUB_Port_Interface_HandleTypeDef *Itf);
uint8_t USBH_HUB_GetASCIICode(HID_KEYBD_Info_TypeDef *info);
USBH_StatusTypeDef USBH_HUB_KeybdDecode(HUB_Port_Interface_HandleTypeDef *Itf);


#endif /* __USBH_HUB_KEYBD_H */



