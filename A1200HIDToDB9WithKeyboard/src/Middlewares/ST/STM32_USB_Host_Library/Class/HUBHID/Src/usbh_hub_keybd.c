#include "usbh_hub_keybd.h"


HID_KEYBD_Info_TypeDef     hub_keybd_info;
uint32_t                   hub_keybd_rx_report_buf[2];
uint32_t                   hub_keybd_report_data[2];

USBH_StatusTypeDef USBH_HUB_KeybdInit(USBH_HandleTypeDef *phost,HUB_Port_HandleTypeDef *port,HUB_Port_Interface_HandleTypeDef *Itf)
{

  uint32_t x;


    hub_keybd_info.lctrl = hub_keybd_info.lshift = 0U;
    hub_keybd_info.lalt = hub_keybd_info.lgui = 0U;
    hub_keybd_info.rctrl = hub_keybd_info.rshift = 0U;
    hub_keybd_info.ralt = hub_keybd_info.rgui = 0U;


  for (x = 0U; x < (sizeof(hub_keybd_report_data) / sizeof(uint32_t)); x++)
  {
    hub_keybd_report_data[x] = 0U;
    hub_keybd_rx_report_buf[x] = 0U;
  }

  if (Itf->length > (sizeof(hub_keybd_report_data)))
  {
    Itf->length = (sizeof(hub_keybd_report_data));
  }
  //HID_Handle->pData = (uint8_t *)(void *)hub_keybd_rx_report_buf;

    Itf->pFIFObuf = malloc(HID_QUEUE_SIZE * Itf->length);
    USBH_HID_FifoInit(&Itf->fifo, Itf->pFIFObuf, HID_QUEUE_SIZE * sizeof(hub_keybd_report_data));

  return USBH_OK;

}