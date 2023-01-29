#ifndef __USBH_HUB_GAMEPAD_H
#define __USBH_HUB_GAMEPAD_H

/* Includes ------------------------------------------------------------------*/
#include "usbh_hid.h"
#include "usbh_hub.h"


USBH_StatusTypeDef USBH_HUB_GamepadInit(HUB_Port_Interface_HandleTypeDef *Itf);
HID_gamepad_Info_TypeDef *USBH_HUB_GetGamepadInfo(HUB_Port_Interface_HandleTypeDef *Itf);




#endif
