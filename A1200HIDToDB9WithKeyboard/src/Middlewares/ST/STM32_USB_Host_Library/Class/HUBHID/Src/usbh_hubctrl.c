#include "usbh_hubctrl.h"



USBH_StatusTypeDef USBH_HUB_GetDescriptor(USBH_HandleTypeDef *phost)
{
  uint16_t lenght = sizeof(HUB_DescTypeDef);

  if (phost->RequestState == CMD_SEND)
  {
    phost->Control.setup.b.bmRequestType = 0b10100000;
    phost->Control.setup.b.bRequest = USB_REQ_GET_DESCRIPTOR;		
    phost->Control.setup.b.wValue.bw.msb = 0;
	  phost->Control.setup.b.wValue.bw.lsb = 0x29;
    phost->Control.setup.b.wIndex.w = 0;
    phost->Control.setup.b.wLength.w = lenght;
  }

  return USBH_CtlReq(phost, phost->device.Data, lenght) ;
}

void USBH_HUB_GetHUBStatus(USBH_HandleTypeDef *phost)
{
      HUB_HandleTypeDef *HUB_Handle = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0];


  if (phost->RequestState == CMD_SEND)
  {
    phost->Control.setup.b.bmRequestType = 0b10100000;
    phost->Control.setup.b.bRequest = USB_REQUEST_GET_STATUS;		
    phost->Control.setup.b.wValue.w = 0;
    phost->Control.setup.b.wIndex.w = 0;
    phost->Control.setup.b.wLength.w = 4;
  }

  while (USBH_CtlReq(phost, phost->device.Data, 4) != USBH_OK);

  USBH_HUB_ParseHUBStatus(HUB_Handle,phost->device.Data);
}


USBH_StatusTypeDef USBH_HUB_GetPortStatus(USBH_HandleTypeDef *phost, uint8_t PortNum)
{
  USBH_StatusTypeDef status;

  HUB_HandleTypeDef *HUB_Handle = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0];

  
	phost->Control.setup.b.bmRequestType = 0b10100011;
	phost->Control.setup.b.bRequest  	 = USB_REQUEST_GET_STATUS;
	phost->Control.setup.b.wValue.bw.msb = HUB_FEAT_SEL_PORT_CONN;
	phost->Control.setup.b.wValue.bw.lsb = 0;
	phost->Control.setup.b.wIndex.bw.msb = PortNum;
  phost->Control.setup.b.wIndex.bw.lsb = 0;
	phost->Control.setup.b.wLength.w     =  4;
  
	
  if ((status = USBH_CtlReq(phost, HUB_Handle->buff, 4)) == USBH_OK)
  {
     USB_HUB_PORT_STATUS old_status = HUB_Handle->Port[PortNum-1].PortStatus;
    USBH_HUB_ParsePortStatus(HUB_Handle,HUB_Handle->buff,&HUB_Handle->Port[PortNum-1]);

    if (HUB_Handle->Port[PortNum-1].PortStatus.wPortStatus.val != old_status.wPortStatus.val)
    {
      HUB_Handle->PortStatusChangeFlag = 1;

      if ((old_status.wPortStatus.PORT_CONNECTION == 1) && (HUB_Handle->Port[PortNum-1].PortStatus.wPortStatus.PORT_CONNECTION == 0))
      {
        HUB_Handle->Port[PortNum-1].Disconnected = 1;
      }

      if ((old_status.wPortStatus.PORT_CONNECTION == 0) && (HUB_Handle->Port[PortNum-1].PortStatus.wPortStatus.PORT_CONNECTION == 1))
      {
        HUB_Handle->Port[PortNum-1].Connected = 1;
      }


    }
  }

  return status;
}



USBH_StatusTypeDef USBH_HUB_SetPortFeature(USBH_HandleTypeDef *phost, uint8_t feature, uint8_t PortNum)
{

  if (phost->RequestState == CMD_SEND)
  {
    phost->Control.setup.b.bmRequestType = 0b00100011; 
    phost->Control.setup.b.bRequest = USB_REQUEST_SET_FEATURE;		
    phost->Control.setup.b.wValue.bw.msb = feature;
	  phost->Control.setup.b.wValue.bw.lsb = 0x0;
    phost->Control.setup.b.wIndex.bw.msb = PortNum;
    phost->Control.setup.b.wIndex.bw.lsb = 0x0;
    phost->Control.setup.b.wLength.w = 0;
  }    

   return USBH_CtlReq(phost, 0, 0);
}


USBH_StatusTypeDef USBH_HUB_ClearPortFeature(USBH_HandleTypeDef *phost, uint8_t feature, uint8_t PortNum)
{

  if (phost->RequestState == CMD_SEND)
  {
    phost->Control.setup.b.bmRequestType = 0b00100011;
    phost->Control.setup.b.bRequest = USB_REQUEST_CLEAR_FEATURE;	
    phost->Control.setup.b.wValue.bw.msb = feature;
	  phost->Control.setup.b.wValue.bw.lsb = 0x0;
    phost->Control.setup.b.wIndex.bw.msb = PortNum;
    phost->Control.setup.b.wIndex.bw.lsb = 0x0;
    phost->Control.setup.b.wLength.w = 0;
  }    

   return USBH_CtlReq(phost, 0, 0);
}


USBH_StatusTypeDef USBH_HUB_Get_DevDesc(USBH_HandleTypeDef *phost, uint8_t length,HUB_Port_HandleTypeDef *Port)
{
      USBH_StatusTypeDef status;

   status = USBH_GetDescriptor(phost,USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD,USB_DESC_DEVICE, 0U,Port->buff, (uint16_t)length);
  if (status == USBH_OK)                                  
 { 
    /* Commands successfully sent and Response Received */
    USBH_HUB_ParseDevDesc(&Port->DevDesc, Port->buff, (uint16_t)length);
 }


 return status;

}


