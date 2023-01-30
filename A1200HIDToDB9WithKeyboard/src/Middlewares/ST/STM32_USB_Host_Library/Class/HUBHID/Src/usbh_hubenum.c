#include "usbh_hubenum.h"
#include "usbh_hubctrl.h"
#include "usbh_core.h"
#include "usbh_hid_reportparser.h"
#include "usbh_hub_keybd.h"
#include "usbh_hub_mouse.h"
#include "usbh_hub_gamepad.h"

USBH_StatusTypeDef USBH_HUB_Device_Enum(USBH_HandleTypeDef *phost, HUB_Port_HandleTypeDef *port)
{
    USBH_StatusTypeDef status = USBH_BUSY;
    HUB_HandleTypeDef *HUB_Handle = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0];

    uint8_t max_ep = 0U;
    uint8_t num = 0U;

    switch (port->EnumState)
   {
    case HUB_ENUM_INIT:
            USBH_OpenPipe(phost, phost->Control.pipe_in, 0x80,
            phost->device.address, phost->device.speed,
            USBH_EP_CONTROL, (uint16_t)phost->Control.pipe_size);

            USBH_OpenPipe(phost, phost->Control.pipe_out, 0x00U,
            phost->device.address, phost->device.speed,
            USBH_EP_CONTROL, (uint16_t)phost->Control.pipe_size);

            port->EnumTime = phost->Timer;
            port->DevDescNum = 0;

            port->MFC = (uint8_t *)malloc(0xFF);
            port->Product = (uint8_t *)malloc(0xFF);

            //No HS support
            if (port->PortStatus.wPortStatus.PORT_LOW_SPEED)
            {
                port->speed = USBH_SPEED_LOW;
            }
            else
            {
                port->speed = USBH_SPEED_FULL;
            }
              port->address = HUB_Handle->portNumber+10;
              phost->Control.pipe_size = phost->device.DevDesc.bMaxPacketSize;
              status = USBH_BUSY;
              port->EnumState = HUB_ENUM_RESET_PORT;
    break;

        case HUB_ENUM_RESET_PORT:
        status = USBH_HUB_SetPortFeature(phost,HUB_FEAT_SEL_PORT_RESET,HUB_Handle->portNumber);
        if (status == USBH_OK)
        {
              HAL_Delay(100);
              status = USBH_BUSY;
              port->EnumState = HUB_ENUM_RESET_PORT2;              
        }

    break;

        case HUB_ENUM_RESET_PORT2:
        status = USBH_HUB_SetPortFeature(phost,HUB_FEAT_SEL_PORT_RESET,HUB_Handle->portNumber);
        if (status == USBH_OK)
        {
            HAL_Delay(200);

            USBH_OpenPipe(phost, phost->Control.pipe_in, 0x80U, 0U, port->speed, USBH_EP_CONTROL, (uint16_t)0x40U);
            USBH_OpenPipe(phost, phost->Control.pipe_out, 0x00U, 0U, port->speed, USBH_EP_CONTROL, (uint16_t)0x40U);
            phost->Control.pipe_size = 0x40U;
        
            status = USBH_BUSY;
            port->EnumState = HUB_ENUM_GET_DEV_DESC;             
        }
    break;
    
    case HUB_ENUM_GET_DEV_DESC:
    status = USBH_HUB_Get_DevDesc(phost, 8U,port);
        if (status == USBH_OK)
        {
        port->DevDescNum = 0;
        phost->Control.pipe_size = port->DevDesc.bMaxPacketSize;

                    USBH_OpenPipe(phost, phost->Control.pipe_in, 0x80U,
                    0U, port->speed,
                    USBH_EP_CONTROL, (uint16_t)port->DevDesc.bMaxPacketSize);

                    USBH_OpenPipe(phost, phost->Control.pipe_out, 0x00U,
                    0U, port->speed,
                    USBH_EP_CONTROL, (uint16_t)port->DevDesc.bMaxPacketSize);

            status = USBH_BUSY;
            port->EnumState = HUB_ENUM_SET_ADDR;
        } 
    break;

    case HUB_ENUM_SET_ADDR:
           status = USBH_SetAddress(phost,port->address);
           if (status == USBH_OK)
           {
                    USBH_OpenPipe(phost, phost->Control.pipe_in, 0x80U,
                    port->address, port->speed,
                    USBH_EP_CONTROL, (uint16_t)port->DevDesc.bMaxPacketSize);

                    USBH_OpenPipe(phost, phost->Control.pipe_out, 0x00U,
                    port->address, port->speed,
                    USBH_EP_CONTROL, (uint16_t)port->DevDesc.bMaxPacketSize);

            status = USBH_BUSY;
            port->EnumState = HUB_ENUM_GET_FULL_DEV_DESC; //HUB_ENUM_GET_CFG_DESC;
           }
    break;

        case HUB_ENUM_GET_FULL_DEV_DESC:
       status = USBH_HUB_Get_DevDesc(phost, port->DevDesc.bLength,port);
        if (status == USBH_OK)
        {
            status = USBH_BUSY;
            port->EnumState = HUB_ENUM_GET_CFG_DESC;
        }
        break;

    case HUB_ENUM_GET_CFG_DESC:
          status = USBH_HUB_Get_CfgDesc(phost,USB_CONFIGURATION_DESC_SIZE,port);
          if (status == USBH_OK)
          {
            status = USBH_BUSY;
            port->EnumState = HUB_ENUM_GET_FULL_CFG_DESC;
          }
    break;

    case HUB_ENUM_GET_FULL_CFG_DESC:
          status = USBH_HUB_Get_CfgDesc(phost,port->CfgDesc.wTotalLength,port);

          if (status == USBH_OK)
          {
            status = USBH_BUSY;
            port->EnumState = HUB_ENUM_GET_MFC_STRING_DESC;
          }

    break;

    case HUB_ENUM_GET_MFC_STRING_DESC:
          status = USBH_HUB_Get_StringDesc(phost, port->DevDesc.iManufacturer,port->MFC, 0xFFU,port);

          if (status == USBH_OK)
          {
            status = USBH_BUSY;
            port->EnumState = HUB_ENUM_GET_PRODUCT_STRING_DESC;
          }
    break;

    case HUB_ENUM_GET_PRODUCT_STRING_DESC:
         status = USBH_HUB_Get_StringDesc(phost, port->DevDesc.iProduct,port->Product, 0xFFU,port);
          if (status == USBH_OK)
          {
            status = USBH_BUSY;
            port->EnumState = HUB_ENUM_SET_CONFIGURATION;
          }
    break;

    //ENABLE DEVICE
        case HUB_ENUM_SET_CONFIGURATION:
            status = USBH_SetCfg(phost, (uint16_t)port->CfgDesc.bConfigurationValue);
            if (status == USBH_OK)
            {
                HAL_Delay(5);
                port->EnumState = HUB_ENUM_SET_WAKEUP_FEATURE;
                status = USBH_BUSY;
            }
    break;

      case HUB_ENUM_SET_WAKEUP_FEATURE:
      if ((port->CfgDesc.bmAttributes) & (1U << 5))
      {
            status = USBH_SetFeature(phost, FEATURE_SELECTOR_REMOTEWAKEUP);
            if (status == USBH_OK)
            {

                port->EnumState = HUB_ENUM_GET_HID_DESC;
                status = USBH_BUSY;
            }
      }
      else
      {
        port->EnumState = HUB_ENUM_GET_HID_DESC;
        status = USBH_BUSY;
      }
    break;

    case HUB_ENUM_GET_HID_DESC:
        status = USBH_HUB_GetHIDDescriptor(phost, USB_HID_DESC_SIZE, 0U,port->buff);
                if (status == USBH_OK)
                {
			        USBH_HID_ParseHIDDesc(&port->HIDDesc[0], port->buff);
                    status = USBH_BUSY;
                    port->EnumState = HUB_ENUM_GET_HID_REPORT_DESC;
                    HAL_Delay(5);
                }
    break;

    case HUB_ENUM_GET_HID_REPORT_DESC:
        	status = USBH_HUB_GetHIDReportDescriptor(phost, port->HIDDesc[0].wItemLength, 0U,port->buff);
            if (status == USBH_OK)
            {
                HAL_Delay(5);
			    parse_report_descriptor(port->buff, port->HIDDesc[0].wItemLength, &port->HIDDesc[0].RptDesc);
                if (port->CfgDesc.bNumInterfaces>1)
                {
                    //read second interface HID Descriptor if exist
                    status = USBH_BUSY;
                    port->EnumState = HUB_ENUM_GET_HID_DESC_INTER2;
                }
                else
                {
			        port->EnumState = HUB_ENUM_SET_PROTOCOL;
                    status = USBH_BUSY;
                }
		    }
    break;

    case HUB_ENUM_GET_HID_DESC_INTER2:

        status = USBH_HUB_GetHIDDescriptor(phost, USB_HID_DESC_SIZE, 1U,port->buff);
                if (status == USBH_OK)
                {
			        USBH_HID_ParseHIDDesc(&port->HIDDesc[1], port->buff);
                    status = USBH_BUSY;
                    port->EnumState = HUB_ENUM_GET_HID_REPORT_DESC_INTER2;
                }
    break;

    case HUB_ENUM_GET_HID_REPORT_DESC_INTER2:
        	status = USBH_HUB_GetHIDReportDescriptor(phost, port->HIDDesc[1].wItemLength, 1U,port->buff);
            if (status == USBH_OK)
            {
			    parse_report_descriptor(port->buff, port->HIDDesc[1].wItemLength, &port->HIDDesc[1].RptDesc);
			    port->EnumState = HUB_ENUM_SET_PROTOCOL;
                status = USBH_BUSY;
		    }
    break;

    case HUB_ENUM_SET_PROTOCOL:
            status = USBH_HID_SetProtocol(phost, 1U, 0U);
            if (status == USBH_OK || status == USBH_NOT_SUPPORTED)
            {
                port->EnumState = HUB_ENUM_SET_IDLE;
                status = USBH_BUSY;
            }
    break;

    case HUB_ENUM_SET_IDLE:
            if(port->CfgDesc.Itf_Desc[0].bInterfaceClass == 0x03 && port->CfgDesc.Itf_Desc[0].bInterfaceSubClass == 0x01 && (port->CfgDesc.Itf_Desc[0].bInterfaceProtocol == HID_KEYBRD_BOOT_CODE || port->HIDDesc[0].RptDesc.type == REPORT_TYPE_KEYBOARD) )
            {
			    status = USBH_HID_SetIdle(phost, 0U, 0U, 0U);
            }
            else
            {
                if (port->CfgDesc.bNumInterfaces>1)
                {
                    //read second interface HID Descriptor if exist
                    status = USBH_BUSY;
                    port->EnumState = HUB_ENUM_SET_PROTOCOL_INTER2;
                }
                else
                {
			        port->EnumState = HUB_ENUM_INTERFACE_INIT;
                    status = USBH_BUSY;
                }
            }

            if (status == USBH_OK || status == USBH_NOT_SUPPORTED)
            {
                if (port->CfgDesc.bNumInterfaces>1)
                {
                    //read second interface HID Descriptor if exist
                    status = USBH_BUSY;
                    port->EnumState = HUB_ENUM_SET_PROTOCOL_INTER2;
                }
                else
                {
			        port->EnumState = HUB_ENUM_INTERFACE_INIT;
                    status = USBH_BUSY;
                }
            }
    break;

    case HUB_ENUM_SET_PROTOCOL_INTER2:
            status = USBH_HID_SetProtocol(phost, 1U, 1U);
            if (status == USBH_OK || status == USBH_NOT_SUPPORTED)
            {
                port->EnumState = HUB_ENUM_SET_IDLE_INTER2;
                status = USBH_BUSY;
            }
    break;

    case HUB_ENUM_SET_IDLE_INTER2:
            if(port->CfgDesc.Itf_Desc[1].bInterfaceClass == 0x03 && port->CfgDesc.Itf_Desc[1].bInterfaceSubClass == 0x01 && (port->CfgDesc.Itf_Desc[1].bInterfaceProtocol == HID_KEYBRD_BOOT_CODE || port->HIDDesc[1].RptDesc.type == REPORT_TYPE_KEYBOARD))
            {
			    status = USBH_HID_SetIdle(phost, 0U, 0U, 1U);
            }
            else
            {
               // status = USBH_HID_SetIdle(phost, 0U, 0U, 1U);
               		port->EnumState = HUB_ENUM_INTERFACE_INIT;
                    status = USBH_BUSY;
            }

            if (status == USBH_OK||status == USBH_NOT_SUPPORTED)
            {
                    port->EnumState = HUB_ENUM_INTERFACE_INIT;
                    status = USBH_BUSY;
            }
    break;

    case HUB_ENUM_INTERFACE_INIT:
        port->Interface[0].Id = 0;
        port->Interface[0].length = port->HIDDesc[0].RptDesc.report_size;
        port->Interface[0].poll   = port->CfgDesc.Itf_Desc[0].Ep_Desc[0].bInterval;
        port->Interface[0].ep_addr = port->CfgDesc.Itf_Desc[0].Ep_Desc[0].bEndpointAddress;

        max_ep = port->CfgDesc.Itf_Desc[0].bNumEndpoints;
        num = 0;
        
        for (; num < max_ep; num++) {
            
            if (port->CfgDesc.Itf_Desc[0].Ep_Desc[num].bEndpointAddress & 0x80U) 
            {
				port->Interface[0].InEp = port->CfgDesc.Itf_Desc[0].Ep_Desc[num].bEndpointAddress;
            } 
        }

        port->Interface[0].Pipe_in  = USBH_AllocPipe(phost,port->Interface[0].InEp);

        if ((port->CfgDesc.Itf_Desc[0].bInterfaceClass == 0x03 && port->CfgDesc.Itf_Desc[0].bInterfaceSubClass == 0x01 && port->CfgDesc.Itf_Desc[0].bInterfaceProtocol == HID_KEYBRD_BOOT_CODE ) || (port->HIDDesc[0].RptDesc.type == REPORT_TYPE_KEYBOARD))
        {       
            USBH_HUB_KeybdInit(&port->Interface[0]);
        }

        if ((port->CfgDesc.Itf_Desc[1].bInterfaceClass == 0x03 && port->CfgDesc.Itf_Desc[0].bInterfaceSubClass == 0x01 && port->CfgDesc.Itf_Desc[0].bInterfaceProtocol == HID_MOUSE_BOOT_CODE )|| (port->HIDDesc[0].RptDesc.type == REPORT_TYPE_MOUSE))
        {       
            port->Interface[0].length = port->HIDDesc[0].RptDesc.report_size + (port->HIDDesc[0].RptDesc.report_id?1:0);
            port->Interface[0].HIDDesc = &port->HIDDesc[0];
            USBH_HUB_MouseInit(&port->Interface[0]);
        }

        if (port->HIDDesc[0].RptDesc.type == REPORT_TYPE_JOYSTICK)
        {       
            port->Interface[0].length = port->HIDDesc[0].RptDesc.report_size + (port->HIDDesc[0].RptDesc.report_id?1:0);
            port->Interface[0].HIDDesc = &port->HIDDesc[0];
            USBH_HUB_GamepadInit(&port->Interface[0]);
        }

        if (port->CfgDesc.bNumInterfaces>1)
        {
            //read second interface HID Descriptor if exist
            status = USBH_BUSY;
            port->EnumState = HUB_ENUM_INTERFACE_2_INIT;
        } else
        {
		    port->EnumState = HUB_ENUM_READY;
            status = USBH_BUSY;
        }
    break;

    case HUB_ENUM_INTERFACE_2_INIT:
        port->Interface[1].Id = 1;
        port->Interface[1].length = port->HIDDesc[1].RptDesc.report_size;
        port->Interface[1].poll   = port->CfgDesc.Itf_Desc[1].Ep_Desc[0].bInterval;
        port->Interface[1].ep_addr = port->CfgDesc.Itf_Desc[1].Ep_Desc[0].bEndpointAddress;

        max_ep = port->CfgDesc.Itf_Desc[1].bNumEndpoints;
        num = 0;

        for (; num < max_ep; num++) {
            
        if (port->CfgDesc.Itf_Desc[1].Ep_Desc[num].bEndpointAddress & 0x80U) 
        {
	    	port->Interface[1].InEp = port->CfgDesc.Itf_Desc[1].Ep_Desc[num].bEndpointAddress;
        } 
        }
            port->Interface[1].Pipe_in =  USBH_AllocPipe(phost,port->Interface[1].InEp);

        if ((port->CfgDesc.Itf_Desc[1].bInterfaceClass == 0x03 && port->CfgDesc.Itf_Desc[1].bInterfaceSubClass == 0x01 && port->CfgDesc.Itf_Desc[1].bInterfaceProtocol == HID_KEYBRD_BOOT_CODE) || (port->HIDDesc[1].RptDesc.type == REPORT_TYPE_KEYBOARD))
        {       
            USBH_HUB_KeybdInit(&port->Interface[1]);
        }

        if ((port->CfgDesc.Itf_Desc[1].bInterfaceClass == 0x03 && port->CfgDesc.Itf_Desc[1].bInterfaceSubClass == 0x01 && port->CfgDesc.Itf_Desc[1].bInterfaceProtocol == HID_MOUSE_BOOT_CODE ) || (port->HIDDesc[1].RptDesc.type == REPORT_TYPE_MOUSE))
        {       
            port->Interface[1].length = port->HIDDesc[1].RptDesc.report_size + (port->HIDDesc[1].RptDesc.report_id?1:0);
            port->Interface[1].HIDDesc = &port->HIDDesc[1];
            USBH_HUB_MouseInit(&port->Interface[1]);
        }

        if (port->HIDDesc[1].RptDesc.type == REPORT_TYPE_JOYSTICK)
        {       
            port->Interface[1].length = port->HIDDesc[1].RptDesc.report_size + (port->HIDDesc[1].RptDesc.report_id?1:0);
            USBH_HUB_GamepadInit(&port->Interface[1]);
        }
    		port->EnumState = HUB_ENUM_READY;
            status = USBH_BUSY;
    break;

    case HUB_ENUM_READY:
        status = USBH_OK;
    break;

   }
    return status;
}


