#include "usbh_hub.h"
#include "usbh_hubctrl.h"


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
            USBH_HUB_ParseHubDescriptor(&HUB_Handle->HUB_Desc,phost->device.Data);
            HUB_Handle->ctl_state = HUB_REQ_SET_POWER_PORT1 ;
        }

      break;

	  case HUB_REQ_SET_POWER_PORT1:

      if(USBH_HUB_SetPortFeature(phost,HUB_FEAT_SEL_PORT_POWER,1) == USBH_OK)
      {
        HUB_Handle->ctl_state = HUB_REQ_SET_POWER_PORT2 ;
      }

      break;
  
	  case HUB_REQ_SET_POWER_PORT2:

      if(USBH_HUB_SetPortFeature(phost,HUB_FEAT_SEL_PORT_POWER,2) == USBH_OK)
      {
        HUB_Handle->ctl_state = HUB_REQ_SET_POWER_PORT3 ;
      }

      break;

  	case HUB_REQ_SET_POWER_PORT3:

      if(USBH_HUB_SetPortFeature(phost,HUB_FEAT_SEL_PORT_POWER,3) == USBH_OK)
      {
        HUB_Handle->ctl_state = HUB_REQ_SET_POWER_PORT4;
      }

    break;
  
  	case HUB_REQ_SET_POWER_PORT4:
      if(USBH_HUB_SetPortFeature(phost,HUB_FEAT_SEL_PORT_POWER,4) == USBH_OK)
      {
        HUB_Handle->ctl_state = HUB_WAIT_PWRGOOD ;
      }
      break;

   	case HUB_WAIT_PWRGOOD:  
      HAL_Delay(HUB_Handle->HUB_Desc.bPwrOn2PwrGood);
      HUB_Handle->ctl_state = HUB_REQ_DONE;
      break;

    case HUB_REQ_DONE:
      status = USBH_OK;
      break;

  }

	return status;
}

static USBH_StatusTypeDef USBH_HUB_Process(USBH_HandleTypeDef *phost)
{

    USBH_StatusTypeDef status = USBH_BUSY;
    HUB_HandleTypeDef *HUB_Handle = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0];


    //USBH_HUB_GetPortStatus(phost,1);

    

  
    //USBH_HUB_GetPortStatus(phost,1);



   // USBH_HUB_GetDevDescriptor(phost);



      switch (HUB_Handle->state)
   {
    case HUB_REQ_INIT:
    USBH_HUB_SetPortFeatureBL(phost,HUB_FEAT_SEL_PORT_RESET,1);

     // phost->Control.pipe_out = USBH_AllocPipe(phost, 0x00U);
     // phost->Control.pipe_in  = USBH_AllocPipe(phost, 0x80U);
     HUB_Handle->Port1.Pipe_out = USBH_AllocPipe(phost, 0x00U);
     HUB_Handle->Port1.Pipe_in = USBH_AllocPipe(phost, 0x80U);
      phost->device.address = 0U;
      phost->Control.pipe_in = HUB_Handle->Port1.Pipe_in;
      phost->Control.pipe_out = HUB_Handle->Port1.Pipe_out;



      /* Open Control pipes */
      USBH_OpenPipe(phost, HUB_Handle->Port1.Pipe_in, 0x80U,
                    phost->device.address, phost->device.speed,
                    USBH_EP_CONTROL, (uint16_t)phost->Control.pipe_size);

      /* Open Control pipes */
      USBH_OpenPipe(phost, HUB_Handle->Port1.Pipe_out, 0x00U,
                    phost->device.address, phost->device.speed,
                    USBH_EP_CONTROL, (uint16_t)phost->Control.pipe_size);

        HUB_Handle->state = HUB_REQ_GET_DESCRIPTOR;


  break;

    case HUB_REQ_GET_DESCRIPTOR:

       USBH_HUB_Get_DevDesc(phost, 8U);
       
        phost->Control.pipe_size = HUB_Handle->Port1.DevDesc.bMaxPacketSize;



        /* modify control channels configuration for MaxPacket size */
        USBH_OpenPipe(phost, HUB_Handle->Port1.Pipe_in, 0x80U, phost->device.address,
                      phost->device.speed, USBH_EP_CONTROL,
                      (uint16_t)HUB_Handle->Port1.DevDesc.bMaxPacketSize);

        /* Open Control pipes */
        USBH_OpenPipe(phost, HUB_Handle->Port1.Pipe_out , 0x00U, phost->device.address,
                      phost->device.speed, USBH_EP_CONTROL,
                      (uint16_t)HUB_Handle->Port1.DevDesc.bMaxPacketSize);

        USBH_HUB_Get_DevDesc(phost, USB_DEVICE_DESC_SIZE);

        while (USBH_SetAddress(phost,2) != USBH_OK);

        phost->device.address = 0x02;

                /* modify control channels to update device address */
        USBH_OpenPipe(phost, HUB_Handle->Port1.Pipe_in, 0x80U,  phost->device.address,
                      phost->device.speed, USBH_EP_CONTROL,
                      (uint16_t)phost->Control.pipe_size);

        /* Open Control pipes */
        USBH_OpenPipe(phost, HUB_Handle->Port1.Pipe_out , 0x00U, phost->device.address,
                      phost->device.speed, USBH_EP_CONTROL,
                      (uint16_t)phost->Control.pipe_size);


      while(USBH_HUB_Get_CfgDesc(phost,USB_CONFIGURATION_DESC_SIZE)!=USBH_OK);

      while(USBH_HUB_Get_CfgDesc(phost,HUB_Handle->Port1.CfgDesc.wTotalLength)!=USBH_OK);



    break;


   }







	

 	return status;

   }

static USBH_StatusTypeDef USBH_HUB_SOFProcess(USBH_HandleTypeDef *phost)
{
    USBH_StatusTypeDef status = USBH_BUSY;


	

	return status;
}





