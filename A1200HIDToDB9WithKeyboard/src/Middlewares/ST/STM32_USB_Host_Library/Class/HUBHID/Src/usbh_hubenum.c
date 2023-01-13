#include "usbh_hubenum.h"
#include "usbh_hubctrl.h"



USBH_StatusTypeDef USBH_HUB_Device_Enum(USBH_HandleTypeDef *phost, HUB_Port_HandleTypeDef *port)
{


    switch (port->EnumState)
   {
    case HUB_ENUM_IDLE:
    case HUB_ENUM_RESET_PORT:
    USBH_HUB_SetPortFeature(phost,HUB_FEAT_SEL_PORT_RESET,1);
    break;

    case HUB_ENUM_GET_DEV_DESC:

     // phost->Control.pipe_out = USBH_AllocPipe(phost, 0x00U);
     // phost->Control.pipe_in  = USBH_AllocPipe(phost, 0x80U);
     //HUB_Handle->Port1.Pipe_out = USBH_AllocPipe(phost, 0x00U);
     //HUB_Handle->Port1.Pipe_in = USBH_AllocPipe(phost, 0x80U);
     // phost->device.address = 0U;
     // phost->Control.pipe_in = HUB_Handle->Port1.Pipe_in;
     // phost->Control.pipe_out = HUB_Handle->Port1.Pipe_out;




      /* 
      USBH_OpenPipe(phost, HUB_Handle->Port1.Pipe_in, 0x80U,
                    phost->device.address, phost->device.speed,
                    USBH_EP_CONTROL, (uint16_t)phost->Control.pipe_size);

      /* Open Control pipes 
      USBH_OpenPipe(phost, HUB_Handle->Port1.Pipe_out, 0x00U,
                    phost->device.address, phost->device.speed,
                    USBH_EP_CONTROL, (uint16_t)phost->Control.pipe_size);
*/



      // USBH_HUB_Get_DevDesc(phost, 8U);




    break;

    case HUB_ENUM_GET_FULL_DEV_DESC:
           
        //phost->Control.pipe_size = HUB_Handle->Port1.DevDesc.bMaxPacketSize;



        /* modify control channels configuration for MaxPacket size */
        /* USBH_OpenPipe(phost, HUB_Handle->Port1.Pipe_in, 0x80U, phost->device.address,
                      phost->device.speed, USBH_EP_CONTROL,
                      (uint16_t)HUB_Handle->Port1.DevDesc.bMaxPacketSize);

        /* Open Control pipes */
        /* USBH_OpenPipe(phost, HUB_Handle->Port1.Pipe_out , 0x00U, phost->device.address,
                      phost->device.speed, USBH_EP_CONTROL,
                      (uint16_t)HUB_Handle->Port1.DevDesc.bMaxPacketSize);

        USBH_HUB_Get_DevDesc(phost, USB_DEVICE_DESC_SIZE);
    break;

    case HUB_ENUM_SET_ADDR:
            while (USBH_SetAddress(phost,2) != USBH_OK);

        phost->device.address = 0x02;

                /* modify control channels to update device address */
        /* USBH_OpenPipe(phost, HUB_Handle->Port1.Pipe_in, 0x80U,  phost->device.address,
                      phost->device.speed, USBH_EP_CONTROL,
                      (uint16_t)phost->Control.pipe_size);

        /* Open Control pipes */
        /* USBH_OpenPipe(phost, HUB_Handle->Port1.Pipe_out , 0x00U, phost->device.address,
                      phost->device.speed, USBH_EP_CONTROL,
                      (uint16_t)phost->Control.pipe_size);*/
    break;

    case HUB_ENUM_GET_CFG_DESC:
         // while(USBH_HUB_Get_CfgDesc(phost,USB_CONFIGURATION_DESC_SIZE)!=USBH_OK);
    break;

    case HUB_ENUM_GET_FULL_CFG_DESC:

    //  while(USBH_HUB_Get_CfgDesc(phost,HUB_Handle->Port1.CfgDesc.wTotalLength)!=USBH_OK);

    break;

    case HUB_ENUM_GET_MFC_STRING_DESC:
    break;

    case HUB_ENUM_GET_PRODUCT_STRING_DESC:
    break;

    case HUB_ENUM_GET_SERIALNUM_STRING_DESC:
    break;


   }


    return 1;
}



