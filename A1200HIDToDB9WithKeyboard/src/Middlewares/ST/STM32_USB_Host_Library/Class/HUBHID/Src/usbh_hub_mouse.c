#include "usbh_hub_mouse.h"
#include "usbh_hub.h"

HID_MOUSE_Info_TypeDef    hub_mouse_info;
uint8_t                 hub_mouse_report_data[8];
uint32_t                hub_mouse_rx_report_buf[2];


USBH_StatusTypeDef USBH_HUB_MouseInit(USBH_HandleTypeDef *phost,HUB_Port_HandleTypeDef *port,HUB_Port_Interface_HandleTypeDef *Itf)
{
  uint32_t i;
  uint8_t Itf_num = Itf->Id;

  hub_mouse_info.x = 0U;
  hub_mouse_info.y = 0U;
  hub_mouse_info.buttons[0] = 0U;
  hub_mouse_info.buttons[1] = 0U;
  hub_mouse_info.buttons[2] = 0U;

  for (i = 0U; i < (sizeof(hub_mouse_report_data) / sizeof(uint32_t)); i++)
  {
    hub_mouse_report_data[i] = 0U;
    hub_mouse_rx_report_buf[i] = 0U;
  }

  Itf->length = port->HIDDesc[Itf_num].RptDesc.report_size + (port->HIDDesc[Itf_num].RptDesc.report_id?1:0);
  Itf->pFIFObuf = malloc(HID_QUEUE_SIZE * Itf->length);
  USBH_HID_FifoInit(&Itf->fifo, Itf->pFIFObuf, HID_QUEUE_SIZE * Itf->length);

  return USBH_OK;
}


HID_MOUSE_Info_TypeDef *USBH_HUB_GetMouseInfo(USBH_HandleTypeDef *phost)
{
  if (USBH_HUB_MouseDecode(phost) == USBH_OK)
  {
    return &hub_mouse_info;
  }
  else
  {
    return NULL;
  }
}


USBH_StatusTypeDef USBH_HUB_MouseDecode(USBH_HandleTypeDef *phost)
{
  HID_HandleTypeDef *HID_Handle =  (HID_HandleTypeDef *) phost->pActiveClass->pData[phost->device.current_interface];

  if (HID_Handle->length == 0U)
  {
    return USBH_FAIL;
  }

  //Clear mouse_report_data

  memset(&hub_mouse_report_data,0,sizeof(hub_mouse_report_data));


  /*Fill report */
  if (USBH_HID_FifoRead(&HID_Handle->fifo, &hub_mouse_report_data, HID_Handle->length) !=0)
  {

	  uint8_t btn = 0;
	  uint8_t btn_extra = 0;
	  int16_t a[2];
	  uint8_t i;



	  // skip report id if present
	  uint8_t *p = hub_mouse_report_data + (HID_Handle->HID_Desc.RptDesc.report_id?1:0);


	  //process axis
	  // two axes ...
	  		for(i=0;i<2;i++) {
	  			// if logical minimum is > logical maximum then logical minimum
	  			// is signed. This means that the value itself is also signed
	  			int is_signed = HID_Handle->HID_Desc.RptDesc.joystick_mouse.axis[i].logical.min >
	  				HID_Handle->HID_Desc.RptDesc.joystick_mouse.axis[i].logical.max;
	  			a[i] = collect_bits(p, HID_Handle->HID_Desc.RptDesc.joystick_mouse.axis[i].offset,
	  					HID_Handle->HID_Desc.RptDesc.joystick_mouse.axis[i].size, is_signed);
	  		}

	  //process 4 first buttons
	  for(i=0;i<4;i++)
	  	if(p[HID_Handle->HID_Desc.RptDesc.joystick_mouse.button[i].byte_offset] &
	  			HID_Handle->HID_Desc.RptDesc.joystick_mouse.button[i].bitmask) btn |= (1<<i);

	  // ... and the eight extra buttons
	  for(i=4;i<12;i++)
	  	if(p[HID_Handle->HID_Desc.RptDesc.joystick_mouse.button[i].byte_offset] &
	  			HID_Handle->HID_Desc.RptDesc.joystick_mouse.button[i].bitmask) btn_extra |= (1<<(i-4));

	  //process mouse
	  if(HID_Handle->HID_Desc.RptDesc.type == REPORT_TYPE_MOUSE) {
	  		// iprintf("mouse %d %d %x\n", (int16_t)a[0], (int16_t)a[1], btn);
	  		// limit mouse movement to +/- 128
	  		for(i=0;i<2;i++) {
	  		if((int16_t)a[i] >  127) a[i] =  127;
	  		if((int16_t)a[i] < -128) a[i] = -128;
	  		}
	  		//btn
	  	  hub_mouse_info.x = a[0];
	  	  hub_mouse_info.y = a[1];
	  	  hub_mouse_info.buttons[0] = btn&0x1;
	  	  hub_mouse_info.buttons[1] = (btn>>1)&0x1;
	  	  hub_mouse_info.buttons[2] = (btn>>2)&0x1;
	  	}
    return USBH_OK;
  }
  return   USBH_FAIL;
}