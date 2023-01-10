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


void USBH_HUB_GetPortStatus(USBH_HandleTypeDef *phost, uint8_t PortNum)
{
  HUB_HandleTypeDef *HUB_Handle = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0];

  
	phost->Control.setup.b.bmRequestType = 0b10100011;
	phost->Control.setup.b.bRequest  	 = USB_REQUEST_GET_STATUS;
	phost->Control.setup.b.wValue.bw.msb = HUB_FEAT_SEL_PORT_CONN;
	phost->Control.setup.b.wValue.bw.lsb = 0;
	phost->Control.setup.b.wIndex.bw.msb = PortNum;
  phost->Control.setup.b.wIndex.bw.lsb = 0;
	phost->Control.setup.b.wLength.w     =  4;
  
	
  while(USBH_CtlReq(phost, phost->device.Data, 4) != USBH_OK);

  USBH_HUB_ParsePortStatus(HUB_Handle,phost->device.Data,PortNum-1);

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

void USBH_HUB_SetPortFeatureBL(USBH_HandleTypeDef *phost, uint8_t feature, uint8_t PortNum)
{


    phost->Control.setup.b.bmRequestType = 0b00100011; 
    phost->Control.setup.b.bRequest = USB_REQUEST_SET_FEATURE;		
    phost->Control.setup.b.wValue.bw.msb = feature;
	  phost->Control.setup.b.wValue.bw.lsb = 0x0;
    phost->Control.setup.b.wIndex.bw.msb = PortNum;
    phost->Control.setup.b.wIndex.bw.lsb = 0x0;
    phost->Control.setup.b.wLength.w = 0;

   while(USBH_CtlReq(phost, 0, 0)!=USBH_OK);

}


void USBH_HUB_GetDevDescriptor(USBH_HandleTypeDef *phost)
{

    phost->device.address = USBH_DEVICE_ADDRESS_DEFAULT;
    HUB_HandleTypeDef *HUB_Handle = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0];

    phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD;
    phost->Control.setup.b.bRequest = USB_REQ_GET_DESCRIPTOR;
    phost->Control.setup.b.wValue.w = USB_DESC_DEVICE;
    phost->Control.setup.b.wIndex.w = 0U;
    phost->Control.setup.b.wLength.w = 8U;
  
  while (USBH_CtlReq(phost, HUB_Handle->buff, 8U) != USBH_OK);

  //uint8_t *buf = HUB_Handle->buff;

  //uint8_t   bLength = *(uint8_t *)(buf +  0);
  //uint8_t   bDescriptorType = *(uint8_t *)(buf +  1);
  //uint16_t  bcdUSB = LE16(buf +  2);
  //uint8_t   bDeviceClass = *(uint8_t *)(buf +  4);
  //uint8_t   bDeviceSubClass = *(uint8_t *)(buf +  5);
  //uint8_t   bDeviceProtocol= *(uint8_t *)(buf +  6);
  //uint8_t   bMaxPacketSize     = *(uint8_t *)(buf +  7);


  //phost->device.address = PrevAddress;

}

void USBH_HUB_Get_DevDesc(USBH_HandleTypeDef *phost, uint8_t length)
{

      HUB_HandleTypeDef *HUB_Handle = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0];


  while(USBH_GetDescriptor(phost,
                                   USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD,
                                   USB_DESC_DEVICE, 0U,HUB_Handle->Port1.buff,
                                   (uint16_t)length) != USBH_OK);
                                   ;
  
    /* Commands successfully sent and Response Received */
    USBH_HUB_ParseDevDesc(&HUB_Handle->Port1.DevDesc, HUB_Handle->Port1.buff,
                      (uint16_t)length);
  

}


USBH_StatusTypeDef USBH_HUB_Get_CfgDesc(USBH_HandleTypeDef *phost,
                                    uint16_t length)

{
  USBH_StatusTypeDef status;
        HUB_HandleTypeDef *HUB_Handle = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0];


  if ((status = USBH_GetDescriptor(phost, (USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD),
                                   USB_DESC_CONFIGURATION,0U,HUB_Handle->Port1.buff, length)) == USBH_OK)
  {
    /* Commands successfully sent and Response Received  */
    USBH_ParseCfgDesc(&HUB_Handle->Port1.CfgDesc, HUB_Handle->Port1.buff, length);
  }

  return status;
}



static void  USBH_HUB_ParseDevDesc(USBH_DevDescTypeDef *dev_desc, uint8_t *buf,
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

 void  USBH_HUB_ParsePortStatus(HUB_HandleTypeDef *HUB_Handle,uint8_t *buf,uint8_t PortNum)
{
  HUB_Handle->PortStatus[PortNum]  = *(USB_HUB_PORT_STATUS *)(buf);
}
