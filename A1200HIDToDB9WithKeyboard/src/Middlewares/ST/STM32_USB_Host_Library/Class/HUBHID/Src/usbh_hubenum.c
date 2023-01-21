#include "usbh_hubenum.h"
#include "usbh_hubctrl.h"
#include "usbh_core.h"
#include "usbh_hid_reportparser.h"
#include "usbh_hub_keybd.h"



USBH_StatusTypeDef USBH_HUB_Device_Enum(USBH_HandleTypeDef *phost, HUB_Port_HandleTypeDef *port)
{
    USBH_StatusTypeDef status = USBH_BUSY;
    HUB_HandleTypeDef *HUB_Handle = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0];

    uint8_t max_ep = 0U;
    uint8_t num = 0U;

    switch (port->EnumState)
   {
    case HUB_ENUM_INIT:
            port->DevDescNum = 0;

            port->MFC = (uint8_t *)malloc(0xFF);
            port->Product = (uint8_t *)malloc(0xFF);


           port->Pipe_in = HUB_Handle->DevInPipe;
           port->Pipe_out =HUB_Handle->DevOutPipe;

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
              port->Pipe_size = 0x40U; //MPS_DEFAULT;

              //Make sure to use righ pipes and address to communitate with hub.
              phost->Control.pipe_size = phost->device.DevDesc.bMaxPacketSize;
              phost->Control.pipe_in = HUB_Handle->InPipe;
              phost->Control.pipe_out = HUB_Handle->OutPipe;


              status = USBH_BUSY;
              port->EnumState = HUB_ENUM_CLEAR_POWER_OFF_PORT;
    break;

    case HUB_ENUM_CLEAR_POWER_OFF_PORT:
        status = USBH_HUB_ClearPortFeature(phost,HUB_FEAT_SEL_PORT_POWER,HUB_Handle->portNumber);
                if (status == USBH_OK)
        {

              status = USBH_BUSY;
              port->EnumState = HUB_ENUM_CLEAR_POWER_ON_PORT;
        }
    break;


    case HUB_ENUM_CLEAR_POWER_ON_PORT:

        status = USBH_HUB_SetPortFeature(phost,HUB_FEAT_SEL_PORT_POWER,HUB_Handle->portNumber);
        if (status == USBH_OK)
        {
              status = USBH_BUSY;
              port->EnumState = HUB_ENUM_RESET_PORT;              
        }

    break;

        case HUB_ENUM_RESET_PORT:
        status = USBH_HUB_SetPortFeature(phost,HUB_FEAT_SEL_PORT_RESET,HUB_Handle->portNumber);
        if (status == USBH_OK)
        {
            HAL_Delay(200);
              status = USBH_BUSY;
              port->EnumState = HUB_ENUM_RESET_PORT2;              
        }

    break;


        case HUB_ENUM_RESET_PORT2:
        status = USBH_HUB_SetPortFeature(phost,HUB_FEAT_SEL_PORT_RESET,HUB_Handle->portNumber);
        if (status == USBH_OK)
        {
              HAL_Delay(200);
              status = USBH_BUSY;
              port->EnumState = HUB_ENUM_CHECK_ENABLE_PORT;              
        }

    break;
    



    case HUB_ENUM_CHECK_ENABLE_PORT:

        status = USBH_HUB_GetPortStatus(phost,HUB_Handle->portNumber);
        if (status == USBH_OK)
        {
                   USBH_OpenPipe(phost, port->Pipe_in, 0x80U,
                    0U, port->speed,
                    USBH_EP_CONTROL, (uint16_t)port->Pipe_size);

                    USBH_OpenPipe(phost, port->Pipe_out, 0x00U,
                    0U, port->speed,
                    USBH_EP_CONTROL, (uint16_t)port->Pipe_size);
                


                    phost->Control.pipe_size = port->Pipe_size;
                    phost->Control.pipe_in = port->Pipe_in;
                    phost->Control.pipe_out = port->Pipe_out;
              status = USBH_BUSY;
              port->EnumState = HUB_ENUM_GET_DEV_DESC;              
        }

    break;

    case HUB_ENUM_GET_DEV_DESC:
    HAL_Delay(10);
    status = USBH_HUB_Get_DevDesc(phost, 8U,port);
        port->DevDescNum++;

        if (port->DevDescNum > 100)
        {
            port->EnumState = HUB_ENUM_RESET_PORT;
            status=USBH_BUSY;   
        }

        if (status == USBH_OK)
        {
        port->DevDescNum = 0;
          phost->Control.pipe_size = port->DevDesc.bMaxPacketSize;
          port->Pipe_size = port->DevDesc.bMaxPacketSize;

        /* modify control channels configuration for MaxPacket size */
                   USBH_OpenPipe(phost, port->Pipe_in, 0x80U,
                    0U, port->speed,
                    USBH_EP_CONTROL, (uint16_t)port->Pipe_size);

                    USBH_OpenPipe(phost, port->Pipe_out, 0x00U,
                    0U, port->speed,
                    USBH_EP_CONTROL, (uint16_t)port->Pipe_size);

            status = USBH_BUSY;
            port->EnumState = HUB_ENUM_GET_FULL_DEV_DESC;
        } 

    break;

    case HUB_ENUM_GET_FULL_DEV_DESC:
       status = USBH_HUB_Get_DevDesc(phost, port->DevDesc.bLength,port);
        if (status == USBH_OK)
        {
            status = USBH_BUSY;
            port->EnumState = HUB_ENUM_SET_ADDR;
        }
        break;

    case HUB_ENUM_SET_ADDR:
           status = USBH_SetAddress(phost,port->address);

           if (status == USBH_OK)
           {
            /* modify control channels to update device address */
                USBH_OpenPipe(phost, port->Pipe_in, 0x80U,  port->address,
                      port->speed, USBH_EP_CONTROL,
                      (uint16_t)port->Pipe_size);
                      
                USBH_OpenPipe(phost, port->Pipe_out , 0x00U, port->address,
                      port->speed, USBH_EP_CONTROL,
                      (uint16_t)port->Pipe_size);

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
            port->EnumState = HUB_ENUM_GET_HID_DESC;
          }
    break;


    case HUB_ENUM_GET_HID_DESC:

        status = USBH_HUB_GetHIDDescriptor(phost, USB_HID_DESC_SIZE, 0U,port->buff);
                if (status == USBH_OK)
                {
			        USBH_HID_ParseHIDDesc(&port->HIDDesc[0], port->buff);
                    status = USBH_BUSY;
                    port->EnumState = HUB_ENUM_GET_HID_REPORT_DESC;
                }
    break;

    case HUB_ENUM_GET_HID_REPORT_DESC:

        	status = USBH_HUB_GetHIDReportDescriptor(phost, port->HIDDesc[0].wItemLength, 0U,port->buff);
                
            if (status == USBH_OK)
            {
			    parse_report_descriptor(port->buff, port->HIDDesc[0].wItemLength, &port->HIDDesc[0].RptDesc);
                if (port->CfgDesc.bNumInterfaces>1)
                {
                    //read second interface HID Descriptor if exist
                    status = USBH_BUSY;
                    port->EnumState = HUB_ENUM_GET_HID_DESC_INTER2;
                }
                else
                {
			        port->EnumState = HUB_ENUM_SET_CONFIGURATION;
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
			    port->EnumState = HUB_ENUM_SET_CONFIGURATION;
                status = USBH_BUSY;
		    }
    break;

    case HUB_ENUM_SET_CONFIGURATION:
            status = USBH_SetCfg(phost, (uint16_t)port->CfgDesc.bConfigurationValue);
            if (status == USBH_OK)
            {
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
                port->EnumState = HUB_ENUM_SET_PROTOCOL;
                status = USBH_BUSY;
            }
      }
      else
      {
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
			    status = USBH_HID_SetIdle(phost, 125U, 0U, 0U);
            }
            else
            {
                status = USBH_HID_SetIdle(phost, 0U, 0U, 0U);
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
			    status = USBH_HID_SetIdle(phost, 125U, 0U, 1U);
            }
            else
            {
                status = USBH_HID_SetIdle(phost, 0U, 0U, 1U);
            }

            if (status == USBH_OK||status == USBH_NOT_SUPPORTED)
            {
			        port->EnumState = HUB_ENUM_INTERFACE_INIT;
                    status = USBH_BUSY;
            }
    break;

    case HUB_ENUM_INTERFACE_INIT:

        if ((port->CfgDesc.Itf_Desc[0].bInterfaceClass == 0x03 && port->CfgDesc.Itf_Desc[0].bInterfaceSubClass == 0x01 && port->CfgDesc.Itf_Desc[0].bInterfaceProtocol == HID_KEYBRD_BOOT_CODE) || (port->HIDDesc[0].RptDesc.type == REPORT_TYPE_KEYBOARD))
        {       
            USBH_HUB_KeybdInit(phost);
        }

        //read report lenght

        port->Interface[0].length = port->HIDDesc[0].RptDesc.report_size;
        port->Interface[0].pData = malloc(port->HIDDesc[0].RptDesc.report_size);
        port->Interface[0].poll   = port->CfgDesc.Itf_Desc[0].Ep_Desc[0].bInterval;
        port->Interface[0].ep_addr = port->CfgDesc.Itf_Desc[0].Ep_Desc[0].bEndpointAddress;

        max_ep = port->CfgDesc.Itf_Desc[0].bNumEndpoints;
        num = 0;
        
        for (; num < max_ep; num++) {
            
            if (port->CfgDesc.Itf_Desc[0].Ep_Desc[num].bEndpointAddress & 0x80U) 
            {
				port->Interface[0].InEp = port->CfgDesc.Itf_Desc[0].Ep_Desc[num].bEndpointAddress;
            } 
            else 
            {
			    port->Interface[0].OutEp = port->CfgDesc.Itf_Desc[0].Ep_Desc[num].bEndpointAddress;
			}
        }

        port->Interface[0].Pipe_in  = port->Pipe_in;
        port->Interface[0].Pipe_out = port->Pipe_out;




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
        if ((port->CfgDesc.Itf_Desc[1].bInterfaceClass == 0x03 && port->CfgDesc.Itf_Desc[1].bInterfaceSubClass == 0x01 && port->CfgDesc.Itf_Desc[1].bInterfaceProtocol == HID_KEYBRD_BOOT_CODE) || (port->HIDDesc[1].RptDesc.type == REPORT_TYPE_KEYBOARD))
        {       
            USBH_HUB_KeybdInit(phost);
        }

        //read report lenght

        port->Interface[1].length = 8U; //port->HIDDesc[1].RptDesc.report_size;
        port->Interface[1].pData = malloc(8U);//malloc(port->HIDDesc[1].RptDesc.report_size);
        port->Interface[1].poll   = port->CfgDesc.Itf_Desc[1].Ep_Desc[0].bInterval;
        port->Interface[1].ep_addr = port->CfgDesc.Itf_Desc[1].Ep_Desc[0].bEndpointAddress;

        max_ep = port->CfgDesc.Itf_Desc[1].bNumEndpoints;
        num = 0;

        for (; num < max_ep; num++) {
            
        if (port->CfgDesc.Itf_Desc[1].Ep_Desc[num].bEndpointAddress & 0x80U) 
        {
	    	port->Interface[1].InEp = port->CfgDesc.Itf_Desc[1].Ep_Desc[num].bEndpointAddress;
        } else 
        {
		    port->Interface[1].OutEp = port->CfgDesc.Itf_Desc[1].Ep_Desc[num].bEndpointAddress;
		}
        }

            port->Interface[1].Pipe_in = USBH_AllocPipe(phost, port->Interface[1].InEp);
            port->Interface[1].Pipe_out= USBH_AllocPipe(phost, port->Interface[1].OutEp );




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
USBH_StatusTypeDef status = USBH_BUSY;
HUB_HandleTypeDef *HUB_Handle  = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0]; 
HUB_Port_HandleTypeDef *port ;
HUB_Port_Interface_HandleTypeDef *Itf; 

    uint8_t XferSize = 0U;
    USBH_URBStateTypeDef URBStatus;


    //sync
    while(phost->Timer & 1U);

    uint8_t portNumber = 0;
    uint8_t interfaceNumber = 1;