USBH_StatusTypeDef USBH_HUB_Get_CfgDesc(USBH_HandleTypeDef *phost,
                                    uint16_t length, HUB_Port_HandleTypeDef *Port)

{
  USBH_StatusTypeDef status;
        HUB_HandleTypeDef *HUB_Handle = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0];


  if ((status = USBH_GetDescriptor(phost, (USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD),
                                   USB_DESC_CONFIGURATION,0U,HUB_Handle->buff, length)) == USBH_OK)
  {
    /* Commands successfully sent and Response Received  */
    USBH_ParseCfgDesc(&Port->CfgDesc, HUB_Handle->buff, length);
  }

  return status;
}



USBH_StatusTypeDef USBH_HUB_Get_StringDesc(USBH_HandleTypeDef *phost,
                                       uint8_t string_index, uint8_t *buff,
                                       uint16_t length, HUB_Port_HandleTypeDef *Port)
{
  USBH_StatusTypeDef status;

  if ((status = USBH_GetDescriptor(phost,
                                   USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD,
                                   USB_DESC_STRING | string_index,0U,
                                   Port->buff, length)) == USBH_OK)
  {
    /* Commands successfully sent and Response Received  */
    USBH_ParseStringDesc(Port->buff, buff, length);
  }

  return status;
}



void  USBH_HUB_ParseDevDesc(USBH_DevDescTypeDef *dev_desc, uint8_t *buf,
                               uint16_t length)
{
  dev_desc->bLength            = *(uint8_t *)(buf +  0);
  dev_desc->bDescriptorType    = *(uint8_t *)(buf +  1);
  dev_desc->bcdUSB             = LE16(buf +  2);
  dev_desc->bDeviceClass       = *(uint8_t *)(buf +  4);
  dev_desc->bDeviceSubClass    = *(uint8_t *)(buf +  5);
  dev_desc->bDeviceProtocol    = *(uint8_t *)(buf +  6);
  dev_desc->bMaxPacketSize     = *(uint8_t *)(buf +  7);

  if (length > 8U)
  {
    /* For 1st time after device connection, Host may issue only 8 bytes for
    Device Descriptor Length  */
    dev_desc->idVendor           = LE16(buf +  8);
    dev_desc->idProduct          = LE16(buf + 10);
    dev_desc->bcdDevice          = LE16(buf + 12);
    dev_desc->iManufacturer      = *(uint8_t *)(buf + 14);
    dev_desc->iProduct           = *(uint8_t *)(buf + 15);
    dev_desc->iSerialNumber      = *(uint8_t *)(buf + 16);
    dev_desc->bNumConfigurations = *(uint8_t *)(buf + 17);
  }
}



void  USBH_HUB_ParseHubDescriptor(HUB_DescTypeDef  *hub_descriptor,
                              uint8_t *buf)
{
  hub_descriptor->bDescLength         = *(uint8_t *)(buf + 0);
  hub_descriptor->bDescriptorType     = *(uint8_t *)(buf + 1);
  hub_descriptor->bNbrPorts           = *(uint8_t *)(buf + 2);
  hub_descriptor->wHubCharacteristics = LE16(buf + 3);
  hub_descriptor->bPwrOn2PwrGood      = *(uint8_t *)(buf + 5);
  hub_descriptor->bHubContrCurrent    = *(uint8_t *)(buf + 6);
  hub_descriptor->DeviceRemovable     = *(uint8_t *)(buf + 7);
  hub_descriptor->PortPwrCtrlMask     = *(uint8_t *)(buf + 8);

}


void  USBH_HUB_ParseHUBStatus(HUB_HandleTypeDef *HUB_Handle,uint8_t *buf)
{
  HUB_Handle->HubStatus[0]         = *(uint8_t *)(buf + 0);
  HUB_Handle->HubStatus[1]         = *(uint8_t *)(buf + 1);
  HUB_Handle->HubStatus[2]         = *(uint8_t *)(buf + 2);
  HUB_Handle->HubStatus[3]         = *(uint8_t *)(buf + 3);
}

 void  USBH_HUB_ParsePortStatus(HUB_HandleTypeDef *HUB_Handle,uint8_t *buf,HUB_Port_HandleTypeDef *Port)
{
  Port->PortStatus= *(USB_HUB_PORT_STATUS *)(buf);
}


USBH_StatusTypeDef USBH_HUB_GetHIDDescriptor(USBH_HandleTypeDef *phost,
		uint16_t length, uint8_t iface_idx, uint8_t *buf) {
	USBH_StatusTypeDef status;

	status = USBH_GetDescriptor(phost,
	USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_STANDARD,
	USB_DESC_HID, iface_idx, phost->device.Data, length);

	return status;
}


USBH_StatusTypeDef USBH_HUB_GetHIDReportDescriptor(USBH_HandleTypeDef *phost, uint16_t length, uint8_t iface_idx,uint8_t *buf)
  {

	USBH_StatusTypeDef status;

	status = USBH_GetDescriptor(phost,
	USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_STANDARD,
	USB_DESC_HID_REPORT, iface_idx, buf, length);

	return status;
}



USBH_StatusTypeDef USBH_HUB_SetIdle(USBH_HandleTypeDef *phost, uint16_t length, uint8_t iface_idx,uint8_t *buf)
  {

	USBH_StatusTypeDef status;

	status = USBH_GetDescriptor(phost,
	USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_STANDARD,
	USB_DESC_HID_REPORT, iface_idx, buf, length);

	return status;
}