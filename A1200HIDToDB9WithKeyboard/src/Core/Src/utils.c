#include "utils.h"
#include "usbh_hub.h"

void FifoInit(FIFO_Utils_TypeDef *f, uint8_t *buf, uint16_t size)
{
  f->head = 0U;
  f->tail = 0U;
  f->lock = 0U;
  f->size = size;
  f->buf = buf;
}


uint16_t FifoRead(FIFO_Utils_TypeDef *f, void *buf, uint16_t nbytes)
{
  uint16_t i;
  uint8_t *p;

  p = (uint8_t *) buf;

  if (f->lock == 0U)
  {
    f->lock = 1U;

    for (i = 0U; i < nbytes; i++)
    {
      if (f->tail != f->head)
      {
        *p++ = f->buf[f->tail];
        f->tail++;

        if (f->tail == f->size)
        {
          f->tail = 0U;
        }
      }
      else
      {
        f->lock = 0U;
        return i;
      }
    }
  }

  f->lock = 0U;

  return nbytes;
}

/**
  * @brief  USBH_HID_FifoWrite
  *         Write To FIFO.
  * @param  f: Fifo address
  * @param  buf: read buffer
  * @param  nbytes: number of item to write
  * @retval number of written items
  */
uint16_t FifoWrite(FIFO_Utils_TypeDef *f, void *buf, uint16_t  nbytes)
{
  uint16_t i;
  uint8_t *p;

  p = (uint8_t *) buf;

  if (f->lock == 0U)
  {
    f->lock = 1U;

    for (i = 0U; i < nbytes; i++)
    {
      if ((f->head + 1U == f->tail) ||
          ((f->head + 1U == f->size) && (f->tail == 0U)))
      {
        f->lock = 0U;
        return i;
      }
      else
      {
        f->buf[f->head] = *p++;
        f->head++;

        if (f->head == f->size)
        {
          f->head = 0U;
        }
      }
    }
  }

  f->lock = 0U;

  return nbytes;
}



void delay_us (uint16_t us)
{
	__HAL_TIM_SET_COUNTER(&htim11,0);  // set the counter value a 0
	while (__HAL_TIM_GET_COUNTER(&htim11) < us);  // wait for the counter to reach the us input in the parameter
}





//Finds first port and first interface with device data, and returns it HID_KEYBD_Info_TypeDef


uint8_t *USBH_Get_Device_Data(HUB_DEVICETypeDef deviceType)
{

  USBH_HandleTypeDef *phost = &hUsbHostFS;



  //handle device when connected to Hub
  if (phost->device.DevDesc.bDeviceClass == 9 && Appli_state == APPLICATION_READY)
  {
    HUB_HandleTypeDef *HUB_Handle  = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0]; 

    for (int port = 0; port <4; port++)
    {
      for (int interface = 0; interface <2; interface ++)
      {
        if (HUB_Handle->Port[port].Interface[interface].DeviceType == deviceType)
        {

          return (uint8_t *)USBH_HUB_GetKeybdInfo(&HUB_Handle->Port[port].Interface[interface]);
        }
      }


    }
    
  }
  else if (Appli_state == APPLICATION_READY)
  {
      for (int interface = 0; interface <2; interface ++)
      {
        HID_HandleTypeDef *HID_Handle  = (HID_HandleTypeDef *) phost->pActiveClass->pData[interface]; 
        if (HID_Handle->HID_Desc.RptDesc.type == REPORT_TYPE_KEYBOARD)
        {
          if (deviceType == HUB_KEYBOARD)
          {
            return (uint8_t *)USBH_HID_GetKeybdInfo(phost);
          }

        }

        if (HID_Handle->HID_Desc.RptDesc.type == REPORT_TYPE_MOUSE)
        {
          if (deviceType == HUB_MOUSE)
          {
            return (uint8_t *)USBH_HID_GetMouseInfo(phost);
          }

        }

        if (HID_Handle->HID_Desc.RptDesc.type == REPORT_TYPE_JOYSTICK)
        {

          if (deviceType == HUB_GAMEPAD)
          {
            return (uint8_t *)USBH_HID_GetGamepadInfo(phost);
          }
        }
        


      }

  }

return NULL;
}


