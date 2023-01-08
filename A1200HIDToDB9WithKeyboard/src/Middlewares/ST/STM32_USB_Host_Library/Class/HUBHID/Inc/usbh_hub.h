#ifndef __USBH_HUB_H
#define __USBH_HUB_H



#include "usbh_core.h"
#include "usbh_def.h"

#define USB_HUB_CLASS     					 0x09
#define HUB_MIN_POLL                         10U

#define MAX_HUB_PORTS 						 4


#define USB_REQUEST_GET_DESCRIPTOR           0x06

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
	HUB_REQ_SET_POWER_PORT1,
  HUB_REQ_SET_POWER_PORT2,
  HUB_REQ_SET_POWER_PORT3,
  HUB_REQ_SET_POWER_PORT4,
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
  uint8_t              HubStatus[4];
  uint8_t              PortStatus[4][4];
  uint8_t              buff[20];

}
HUB_HandleTypeDef;


typedef struct __attribute__ ((packed)) _USB_HUB_PORT_STATUS
{
    union
    {
        struct
        {
        	uint8_t     PORT_CONNECTION      : 1;
        	uint8_t     PORT_ENABLE          : 1;
        	uint8_t     PORT_SUSPEND         : 1;
        	uint8_t     PORT_OVER_CURRENT    : 1;
        	uint8_t     PORT_RESET           : 1;
        	uint8_t     RESERVED_1           : 3;
        	uint8_t     PORT_POWER           : 1;
        	uint8_t     PORT_LOW_SPEED       : 1;
        	uint8_t     PORT_HIGH_SPEED      : 1;
        	uint8_t     PORT_TEST            : 1;
        	uint8_t     PORT_INDICATOR       : 1;
        	uint8_t     RESERVED_2           : 3;
        };

        uint16_t val;

    }   wPortStatus;

    union
    {
        struct
        {
        	uint8_t     C_PORT_CONNECTION    : 1;
        	uint8_t     C_PORT_ENABLE        : 1;
        	uint8_t     C_PORT_SUSPEND       : 1;
        	uint8_t     C_PORT_OVER_CURRENT  : 1;
        	uint8_t     C_PORT_RESET         : 1;
        	uint16_t    RESERVED             : 11;
        };

        uint16_t val;

    }   wPortChange;

} USB_HUB_PORT_STATUS;



//funct
USBH_StatusTypeDef USBH_HUB_GetDescriptor(USBH_HandleTypeDef *phost);
void USBH_HUB_GetHUBStatus(USBH_HandleTypeDef *phost);
void USBH_HUB_GetPortStatus(USBH_HandleTypeDef *phost, uint8_t PortNum);
void USBH_HUB_ParseHubDescriptor(HUB_DescTypeDef  *hub_descriptor, uint8_t *buf);
void USBH_HUB_ParseHUBStatus(HUB_HandleTypeDef *HUB_Handle,uint8_t *buf);
void USBH_HUB_ParsePortStatus(HUB_HandleTypeDef *HUB_Handle,uint8_t *buf,uint8_t PortNum);
USBH_StatusTypeDef USBH_HUB_SetPortFeature(USBH_HandleTypeDef *phost, uint8_t feature, uint8_t PortNum);


#endif /* __USBH_HID_H */