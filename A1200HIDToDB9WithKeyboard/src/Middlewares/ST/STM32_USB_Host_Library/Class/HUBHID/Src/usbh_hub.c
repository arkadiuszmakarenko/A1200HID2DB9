#include "usbh_hub.h"
#include "usbh_hubctrl.h"
#include "usbh_hubenum.h"


static USBH_StatusTypeDef USBH_HUB_InterfaceInit(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_HUB_InterfaceDeInit(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_HUB_ClassRequest(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_HUB_Process(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_HUB_SOFProcess(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_HUB_UpdatePortsStatus(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_HUB_DisconnectDevice(USBH_HandleTypeDef *phost, HUB_Port_HandleTypeDef *Port);

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

      /* Initialize hub handler */
    USBH_memset(HUB_Handle, 0, sizeof(HUB_HandleTypeDef));




  HUB_Handle->state     = HUB_INIT;
  HUB_Handle->ctl_state = HUB_REQ_INIT;
  HUB_Handle->ep_addr   = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
  HUB_Handle->length    = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
  HUB_Handle->InEp      = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
  HUB_Handle->poll      = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bInterval;
  HUB_Handle->portNumber= 1;

	return status;
}

static USBH_StatusTypeDef USBH_HUB_InterfaceDeInit(USBH_HandleTypeDef *phost)
{
  HUB_HandleTypeDef *HUB_Handle = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0];


  for (int i = 0; i<4; i++)
  {
    if(HUB_Handle->Port[i].MFC!=0) 
    {
      free(HUB_Handle->Port[i].MFC);
      HUB_Handle->Port[i].MFC = 0;
    }
    if(HUB_Handle->Port[i].Product!=0) 
    {
      free(HUB_Handle->Port[i].Product);
      HUB_Handle->Port[i].Product = 0;
    }
  }

  if (phost->pActiveClass->pData[0])
  {
    USBH_free(phost->pActiveClass->pData[0]);
    phost->pActiveClass->pData[0] = 0U;
  }


  USBH_memset(HUB_Handle, 0, sizeof(HUB_HandleTypeDef));


  return USBH_OK;
}
static USBH_StatusTypeDef USBH_HUB_ClassRequest(USBH_HandleTypeDef *phost)
{
    USBH_StatusTypeDef status = USBH_BUSY;
    HUB_HandleTypeDef *HUB_Handle = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0];


   switch (HUB_Handle->ctl_state)
   {
    case HUB_REQ_INIT:
    phost->pUser(phost, HOST_USER_CLASS_ACTIVE);
    HUB_Handle->ctl_state = HUB_REQ_GET_DESCRIPTOR; 
    break;
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
      phost->Control.pipe_size = phost->device.DevDesc.bMaxPacketSize;
      HUB_Handle->current_Itf_number = 0;
      HUB_Handle->current_port_number = 0;
      status = USBH_OK;
      break;

  }

	return status;
}

static USBH_StatusTypeDef USBH_HUB_Process(USBH_HandleTypeDef *phost)
{
    USBH_StatusTypeDef status = USBH_OK;
    HUB_HandleTypeDef *HUB_Handle = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0];
    

    switch (HUB_Handle->state)
    {
    case HUB_INIT:
            USBH_OpenPipe(phost, phost->Control.pipe_in, 0x80,
            phost->device.address, phost->device.speed,
            USBH_EP_CONTROL, (uint16_t)0x40);

            USBH_OpenPipe(phost, phost->Control.pipe_out, 0x00U,
            phost->device.address, phost->device.speed,
            USBH_EP_CONTROL, (uint16_t)0x40);

            phost->Control.pipe_size = 0x40;
            phost->Control.length = HUB_Handle->length;

      HUB_Handle->state = HUB_UPDATE_PORTS_STATUS;
      break;

    case HUB_UPDATE_PORTS_STATUS:
          status = USBH_HUB_UpdatePortsStatus(phost);
          if (status == USBH_OK)
          {
            HUB_Handle->state = HUB_HANDLE_DISCONNECTED_PORTS;
          }


      break;
    
    case HUB_HANDLE_DISCONNECTED_PORTS:

      if (HUB_Handle->Port[HUB_Handle->portNumber-1].Disconnected)
      {
        // Handle Disconnection
        //DeInit HID, Free Pipes and clear Handles.
        status = USBH_HUB_DisconnectDevice(phost,&HUB_Handle->Port[HUB_Handle->portNumber-1]);
        if (status == USBH_OK)
        {
          HUB_Handle->Port[HUB_Handle->portNumber-1].Disconnected = 0;
        }
      }
      else 
      {
        if (HUB_Handle->portNumber<4)
        { 
        HUB_Handle->portNumber++;
        }
        else
        {
            HUB_Handle->portNumber = 1;
            HUB_Handle->state = HUB_HANDLE_CONNECTED_PORTS;
        }
      }
      break;

    case HUB_HANDLE_CONNECTED_PORTS:

      if (HUB_Handle->Port[HUB_Handle->portNumber-1].Connected)
      {
        // Handle Connection / Enumeration
        status = USBH_HUB_Device_Enum(phost,&HUB_Handle->Port[HUB_Handle->portNumber-1]);
        if (status==USBH_OK)
        {
          HUB_Handle->Port[HUB_Handle->portNumber-1].Connected = 0;
        }
      }
      else 
      {
        if (HUB_Handle->portNumber<4)
       { 
          HUB_Handle->portNumber++;
        }
        else
        {
            HUB_Handle->portNumber = 1;
            HUB_Handle->state = HUB_PROCESS_PORTS;
        }
      }
    break;


    case HUB_PROCESS_PORTS:
      //handle all ports and interfaces at once.
      USBH_HUB_Device_Process(phost);



      HUB_Handle->state = HUB_INIT;
      break;

    default:
        status = USBH_FAIL;
      break;
    } 
    


 	return status;

   }

static USBH_StatusTypeDef USBH_HUB_SOFProcess(USBH_HandleTypeDef *phost)
{
    USBH_StatusTypeDef status = USBH_OK;
    HUB_HandleTypeDef *HUB_Handle  = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0]; 
    uint8_t currentPort = HUB_Handle->current_port_number;
    uint8_t currentItf  = HUB_Handle->current_Itf_number;

    HUB_Port_HandleTypeDef *port = (HUB_Port_HandleTypeDef *) &HUB_Handle->Port[currentPort];
    if (port->EnumState != HUB_ENUM_READY) return status;

    HUB_Port_Interface_HandleTypeDef *Itf = (HUB_Port_Interface_HandleTypeDef *) &port->Interface[currentItf];

    if (Itf->state == HUB_DEVICE_POLL)
    {
        if(phost->Timer - Itf->timer > Itf->poll)
        {
          Itf->state = HUB_DEVICE_GET_DATA;


          USBH_Next_Interface_Port(phost,port);


        }
    }

	  return status;
}

static USBH_StatusTypeDef USBH_HUB_UpdatePortsStatus(USBH_HandleTypeDef *phost)
{
  static uint8_t PortNumber = 1;
  USBH_StatusTypeDef status = USBH_BUSY;

    status = USBH_HUB_GetPortStatus(phost,PortNumber);
    if ( status == USBH_OK)
    {
      if (PortNumber<4) 
      {
        PortNumber++;
        status = USBH_BUSY;
      }  
      else
      {
        PortNumber = 1;
      } 
      
    }
	return status;
}



static USBH_StatusTypeDef USBH_HUB_DisconnectDevice(USBH_HandleTypeDef *phost, HUB_Port_HandleTypeDef *Port)
{
    USBH_StatusTypeDef status = USBH_OK;

      free(Port->MFC);
      Port->MFC = 0;
      free(Port->Product);
      Port->Product = 0;
      if (Port->Interface[0].Pipe_in != 0 ) USBH_FreePipe(phost, Port->Interface[0].Pipe_in);
      if (Port->Interface[1].Pipe_in != 0 ) USBH_FreePipe(phost, Port->Interface[0].Pipe_in);

      USBH_memset(Port, 0, sizeof(HUB_Port_HandleTypeDef));
      
  return status;
}

void USBH_Next_Interface_Port(USBH_HandleTypeDef *phost,HUB_Port_HandleTypeDef *port)
{
    HUB_HandleTypeDef *HUB_Handle  = (HUB_HandleTypeDef *) phost->pActiveClass->pData[0]; 

          //CHECK IF NEXT INTERFACE NEED TO BE HANDLED
          if(port->CfgDesc.bNumInterfaces>1)
          {
             //NEXT INTERFACE
              if(HUB_Handle->current_Itf_number == 0)
              {
                HUB_Handle->current_Itf_number = 1;
              }
              else
              {
                //ALREADY DONE BOTH INTERFACES RESET INTERFACE AND SELECT NEXT PORT
                HUB_Handle->current_Itf_number = 0;

                if(HUB_Handle->current_port_number ==3)
                {
                  HUB_Handle->current_port_number = 0;
                }
                else
                {
                  HUB_Handle->current_port_number++;
                }
              }
          } else
          {
            //NO MORE INTERFACES SELECT NEXT PORT
                if(HUB_Handle->current_port_number ==3)
                {
                  HUB_Handle->current_port_number = 0;
                }
                else
                {
                  HUB_Handle->current_port_number++;
                }
          }
}