while(1)
{
    for(portNumber = 0;portNumber<4;portNumber++)
    {
        port = (HUB_Port_HandleTypeDef *) &HUB_Handle->Port[portNumber];
        if (port->EnumState != HUB_ENUM_READY) continue;


        for(interfaceNumber = 0 ;interfaceNumber<(port->CfgDesc.bNumInterfaces);interfaceNumber++)
        {
           Itf = (HUB_Port_Interface_HandleTypeDef *) &port->Interface[interfaceNumber];

           USBH_HUB_SETUP_PIPES(phost,HUB_Handle,port,Itf);

                USBH_InterruptReceiveData(phost, Itf->buff, (uint8_t) Itf->length, Itf->Pipe_in);

		        Itf->timer = phost->Timer;
		        Itf->DataReady = 0U;

                while ( (phost->Timer - Itf->timer) < Itf->poll )   
                {
                    URBStatus = USBH_LL_GetURBState(phost, Itf->Pipe_in);
                    if (URBStatus== USBH_URB_DONE)
                    {
			            XferSize = USBH_LL_GetLastXferSize(phost, Itf->Pipe_in);
                        break;
			        }

                    if (URBStatus == USBH_URB_ERROR)
                    {
                        break;
                    }
                }
		    }

        



        }






    


}

return status;
}


void USBH_HUB_SETUP_PIPES(USBH_HandleTypeDef *phost,HUB_HandleTypeDef *HUB_Handle,HUB_Port_HandleTypeDef *port,HUB_Port_Interface_HandleTypeDef *Itf)
{
				/* Open pipe for IN endpoint*/
                
				USBH_OpenPipe(phost, Itf->Pipe_in, Itf->InEp, port->address, port->speed, USB_EP_TYPE_INTR, Itf->length);
				USBH_LL_SetToggle(phost, Itf->Pipe_in, 1U);

                				/* Open pipe for OUT endpoint*/
				USBH_OpenPipe(phost, Itf->Pipe_out, Itf->OutEp, port->address, port->speed, USB_EP_TYPE_INTR, Itf->length);
				USBH_LL_SetToggle(phost, Itf->Pipe_out, 0U);


}