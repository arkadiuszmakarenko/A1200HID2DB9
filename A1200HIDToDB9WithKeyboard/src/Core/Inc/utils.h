#include "main.h"
#include "usb_host.h"
#include "usbh_hid.h"
#include "usbh_hub_keybd.h"



extern TIM_HandleTypeDef htim11;

typedef struct
{
  uint8_t  *buf;
  uint16_t  head;
  uint16_t tail;
  uint16_t size;
  uint8_t  lock;
} FIFO_Utils_TypeDef;


uint16_t FifoWrite(FIFO_Utils_TypeDef *f, void *buf, uint16_t  nbytes);
uint16_t FifoRead(FIFO_Utils_TypeDef *f, void *buf, uint16_t nbytes);
void FifoInit(FIFO_Utils_TypeDef *f, uint8_t *buf, uint16_t size);
void delay_us (uint16_t us);
HID_KEYBD_Info_TypeDef *USBH_Get_Keyboard_Data();


extern USBH_HandleTypeDef hUsbHostFS;
extern ApplicationTypeDef Appli_state;