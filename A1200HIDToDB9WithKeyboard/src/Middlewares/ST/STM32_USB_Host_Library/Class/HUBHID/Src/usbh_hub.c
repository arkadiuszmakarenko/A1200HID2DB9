#include "usbh_hub.h"


static USBH_StatusTypeDef USBH_HUB_InterfaceInit(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_HUB_InterfaceDeInit(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_HUB_ClassRequest(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_HUB_Process(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_HUB_SOFProcess(USBH_HandleTypeDef *phost);

USBH_ClassTypeDef HUB_Class = 
{ 
"HUB",
USB_HUB_CLASS,
USBH_HUB_InterfaceInit, 
USBH_HUB_InterfaceDeInit,
USBH_HUB_ClassRequest, 
USBH_HUB_Process, 
USBH_HUB_SOFProcess, 
0, 0, 0, {
				0 } };




static USBH_StatusTypeDef USBH_HUB_InterfaceInit(USBH_HandleTypeDef *phost)
{
	USBH_StatusTypeDef status = USBH_BUSY;
	HUB_HandleTypeDef *HUB_Handle;
    uint8_t max_ep;
    uint8_t num = 0U;
    uint8_t interface;


    interface = USBH_FindInterface(phost, phost->pActiveClass->ClassCode, 0x00U, 0x00U);

    if ((interface == 0xFFU) || (interface >= USBH_MAX_NUM_INTERFACES)) /* No Valid Interface */
    {
        USBH_DbgLog("Cannot Find the interface for %s class.", phost->pActiveClass->Name);
        return USBH_FAIL;
    }

    status = USBH_SelectInterface(phost, interface);

    if (status != USBH_OK)
    {
        return USBH_FAIL;
    }


    phost->pActiveClass->pData[0] = (HUB_HandleTypeDef *)USBH_malloc(sizeof(HUB_HandleTypeDef));
    HUB_Handle = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0];
    
    if (HUB_Handle == NULL)
    {
        USBH_DbgLog("Cannot allocate memory for HID Handle");
        return USBH_FAIL;
    }

      /* Initialize hid handler */
    USBH_memset(HUB_Handle, 0, sizeof(HUB_HandleTypeDef));




  HUB_Handle->state     = HUB_INIT;
  HUB_Handle->ctl_state = HUB_REQ_INIT;
  HUB_Handle->ep_addr   = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
  HUB_Handle->length    = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
  HUB_Handle->poll      = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bInterval;

  if (HUB_Handle->poll  < HUB_MIN_POLL)
  {
    HUB_Handle->poll = HUB_MIN_POLL;
  }

  /* Check fo available number of endpoints */
  /* Find the number of EPs in the Interface Descriptor */
  /* Choose the lower number in order not to overrun the buffer allocated */
  max_ep = ((phost->device.CfgDesc.Itf_Desc[interface].bNumEndpoints <= USBH_MAX_NUM_ENDPOINTS) ?
             phost->device.CfgDesc.Itf_Desc[interface].bNumEndpoints : USBH_MAX_NUM_ENDPOINTS);


  /* Decode endpoint IN and OUT address from interface descriptor */
  for (num = 0U; num < max_ep; num++)
  {
    if (phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[num].bEndpointAddress & 0x80U)
    {
      HUB_Handle->InEp = (phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[num].bEndpointAddress);
      HUB_Handle->InPipe = USBH_AllocPipe(phost, HUB_Handle->InEp);

      /* Open pipe for IN endpoint */
      USBH_OpenPipe(phost, HUB_Handle->InPipe, HUB_Handle->InEp, phost->device.address,
                    phost->device.speed, USB_EP_TYPE_INTR, HUB_Handle->length);

      USBH_LL_SetToggle(phost, HUB_Handle->InPipe, 0U);
    }
    else
    {
      HUB_Handle->OutEp = (phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[num].bEndpointAddress);
      HUB_Handle->OutPipe  = USBH_AllocPipe(phost, HUB_Handle->OutEp);

      /* Open pipe for OUT endpoint */
      USBH_OpenPipe(phost, HUB_Handle->OutPipe, HUB_Handle->OutEp, phost->device.address,
                    phost->device.speed, USB_EP_TYPE_INTR, HUB_Handle->length);

      USBH_LL_SetToggle(phost, HUB_Handle->OutPipe, 0U);
    }
  }

	return status;
}

static USBH_StatusTypeDef USBH_HUB_InterfaceDeInit(USBH_HandleTypeDef *phost)
{
  HUB_HandleTypeDef *HUB_Handle = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0];

  if (HUB_Handle->InPipe != 0x00U)
  {
    USBH_ClosePipe(phost, HUB_Handle->InPipe);
    USBH_FreePipe(phost, HUB_Handle->InPipe);
    HUB_Handle->InPipe = 0U;     /* Reset the pipe as Free */
  }

  if (HUB_Handle->OutPipe != 0x00U)
  {
    USBH_ClosePipe(phost, HUB_Handle->OutPipe);
    USBH_FreePipe(phost, HUB_Handle->OutPipe);
    HUB_Handle->OutPipe = 0U;     /* Reset the pipe as Free */
  }

  if (phost->pActiveClass->pData[0])
  {
    USBH_free(phost->pActiveClass->pData[0]);
    phost->pActiveClass->pData[0] = 0U;
  }

  return USBH_OK;
}
static USBH_StatusTypeDef USBH_HUB_ClassRequest(USBH_HandleTypeDef *phost)
{
    USBH_StatusTypeDef status = USBH_BUSY;
    HUB_HandleTypeDef *HUB_Handle = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0];


   switch (HUB_Handle->ctl_state)
   {
    case HUB_REQ_INIT:
    case HUB_REQ_GET_DESCRIPTOR:

        if (USBH_HUB_GetDescriptor(phost) == USBH_OK)
        {
                //we are in. parse descriptor

            HUB_Handle->ctl_state =  HUB_REQ_SET_POWER ;
        }
    break;

	case HUB_REQ_SET_POWER:

    default:
    break;

  }




	
	return status;
}

static USBH_StatusTypeDef USBH_HUB_Process(USBH_HandleTypeDef *phost)
{
    USBH_StatusTypeDef status = USBH_BUSY;


	

	return status;
}

static USBH_StatusTypeDef USBH_HUB_SOFProcess(USBH_HandleTypeDef *phost)
{
    USBH_StatusTypeDef status = USBH_BUSY;


	

	return status;
}



USBH_StatusTypeDef USBH_HUB_GetDescriptor(USBH_HandleTypeDef *phost)
{

     // USBH_StatusTypeDef status;
    HUB_HandleTypeDef *HUB_Handle = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0];
      uint16_t lenght = sizeof(HUB_DescTypeDef);

  if (phost->RequestState == CMD_SEND)
  {
    phost->Control.setup.b.bmRequestType = 0b10100000;
    phost->Control.setup.b.bRequest = USB_REQ_GET_DESCRIPTOR;		
    phost->Control.setup.b.wValue.bw.msb = 0;
	phost->Control.setup.b.wValue.bw.lsb = 0x29;
    phost->Control.setup.b.wIndex.w = 0;
    phost->Control.setup.b.wLength.w = lenght;
  }

  return USBH_CtlReq(phost, HUB_Handle->buffer, lenght);


}