#include "mouse.h"

ApplicationTypeDef aState;

uint8_t processMouseMovement(int8_t movementUnits, uint8_t axis, int limitRate,
		int dpiDivide) {
	uint16_t timerTopValue = 0;

	// Set the mouse movement direction and record the movement units
	if (movementUnits > 0) {
		// Moving in the positive direction

		// Apply DPI limiting if required
		if (dpiDivide) {
			movementUnits /= DPI_DIVIDER;
			if (movementUnits < 1)
				movementUnits = 1;
		}

		// Add the movement units to the quadrature output buffer
		if (axis == MOUSEX)
			mouseDistanceX += movementUnits;
		else
			mouseDistanceY += movementUnits;
	} else if (movementUnits < 0) {
		// Moving in the negative direction

		// Apply DPI limiting if required
		if (dpiDivide) {
			movementUnits /= DPI_DIVIDER;
			if (movementUnits > -1)
				movementUnits = -1;
		}

		// Add the movement units to the quadrature output buffer
		if (axis == MOUSEX)
			mouseDistanceX += -movementUnits;
		else
			mouseDistanceY += -movementUnits;
	} else {
		if (axis == MOUSEX)
			mouseDistanceX = 0;
		else
			mouseDistanceY = 0;
	}

	// Apply the quadrature output buffer limit
	if (axis == MOUSEX) {
		if (mouseDistanceX > Q_BUFFERLIMIT)
			mouseDistanceX = Q_BUFFERLIMIT;
	} else {
		if (mouseDistanceY > Q_BUFFERLIMIT)
			mouseDistanceY = Q_BUFFERLIMIT;
	}

	// Get the current value of the quadrature output buffer
	if (axis == MOUSEX)
		timerTopValue = mouseDistanceX;
	else
		timerTopValue = mouseDistanceY;

	// Range check the quadrature output buffer
	if (timerTopValue > 127)
		timerTopValue = 127;

	// Since the USB reports arrive at 100-125 Hz (even if there is only
	// a small amount of movement, we have to output the quadrature
	// at minimum rate to keep up with the reports (otherwise it creates
	// a slow lag).  If we assume 100 Hz of reports then the following
	// is true:
	//
	// 127 movements = 12,700 interrupts/sec
	// 100 movements = 10,000 interrupts/sec
	//  50 movements =  5,000 interrupts/sec
	//  10 movements =  1,000 interrupts/sec
	//   1 movement  =    100 interrupts/sec
	//
	// Timer speed is 15,625 ticks per second = 64 uS per tick
	//
	// Required timer TOP values (0 is fastest so all results are x-1):
	// 1,000,000 / 12,700 = 78.74 / 64 uS = 1.2 - 1
	// 1,000,000 / 10,000 = 100 / 64 uS = 1.56 - 1
	// 1,000,000 / 5,000 = 200 / 64 uS = 3.125 - 1
	// 1,000,000 / 1,000 = 1000 uS / 64 uS = 15.63 - 1
	// 1,000,000 / 100 = 10000 uS / 64 uS = 156.25 - 1
	//
	// So:
	//   timerTopValue = 10000 / timerTopValue; // i.e. 1,000,000 / (timerTopValue * 100)
	//   timerTopValue = timerTopValue / 64;
	//   timerTopValue = timerTopValue - 1;
	if (timerTopValue != 0) {
		timerTopValue = ((10000 / timerTopValue) / 64) - 1;
	} else {
		timerTopValue = 255;
	}
	// If the 'Slow' configuration jumper is shorted; apply the quadrature rate limit
	if (limitRate) {
		// Rate limit is on

		// Rate limit is provided in hertz
		// Each timer tick is 64 uS
		//
		// Convert hertz into period in uS
		// 1500 Hz = 1,000,000 / 1500 = 666.67 uS
		//
		// Convert period into timer ticks (* 4 due to quadrature)
		// 666.67 us / (64 * 4) = 2.6 ticks
		//
		// Timer TOP is 0-255, so subtract 1
		// 10.42 ticks - 1 = 9.42 ticks

		uint32_t rateLimit = ((1000000 / Q_RATELIMIT) / 256) - 1;

		// If the timerTopValue is less than the rate limit, we output
		// at the maximum allowed rate.  This will cause addition lag that
		// is handled by the quadrature output buffer limit above.
		if (timerTopValue < (uint16_t) rateLimit)
			timerTopValue = (uint16_t) rateLimit;
	}

	// Return the timer TOP value
	return (uint8_t) timerTopValue;
}

