#ifndef __USBH_HUB_H
#define __USBH_HUB_H



#include "usbh_core.h"
#include "usbh_def.h"
#include "usbh_hid.h"

#define USB_HUB_CLASS     					 0x09
#define HUB_MIN_POLL                         10U



extern USBH_ClassTypeDef  HUB_Class;
#define USBH_HUB_CLASS    &HUB_Class

/* States for HUB State Machine */


typedef enum
{
  HUB_INIT = 0,
  HUB_UPDATE_PORTS_STATUS,
  HUB_HANDLE_DISCONNECTED_PORTS,
  HUB_HANDLE_CONNECTED_PORTS,
  HUB_PROCESS_PORTS
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


typedef enum
{
  HUB_DEVICE_INIT = 0,
  HUB_DEVICE_GET_DATA,
  HUB_DEVICE_POLL
}
HUB_PORT_StateTypeDef;


typedef enum
{
  HUB_ENUM_INIT= 0U,
  HUB_ENUM_RESET_PORT,
  HUB_ENUM_RESET_PORT2,
  HUB_ENUM_GET_DEV_DESC,
  HUB_ENUM_GET_FULL_DEV_DESC,
  HUB_ENUM_SET_ADDR,
  HUB_ENUM_GET_CFG_DESC,
  HUB_ENUM_GET_FULL_CFG_DESC,
  HUB_ENUM_GET_MFC_STRING_DESC,
  HUB_ENUM_GET_PRODUCT_STRING_DESC,
  HUB_ENUM_GET_HID_DESC,
  HUB_ENUM_GET_HID_REPORT_DESC,
  HUB_ENUM_GET_HID_DESC_INTER2,
  HUB_ENUM_GET_HID_REPORT_DESC_INTER2,
  HUB_ENUM_SET_CONFIGURATION,
  HUB_ENUM_SET_WAKEUP_FEATURE,
  HUB_ENUM_SET_PROTOCOL,
  HUB_ENUM_SET_IDLE,
  HUB_ENUM_SET_PROTOCOL_INTER2,
  HUB_ENUM_SET_IDLE_INTER2,
  HUB_ENUM_INTERFACE_INIT,
  HUB_ENUM_INTERFACE_2_INIT,
  HUB_ENUM_READY
}
HUB_DEV_ENUMStateTypeDef;







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

typedef enum
{
  HUB_NONE = 0,
  HUB_KEYBOARD,
  HUB_MOUSE,
  HUB_GAMEPAD

} HUB_DEVICETypeDef;


typedef struct _HUB_Port_Interface_Process
{
  uint8_t                           Id;
  HUB_DEVICETypeDef                 DeviceType;
  uint8_t                           Pipe_in;
  uint8_t                           InEp;
  HUB_PORT_StateTypeDef             state;
  uint16_t                          length;
  uint8_t                           ep_addr;
  uint16_t                          poll;
  uint32_t                          timer;
  uint8_t                           DataReady;
  USBH_StatusTypeDef(* Init)(USBH_HandleTypeDef *phost);
  uint8_t                           pData[64];
  FIFO_TypeDef                      fifo;
  uint8_t                           *pFIFObuf;
  HID_DescTypeDef                   *HIDDesc;

}
HUB_Port_Interface_HandleTypeDef;

typedef struct _HUB_Port_Process
{
  USB_HUB_PORT_STATUS               PortStatus;
  uint8_t                           Connected;
  uint8_t                           Disconnected;
  HUB_DEV_ENUMStateTypeDef          EnumState;
  uint8_t                           DevDescNum;
  uint8_t                           address;
  uint8_t                           speed;
  uint8_t                           *MFC;
  uint8_t                           *Product;
  USBH_DevDescTypeDef               DevDesc;
  USBH_CfgDescTypeDef               CfgDesc;
  HID_DescTypeDef                   HIDDesc[2];
  HUB_Port_Interface_HandleTypeDef  Interface[2];
  uint8_t                           buff[512];
  uint32_t                          EnumTime;
  uint8_t                           Reenumerate;

}
HUB_Port_HandleTypeDef;




//Hub Handle


/* Structure for HUB process */
typedef struct _HUB_Process
{
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
  uint8_t              buff[256];
  HUB_Port_HandleTypeDef Port[4];
  uint8_t              PortStatusChangeFlag;
  uint8_t              portNumber;
  uint8_t              current_port_number;
  uint8_t              current_Itf_number;

}
HUB_HandleTypeDef;


void USBH_Next_Interface_Port(USBH_HandleTypeDef *phost,HUB_Port_HandleTypeDef *port);

#endif /* __USBH_HID_H */