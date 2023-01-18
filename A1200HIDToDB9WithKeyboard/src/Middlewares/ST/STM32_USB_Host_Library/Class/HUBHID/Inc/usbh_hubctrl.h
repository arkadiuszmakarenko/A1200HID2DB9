#ifndef __USBH_HUB_CTRL_H
#define __USBH_HUB_CTRL_H

#include "usbh_core.h"
#include "usbh_def.h"
#include "usbh_hub.h"




#define MAX_HUB_PORTS 						 4


#define USB_REQUEST_GET_DESCRIPTOR           0x06

#define USB_DEVICE_REQUEST_SET   			 0x00
#define USB_DEVICE_REQUEST_GET   			 0x01
#define USB_REQUEST_CLEAR_FEATURE   		 0x01
#define USB_REQUEST_SET_FEATURE     		 0x03
#define USB_REQUEST_GET_STATUS          	 0x00

#define HUB_FEAT_SEL_PORT_CONNECTION 		 0x00
#define HUB_FEAT_SEL_C_HUB_LOCAL_POWER       0x00
#define HUB_FEAT_SEL_C_HUB_OVER_CURRENT      0x01

#define HUB_FEAT_SEL_PORT_CONN         		 0x00
#define HUB_FEAT_SEL_PORT_ENABLE             0x01
#define HUB_FEAT_SEL_PORT_SUSPEND            0x02
#define HUB_FEAT_SEL_PORT_OVER_CURRENT       0x03
#define HUB_FEAT_SEL_PORT_RESET              0x04
#define HUB_FEAT_SEL_PORT_POWER              0x08
#define HUB_FEAT_SEL_PORT_LOW_SPEED          0x09
#define HUB_FEAT_SEL_C_PORT_CONNECTION       0x10
#define HUB_FEAT_SEL_C_PORT_ENABLE           0x11
#define HUB_FEAT_SEL_C_PORT_SUSPEND          0x12
#define HUB_FEAT_SEL_C_PORT_OVER_CURRENT     0x13
#define HUB_FEAT_SEL_C_PORT_RESET            0x14
#define HUB_FEAT_SEL_PORT_INDICATOR          0x16



USBH_StatusTypeDef USBH_HUB_GetDescriptor(USBH_HandleTypeDef *phost);
void USBH_HUB_GetHUBStatus(USBH_HandleTypeDef *phost);
USBH_StatusTypeDef USBH_HUB_GetPortStatus(USBH_HandleTypeDef *phost, uint8_t PortNum);
void USBH_HUB_ParseHubDescriptor(HUB_DescTypeDef  *hub_descriptor, uint8_t *buf);
void USBH_HUB_ParseHUBStatus(HUB_HandleTypeDef *HUB_Handle,uint8_t *buf);
void USBH_HUB_ParsePortStatus(HUB_HandleTypeDef *HUB_Handle,uint8_t *buf,HUB_Port_HandleTypeDef *Port);
USBH_StatusTypeDef USBH_HUB_SetPortFeature(USBH_HandleTypeDef *phost, uint8_t feature, uint8_t PortNum);
USBH_StatusTypeDef USBH_HUB_ClearPortFeature(USBH_HandleTypeDef *phost, uint8_t feature, uint8_t PortNum);
USBH_StatusTypeDef USBH_HUB_Get_DevDesc(USBH_HandleTypeDef *phost, uint8_t length,HUB_Port_HandleTypeDef *Port);
USBH_StatusTypeDef USBH_HUB_Get_StringDesc(USBH_HandleTypeDef *phost,uint8_t string_index, uint8_t *buff,uint16_t length, HUB_Port_HandleTypeDef *Port);
USBH_StatusTypeDef USBH_HUB_GetHIDDescriptor(USBH_HandleTypeDef *phost,uint16_t length, uint8_t iface_idx, uint8_t *buf);
USBH_StatusTypeDef USBH_HUB_GetHIDReportDescriptor(USBH_HandleTypeDef *phost, uint16_t length, uint8_t iface_idx,uint8_t *buf);


void  USBH_HUB_ParseDevDesc(USBH_DevDescTypeDef *dev_desc, uint8_t *buf, uint16_t length);

USBH_StatusTypeDef USBH_HUB_Get_CfgDesc(USBH_HandleTypeDef *phost, uint16_t length, HUB_Port_HandleTypeDef *Port);

#endif