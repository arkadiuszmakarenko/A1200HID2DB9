#include "usbh_hubenum.h"
#include "usbh_hubctrl.h"
#include "usbh_core.h"
#include "usbh_hid_reportparser.h"



USBH_StatusTypeDef USBH_HUB_Device_Enum(USBH_HandleTypeDef *phost, HUB_Port_HandleTypeDef *port)
{
    USBH_StatusTypeDef status = USBH_BUSY;
    HUB_HandleTypeDef *HUB_Handle = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0];

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
            port->DevDescNum++;
            if (port->DevDescNum > 100)
        {
                    free(port->MFC);
    free(port->Product);



    port->EnumState = HUB_ENUM_INIT;
    status=USBH_BUSY;
               
        }
           
  

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
			        USBH_HID_ParseHIDDesc(&port->HIDDesc, port->buff);
                    status = USBH_BUSY;
                    port->EnumState = HUB_ENUM_GET_HID_REPORT_DESC;
                }
    break;

    case HUB_ENUM_GET_HID_REPORT_DESC:

        	status = USBH_HUB_GetHIDReportDescriptor(phost, port->HIDDesc.wItemLength, 0U,port->buff);
                
            if (status == USBH_OK)
            {

			parse_report_descriptor(port->buff, port->HIDDesc.wItemLength, &port->HIDDesc.RptDesc);

            if (port->CfgDesc.bNumInterfaces>1)
            {
                //read second interface HID Descriptor if exist
                 status = USBH_BUSY;
                port->EnumState = HUB_ENUM_GET_HID_DESC_INTER2;
            }
            else
            {
			    port->EnumState = HUB_ENUM_READY;
            }
		}
    break;


    case HUB_ENUM_GET_HID_DESC_INTER2:

        status = USBH_HUB_GetHIDDescriptor(phost, USB_HID_DESC_SIZE, 1U,port->buff);
                if (status == USBH_OK)
                {
			        USBH_HID_ParseHIDDesc(&port->HIDDesc_Inter2, port->buff);
                    status = USBH_BUSY;
                    port->EnumState = HUB_ENUM_GET_HID_REPORT_DESC_INTER2;
                }
    break;

    case HUB_ENUM_GET_HID_REPORT_DESC_INTER2:

        	status = USBH_HUB_GetHIDReportDescriptor(phost, port->HIDDesc_Inter2.wItemLength, 1U,port->buff);
                
            if (status == USBH_OK)
            {
			parse_report_descriptor(port->buff, port->HIDDesc_Inter2.wItemLength, &port->HIDDesc_Inter2.RptDesc);
			port->EnumState = HUB_ENUM_READY;
            status = USBH_BUSY;
		}
    break;




    case HUB_ENUM_READY:
        status = USBH_OK;
    break;


   }

    return status;
}



