#include "joystick.h"

void ProcessJoystick() {

			HID_gamepad_Info_TypeDef *joymap = (HID_gamepad_Info_TypeDef *)USBH_Get_Device_Data(HUB_GAMEPAD);


			if (joymap == NULL) return;

				HAL_GPIO_WritePin(RHQ_GPIO_Port, RHQ_Pin, !(joymap->gamepad_data & 0x1));
				HAL_GPIO_WritePin(LVQ_GPIO_Port, LVQ_Pin, !(joymap->gamepad_data >> 1 & 0x1));
				HAL_GPIO_WritePin(BH_GPIO_Port, BH_Pin, !(joymap->gamepad_data >> 2 & 0x1));
				HAL_GPIO_WritePin(FV_GPIO_Port, FV_Pin, !(joymap->gamepad_data >> 3 & 0x1));
				HAL_GPIO_WritePin(LB_GPIO_Port, LB_Pin, !(joymap->gamepad_data >> 4 & 0x1));
				HAL_GPIO_WritePin(MB_GPIO_Port, RB_Pin, !(joymap->gamepad_data >> 5 & 0x1));
				HAL_GPIO_WritePin(RB_GPIO_Port, MB_Pin, !(joymap->gamepad_data >> 6 & 0x1));
	
}