void ProcessMouse() {

	if (Appli_state != APPLICATION_READY)
		return;

	if (USBH_HID_GetDeviceType(&hUsbHostFS) != HID_MOUSE)
		return;




	mousemap = USBH_HID_GetMouseInfo(&hUsbHostFS);
	if (mousemap != NULL) {
		// +X = Mouse going right
		// -X = Mouse going left
		// +Y = Mouse going down
		// -Y = Mouse going up
		//
		// X and Y have a range of -127 to +127

		// If the mouse movement changes direction then disregard any remaining
		// movement units in the previous direction.
		if (mousemap->x > 0 && mouseDirectionX == 0) {
			mouseDistanceX = 0;
			mouseDirectionX = 1;
		} else if (mousemap->x < 0 && mouseDirectionX == 1) {
			mouseDistanceX = 0;
			mouseDirectionX = 0;
		} else if (mousemap->y > 0 && mouseDirectionY == 0) {
			mouseDistanceY = 0;
			mouseDirectionY = 1;
		} else if (mousemap->y < 0 && mouseDirectionY == 1) {
			mouseDistanceY = 0;
			mouseDirectionY = 0;
		}

		// Process mouse X and Y movement -------------------------------------
		//HAL_TIM_Base_Start_IT(&htim2) ;
		//HAL_TIM_Base_Start_IT(&htim3) ;

		int16_t x_val = mousemap->x;
		int16_t y_val = mousemap->y;

		if (x_val > 0 && x_val < 10) {
			x_val = +10;
		}
		if (x_val < 0 && x_val < -10) {
			x_val = -10;
		}

		if (y_val > 0 && y_val < 10) {
			y_val = +10;
		}
		if (y_val < 0 && y_val < -10) {
			y_val = -10;
		}

		xTimerTop = processMouseMovement(mousemap->x, MOUSEX, 0U, 0U);
		yTimerTop = processMouseMovement(mousemap->y, MOUSEY, 0U, 0U);

		// Process mouse buttons ----------------------------------------------

		HAL_GPIO_WritePin(LB_GPIO_Port, LB_Pin, !(mousemap->buttons[0]));
		HAL_GPIO_WritePin(MB_GPIO_Port, RB_Pin, !(mousemap->buttons[1]));
		HAL_GPIO_WritePin(RB_GPIO_Port, MB_Pin, !(mousemap->buttons[2]));
	}
}

void ProcessX_IRQ() {

	// Process X output
	if (mouseDistanceX > 0) {
		// Set the output pins according to the current phase BH RHQ FV LVQ

		if (mouseEncoderPhaseX == 0)
			HAL_GPIO_WritePin(BH_GPIO_Port, BH_Pin, !(1));	// Set X1 to 1
		if (mouseEncoderPhaseX == 1)
			HAL_GPIO_WritePin(RHQ_GPIO_Port, RHQ_Pin, !(1));	// Set X2 to 1
		if (mouseEncoderPhaseX == 2)
			HAL_GPIO_WritePin(BH_GPIO_Port, BH_Pin, !(0));	// Set X1 to 0
		if (mouseEncoderPhaseX == 3)
			HAL_GPIO_WritePin(RHQ_GPIO_Port, RHQ_Pin, !(0));	// Set X2 to 0

		// Change phase
		if (mouseDirectionX == 0)
			mouseEncoderPhaseX--;
		else
			mouseEncoderPhaseX++;

		// Decrement the distance left to move
		mouseDistanceX--;

		// Range check the phase
		if ((mouseDirectionX == 1) && (mouseEncoderPhaseX > 3))
			mouseEncoderPhaseX = 0;
		if ((mouseDirectionX == 0) && (mouseEncoderPhaseX < 0))
			mouseEncoderPhaseX = 3;
	} else {
		// Reset the phase if the mouse isn't moving
		mouseEncoderPhaseX = 0;
	}

	// Set the timer top value for the next interrupt
	if (xTimerTop == 0) {
		TIM2->ARR = 1;
	} else {
		TIM2->ARR = xTimerTop;
	}

}

void ProcessY_IRQ() {

// Process Y output
	if (mouseDistanceY > 0) {
		// Set the output pins according to the current phase
		if (mouseEncoderPhaseY == 3)
			HAL_GPIO_WritePin(FV_GPIO_Port, LVQ_Pin, !(0));	// Set Y1 to 0
		if (mouseEncoderPhaseY == 2)
			HAL_GPIO_WritePin(LVQ_GPIO_Port, FV_Pin, !(0));	// Set Y2 to 0
		if (mouseEncoderPhaseY == 1)
			HAL_GPIO_WritePin(FV_GPIO_Port, LVQ_Pin, !(1));	// Set Y1 to 1
		if (mouseEncoderPhaseY == 0)
			HAL_GPIO_WritePin(LVQ_GPIO_Port, FV_Pin, !(1));	// Set Y2 to 1

		// Change phase
		if (mouseDirectionY == 0)
			mouseEncoderPhaseY--;
		else
			mouseEncoderPhaseY++;

		// Decrement the distance left to move
		mouseDistanceY--;

		// Range check the phase
		if ((mouseDirectionY == 1) && (mouseEncoderPhaseY > 3))
			mouseEncoderPhaseY = 0;
		if ((mouseDirectionY == 0) && (mouseEncoderPhaseY < 0))
			mouseEncoderPhaseY = 3;
	} else {
		// Reset the phase if the mouse isn't moving
		mouseEncoderPhaseY = 0;
	}

// Set the timer top value for the next interrupt
	if (yTimerTop == 0) {
		TIM3->ARR = 1;
	} else {
		TIM3->ARR = yTimerTop;
	}

}

