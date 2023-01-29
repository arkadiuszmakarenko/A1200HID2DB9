#ifndef __USBH_HUB_MOUSE_H
#define __USBH_HUB_MOUSE_H

/* Includes ------------------------------------------------------------------*/
#include "usbh_hid.h"
#include "usbh_hid_mouse.h"
#include "usbh_hub.h"


HID_MOUSE_Info_TypeDef *USBH_HUB_GetMouseInfo(HUB_Port_Interface_HandleTypeDef *Itf);
USBH_StatusTypeDef USBH_HUB_MouseInit(HUB_Port_Interface_HandleTypeDef *Itf);
USBH_StatusTypeDef USBH_HUB_MouseDecode(HUB_Port_Interface_HandleTypeDef *Itf);

#endif /* __USBH_HUB_KEYBD_H */



