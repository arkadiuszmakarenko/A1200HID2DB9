#include "usbh_hub_gamepad.h"


static uint8_t* gamepad_report_data;


USBH_StatusTypeDef USBH_HUB_GamepadInit(USBH_HandleTypeDef *phost)
{
  HID_HandleTypeDef *HID_Handle =  (HID_HandleTypeDef *) phost->pActiveClass->pData[0];
  uint8_t reportSize = 0U;
  reportSize = HID_Handle->HID_Desc.RptDesc.report_size;





  HID_Handle->length = reportSize;


  HID_Handle->pData = (uint8_t*) malloc (reportSize *sizeof(uint8_t)); //(uint8_t*)(void *)
  gamepad_report_data = HID_Handle->pData;
  USBH_HID_FifoInit(&HID_Handle->fifo, phost->device.Data, HID_QUEUE_SIZE * reportSize);

  return USBH_OK;
}