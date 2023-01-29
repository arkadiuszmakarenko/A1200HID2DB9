#include "usbh_hub_keybd.h"


HID_KEYBD_Info_TypeDef     hub_keybd_info;
uint32_t                   hub_keybd_rx_report_buf[2];
uint32_t                   hub_keybd_report_data[2];

static const HID_Report_ItemTypedef imp_0_lctrl =
{
  (uint8_t *)(void *)hub_keybd_report_data + 0, /*data*/
  1,     /*size*/
  0,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min vale device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};
static const HID_Report_ItemTypedef imp_0_lshift =
{
  (uint8_t *)(void *)hub_keybd_report_data + 0, /*data*/
  1,     /*size*/
  1,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min vale device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};
static const HID_Report_ItemTypedef imp_0_lalt =
{
  (uint8_t *)(void *)hub_keybd_report_data + 0, /*data*/
  1,     /*size*/
  2,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min vale device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};
static const HID_Report_ItemTypedef imp_0_lgui =
{
  (uint8_t *)(void *)hub_keybd_report_data + 0, /*data*/
  1,     /*size*/
  3,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min vale device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};
static const HID_Report_ItemTypedef imp_0_rctrl =
{
  (uint8_t *)(void *)hub_keybd_report_data + 0, /*data*/
  1,     /*size*/
  4,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min vale device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};
static const HID_Report_ItemTypedef imp_0_rshift =
{
  (uint8_t *)(void *)hub_keybd_report_data + 0, /*data*/
  1,     /*size*/
  5,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min vale device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};
static const HID_Report_ItemTypedef imp_0_ralt =
{
  (uint8_t *)(void *)hub_keybd_report_data + 0, /*data*/
  1,     /*size*/
  6,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min vale device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};
static const HID_Report_ItemTypedef imp_0_rgui =
{
  (uint8_t *)(void *)hub_keybd_report_data + 0, /*data*/
  1,     /*size*/
  7,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min vale device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};

static const HID_Report_ItemTypedef imp_0_key_array =
{
  (uint8_t *)(void *)hub_keybd_report_data + 2, /*data*/
  8,     /*size*/
  0,     /*shift*/
  6,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  101,   /*max value read can return*/
  0,     /*min vale device can report*/
  101,   /*max value device can report*/
  1      /*resolution*/
};

USBH_StatusTypeDef USBH_HUB_KeybdInit(HUB_Port_Interface_HandleTypeDef *Itf)
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
  //HUB_Handle->pData = (uint8_t *)(void *)hub_keybd_rx_report_buf;

    Itf->pFIFObuf = malloc(HID_QUEUE_SIZE * Itf->length);
    USBH_HID_FifoInit(&Itf->fifo, Itf->pFIFObuf, HID_QUEUE_SIZE * sizeof(hub_keybd_report_data));
    Itf->DeviceType = HUB_KEYBOARD;

  return USBH_OK;
}


HID_KEYBD_Info_TypeDef *USBH_HUB_GetKeybdInfo(HUB_Port_Interface_HandleTypeDef *Itf)
{
  if (USBH_HUB_KeybdDecode(Itf) == USBH_OK)
  {
    return &hub_keybd_info;
  }
  else
  {
    return NULL;
  }
}



USBH_StatusTypeDef USBH_HUB_KeybdDecode(HUB_Port_Interface_HandleTypeDef *Itf)
{
  uint8_t x;


  if (Itf->length == 0U)
  {
    return USBH_FAIL;
  }
  /*Fill report */

  if (USBH_HID_FifoRead(&Itf->fifo, &hub_keybd_report_data, Itf->length) ==  Itf->length)
  {
	  if (hub_keybd_report_data[0] == 0x1010000 && hub_keybd_report_data[1]==0x1010101)
  
	  {
		  return USBH_FAIL;
	  }
	//  if keybd_re  5055
    hub_keybd_info.lctrl = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &imp_0_lctrl, 0U);
    hub_keybd_info.lshift = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &imp_0_lshift, 0U);
    hub_keybd_info.lalt = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &imp_0_lalt, 0U);
    hub_keybd_info.lgui = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &imp_0_lgui, 0U);
    hub_keybd_info.rctrl = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &imp_0_rctrl, 0U);
    hub_keybd_info.rshift = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &imp_0_rshift, 0U);
    hub_keybd_info.ralt = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &imp_0_ralt, 0U);
    hub_keybd_info.rgui = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &imp_0_rgui, 0U);

    for (x = 0U; x < sizeof(hub_keybd_info.keys); x++)
    {
      hub_keybd_info.keys[x] = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &imp_0_key_array, x);
    }

    return USBH_OK;
  }
  return   USBH_FAIL;
}