#ifndef __USBH_HUB_H
#define __USBH_HUB_H



#include "usbh_core.h"
#include "usbh_def.h"

#define USB_HUB_CLASS     					 0x09
#define HUB_MIN_POLL                         10U

#define MAX_HUB_PORTS 						 4


#define USB_REQUEST_GET_DESCRIPTOR           0x06
#define HUB_FEATURE_SEL_PORT_POWER           0x08

#define USB_DEVICE_REQUEST_SET   			 0x00
#define USB_DEVICE_REQUEST_GET   			 0x01
#define USB_REQUEST_CLEAR_FEATURE   		 0x01
#define USB_REQUEST_SET_FEATURE     		 0x03
#define USB_REQUEST_GET_STATUS          	 0x00

#define HUB_FEAT_SEL_PORT_CONNECTION 		 0x00
#define HUB_FEAT_SEL_C_HUB_LOCAL_POWER       0x00
#define HUB_FEAT_SEL_C_HUB_OVER_CURRENT      0x01

#define HUB_FEAT_SEL_PORT_CONN         		 0x00
#define HUB_FEAT_SEL_PORT_ENABLE             0x01
#define HUB_FEAT_SEL_PORT_SUSPEND            0x02
#define HUB_FEAT_SEL_PORT_OVER_CURRENT       0x03
#define HUB_FEAT_SEL_PORT_RESET              0x04
#define HUB_FEAT_SEL_PORT_POWER              0x08
#define HUB_FEAT_SEL_PORT_LOW_SPEED          0x09
#define HUB_FEAT_SEL_C_PORT_CONNECTION       0x10
#define HUB_FEAT_SEL_C_PORT_ENABLE           0x11
#define HUB_FEAT_SEL_C_PORT_SUSPEND          0x12
#define HUB_FEAT_SEL_C_PORT_OVER_CURRENT     0x13
#define HUB_FEAT_SEL_C_PORT_RESET            0x14
#define HUB_FEAT_SEL_PORT_INDICATOR          0x16


extern USBH_ClassTypeDef  HUB_Class;
#define USBH_HUB_CLASS    &HUB_Class

//funct
USBH_StatusTypeDef USBH_HUB_GetDescriptor(USBH_HandleTypeDef *phost);



/* States for HUB State Machine */


typedef enum
{
  HUB_INIT = 0,
  HUB_IDLE,

}
HUB_StateTypeDef;

typedef enum
{
	HUB_REQ_INIT = 0,
	HUB_REQ_GET_DESCRIPTOR,
	HUB_REQ_SET_POWER,
	HUB_WAIT_PWRGOOD,
	HUB_REQ_DONE,
}
HUB_CtlStateTypeDef;







typedef struct _HUBDescriptor
{
  uint8_t   bDescLength;                /*Number of bytes in this descriptor, including this byte*/
  uint8_t   bDescriptorType;            /*Descriptor Type, value: 29H for hub descriptor*/
  uint8_t   bNbrPorts;                  /* Number of downstream facing ports that this hub supports*/
  uint16_t  wHubCharacteristics;        /* Hub Characteristics */
  uint8_t   bPwrOn2PwrGood;             /* Time (in 2 ms intervals) from the time the power-on sequence begins on a port until power is good on that port.*/
  uint8_t   bHubContrCurrent;           /* Maximum current requirements of the Hub Controller electronics in mA. */
  uint8_t  DeviceRemovable;            /* Indicates if a port has a removable device attached.*/
  uint8_t  PortPwrCtrlMask;            /* This field exists for reasons of compatibility with software written for 1.0 compliant devices.*/
}
HUB_DescTypeDef;







//Hub Handle


/* Structure for HUB process */
typedef struct _HUB_Process
{
  uint8_t              OutPipe;
  uint8_t              InPipe;
  HUB_StateTypeDef     state;
  uint8_t              OutEp;
  uint8_t              InEp;
  HUB_CtlStateTypeDef  ctl_state;
  uint8_t              *pData;
  uint16_t             length;
  uint8_t              ep_addr;
  uint16_t             poll;
  uint32_t             timer;
  uint8_t              DataReady;
  HUB_DescTypeDef      HUB_Desc;
  USBH_StatusTypeDef(* Init)(USBH_HandleTypeDef *phost);
  uint8_t              buffer[20];

}
HUB_HandleTypeDef;









#endif /* __USBH_HID_H */