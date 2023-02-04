#include "usbh_hub_gamepad.h"


#define JOYSTICK_AXIS_MIN           0
#define JOYSTICK_AXIS_MID           127
#define JOYSTICK_AXIS_MAX           255
#define JOYSTICK_AXIS_TRIGGER_MIN   64
#define JOYSTICK_AXIS_TRIGGER_MAX   192

#define JOY_RIGHT       0x01
#define JOY_LEFT        0x02
#define JOY_DOWN        0x04
#define JOY_UP          0x08
#define JOY_BTN_SHIFT   4
#define JOY_BTN1        0x10
#define JOY_BTN2        0x20
#define JOY_BTN3        0x40
#define JOY_BTN4        0x80
#define JOY_MOVE        (JOY_RIGHT|JOY_LEFT|JOY_UP|JOY_DOWN)

static uint8_t* hub_gamepad_report_data;
HID_gamepad_Info_TypeDef    hub_gamepad_info;

static USBH_StatusTypeDef USBH_HUB_GamepadDecode(HUB_Port_Interface_HandleTypeDef *Itf);

USBH_StatusTypeDef USBH_HUB_GamepadInit(HUB_Port_Interface_HandleTypeDef *Itf)
{
  uint8_t reportSize = 0U;
  reportSize = Itf->HIDDesc->RptDesc.report_size;



  Itf->pFIFObuf = (uint8_t*) malloc (HID_QUEUE_SIZE * reportSize); 
  hub_gamepad_report_data = Itf->pData;
  USBH_HID_FifoInit(&Itf->fifo, Itf->pFIFObuf, HID_QUEUE_SIZE * reportSize);

	Itf->DeviceType = HUB_GAMEPAD;

  return USBH_OK;
}


HID_gamepad_Info_TypeDef *USBH_HUB_GetGamepadInfo(HUB_Port_Interface_HandleTypeDef *Itf)
{
	//refresh value of joymap and return value
	if(USBH_HUB_GamepadDecode(Itf)== USBH_OK)
	{
		return &hub_gamepad_info;
	}
	else
	{
		return NULL;
	}
}



static USBH_StatusTypeDef USBH_HUB_GamepadDecode(HUB_Port_Interface_HandleTypeDef *Itf)
{

	  if(Itf->length == 0U)
	  {
	    return USBH_FAIL;
	  }

	  if(USBH_HID_FifoRead(&Itf->fifo, hub_gamepad_report_data, Itf->length) ==  Itf->length)
	    {


		uint8_t jmap = 0;
		uint8_t btn = 0;
		uint8_t btn_extra = 0;
		int16_t a[2];
		uint8_t i;

		hid_report_t conf = Itf->HIDDesc->RptDesc;

		// skip report id if present
		uint8_t *p = hub_gamepad_report_data+(conf.report_id?1:0);


		//process axis
		// two axes ...
				for(i=0;i<2;i++) {
					// if logical minimum is > logical maximum then logical minimum
					// is signed. This means that the value itself is also signed
					int is_signed = conf.joystick_mouse.axis[i].logical.min >
					conf.joystick_mouse.axis[i].logical.max;
					a[i] = collect_bits(p, conf.joystick_mouse.axis[i].offset,
								conf.joystick_mouse.axis[i].size, is_signed);
				}

		//process 4 first buttons
		for(i=0;i<4;i++)
			if(p[conf.joystick_mouse.button[i].byte_offset] &
			 conf.joystick_mouse.button[i].bitmask) btn |= (1<<i);

		// ... and the eight extra buttons
		for(i=4;i<12;i++)
			if(p[conf.joystick_mouse.button[i].byte_offset] &
			 conf.joystick_mouse.button[i].bitmask) btn_extra |= (1<<(i-4));



	for(i=0;i<2;i++) {

		int hrange = (conf.joystick_mouse.axis[i].logical.max - abs(conf.joystick_mouse.axis[i].logical.min)) / 2;
		int dead = hrange/63;

		if (a[i] < conf.joystick_mouse.axis[i].logical.min) a[i] = conf.joystick_mouse.axis[i].logical.min;
		else if (a[i] > conf.joystick_mouse.axis[i].logical.max) a[i] = conf.joystick_mouse.axis[i].logical.max;

		a[i] = a[i] - (abs(conf.joystick_mouse.axis[i].logical.min) + conf.joystick_mouse.axis[i].logical.max) / 2;

		hrange -= dead;
		if (a[i] < -dead) a[i] += dead;
		else if (a[i] > dead) a[i] -= dead;
		else a[i] = 0;

		a[i] = (a[i] * 127) / hrange;

		if (a[i] < -127) a[i] = -127;
		else if (a[i] > 127) a[i] = 127;

		a[i]=a[i]+127; // mist wants a value in the range [0..255]
	}

				if(a[0] < JOYSTICK_AXIS_TRIGGER_MIN) jmap |= JOY_LEFT;
				if(a[0] > JOYSTICK_AXIS_TRIGGER_MAX) jmap |= JOY_RIGHT;
				if(a[1] < JOYSTICK_AXIS_TRIGGER_MIN) jmap |= JOY_UP;
				if(a[1] > JOYSTICK_AXIS_TRIGGER_MAX) jmap |= JOY_DOWN;
				jmap |= btn << JOY_BTN_SHIFT;      // add buttons

				hub_gamepad_info.gamepad_data = jmap;
				hub_gamepad_info.gamepad_extraBtn = btn_extra;

		  return USBH_OK;
	    }



	  return USBH_FAIL;

}
