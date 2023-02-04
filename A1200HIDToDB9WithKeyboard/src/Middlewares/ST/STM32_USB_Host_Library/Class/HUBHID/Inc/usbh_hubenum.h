#ifndef __USBH_HUB_NUM_H
#define __USBH_HUB_NUM_H

#include "usbh_core.h"
#include "usbh_hub.h"

USBH_StatusTypeDef USBH_HUB_Device_Enum(USBH_HandleTypeDef *phost, HUB_Port_HandleTypeDef *port);
USBH_StatusTypeDef USBH_HUB_Device_Process(USBH_HandleTypeDef *phost);
void USBH_HUB_SETUP_PIPES(USBH_HandleTypeDef *phost,HUB_HandleTypeDef *HUB_Handle,HUB_Port_HandleTypeDef *port,HUB_Port_Interface_HandleTypeDef *Itf);

#endif