USBH_StatusTypeDef USBH_HUB_Device_Process(USBH_HandleTypeDef *phost)
{
uint8_t XferSize;
USBH_StatusTypeDef status = USBH_BUSY;
HUB_HandleTypeDef *HUB_Handle  = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0]; 
HUB_Port_HandleTypeDef *port ;
HUB_Port_Interface_HandleTypeDef *Itf; 


USBH_URBStateTypeDef URBStatus;

uint8_t interfaceNumber =  HUB_Handle->current_Itf_number ;
uint8_t portNumber = HUB_Handle->current_port_number ;

    port = (HUB_Port_HandleTypeDef *) &HUB_Handle->Port[portNumber];
    if (port->EnumState != HUB_ENUM_READY) 
    {
        if(HUB_Handle->current_port_number == 3)
        {
            HUB_Handle->current_port_number = 0;
        } 
        else
        {
            HUB_Handle->current_port_number++;
        }
     return status;
    }

    Itf = (HUB_Port_Interface_HandleTypeDef *) &port->Interface[interfaceNumber];

    switch(Itf->state)
    {
        case HUB_DEVICE_INIT:
        USBH_HUB_SETUP_PIPES(phost,HUB_Handle,port,Itf);
        Itf->state = HUB_DEVICE_GET_DATA;
        break;
        case HUB_DEVICE_GET_DATA:
                
                USBH_InterruptReceiveData(phost, Itf->pData, (uint8_t) Itf->length, Itf->Pipe_in);
                Itf->timer = phost->Timer;
	            Itf->DataReady = 0U;
                Itf->state = HUB_DEVICE_POLL;
        break;

        case HUB_DEVICE_POLL:
                    URBStatus = USBH_LL_GetURBState(phost, Itf->Pipe_in);
                    if (URBStatus== USBH_URB_DONE)
                    {
			            XferSize = USBH_LL_GetLastXferSize(phost, Itf->Pipe_in);

                        if (Itf->DataReady == 0U && XferSize != 0U) 
                        { 
                            USBH_HID_FifoWrite(&Itf->fifo, Itf->pData, Itf->length);
				            Itf->DataReady = 1U;
                            
                            Itf->state = HUB_DEVICE_GET_DATA;

                            USBH_Next_Interface_Port(phost,port);

                            break;
                        }
			        }
                    if (URBStatus == USBH_URB_STALL)
                    {
                           while (USBH_ClrFeature(phost, Itf->ep_addr) == USBH_OK);
                           Itf->state = HUB_DEVICE_GET_DATA;
                           break;
                    }

                    if (URBStatus == USBH_URB_ERROR)
                    {
                        Itf->state = HUB_DEVICE_GET_DATA;
                        break;
                    }
    }

return status;
}

void USBH_HUB_SETUP_PIPES(USBH_HandleTypeDef *phost,HUB_HandleTypeDef *HUB_Handle,HUB_Port_HandleTypeDef *port,HUB_Port_Interface_HandleTypeDef *Itf)
{
	USBH_OpenPipe(phost, Itf->Pipe_in, Itf->InEp, port->address, port->speed, USB_EP_TYPE_INTR, Itf->length);
	USBH_LL_SetToggle(phost, Itf->Pipe_in, 0U);
}