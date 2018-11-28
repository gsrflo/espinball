/**
 * This is the main file of the ESPLaboratory Demo project.
 * It implements simple sample functions for the usage of UART,
 * writing to the display and processing user inputs.
 *
 * @author: Jonathan Müller-Boruttau,
 * 			Tobias Fuchs tobias.fuchs@tum.de
 * 			Nadja Peters nadja.peters@tum.de (RCS, TUM)
 *
 */

/* ToDo
 *
 * 3.2.1 Switching between Exercise Solution Screens 2 3.3 3.4 --> done
 * https://exploreembedded.com/wiki/Task_Switching
 * 3.2.2 done
 * 3.2.3 done
 * 3.2.4 done
 * 3.2.5 UART
 * 3.3.1 done
 * 3.3.2
*/


#include "includes.h"
// start and stop bytes for the UART protocol

static const uint8_t startByte = 0xAA, stopByte = 0x55;

static const uint16_t displaySizeX = 320, displaySizeY = 240;

QueueHandle_t ESPL_RxQueue; // Already defined in ESPL_Functions.h
SemaphoreHandle_t ESPL_DisplayReady;

// Stores lines to be drawn
QueueHandle_t JoystickQueue;



// creating of dynamic handles
TaskHandle_t drawTaskHandle = NULL, checkJoystickHandle = NULL,
		CircleAppearHandle = NULL, CircleDisappearHandle = NULL,
		countButtonAHandle = NULL, countButtonBHandle = NULL,
		resetCountButtonHandle = NULL, controllableCounterHandle = NULL,
		PriorityOneTaskHandle = NULL, PriorityTwoTaskHandle = NULL,
		PriorityThreeTaskHandle = NULL, PriorityFourTaskHandle = NULL,
		PriorityOutputTaskHandle = NULL, TaskControllerHandle = NULL,
		uartReceiveHandle = NULL, DisplayFPSHandle = NULL/*, sendButtonsHandle = NULL*/;


// creating of static handles
#define STACK_SIZE 200
StaticTask_t xTaskBuffer;
StackType_t xStack[ STACK_SIZE ];
TaskHandle_t CircleDisappearStaticHandle = NULL;


// creating of semaphores
SemaphoreHandle_t	CountButtonASemaphore,
					PriorityThreeTaskSemaphore;



// other global variables
int intCountButtonA = 0;
int intCountButtonB = 0;
int intContrCounter = 0;
int UartFlagButtonD = 0;
int exercise_3_3_2_start = 0;

/*------------------------------------------------------------------------------------------------------------------------------*/
int main() {
	// Initialize Board functions and graphics
	ESPL_SystemInit();

	// Initializes Draw Queue with 100 lines buffer
	JoystickQueue = xQueueCreate(100, 2 * sizeof(char));

	// Initializes Tasks with their respective priority
	xTaskCreate(TaskController, "TaskController", 1000, NULL, 9, &TaskControllerHandle);
	xTaskCreate(uartReceive, "uartReceive", 1000, NULL, 9, &uartReceiveHandle);

	xTaskCreate(drawTask, "drawTask", 1000, NULL, 4, &drawTaskHandle);
	xTaskCreate(checkJoystick, "checkJoystick", 1000, NULL, 5, &checkJoystickHandle);
	xTaskCreate(CircleAppear, "CircleAppear", 1000, NULL, 6, &CircleAppearHandle);							//3.2.2
	xTaskCreate(DisplayFPS, "DisplayFPS", 1000, NULL, 3, &DisplayFPSHandle);								//3.2.2

	xTaskCreate(countButtonA, "countButtonA", 1000, NULL, 5, &countButtonAHandle);							//3.2.3
	xTaskCreate(countButtonB, "countButtonB", 1000, NULL, 4, &countButtonBHandle);							//3.2.3
	xTaskCreate(resetCountButton, "resetCountButton", 1000, NULL, 4, &resetCountButtonHandle);				//3.2.3
	xTaskCreate(controllableCounter,"controllableCounter", 1000, NULL, 4, &controllableCounterHandle);		//3.2.4
	//xTaskCreate(sendButtons,"sendButtons", 1000, NULL, 4, &sendButtonsHandle);		//3.2.4



	xTaskCreate(PriorityOneTask,"PriorityOneTask", 1000, NULL, 1, &PriorityOneTaskHandle);					//3.3.2
	xTaskCreate(PriorityTwoTask,"PriorityTwoTask", 1000, NULL, 2, &PriorityTwoTaskHandle);					//3.3.2
	xTaskCreate(PriorityThreeTask,"PriorityThreeTask", 1000, NULL, 3, &PriorityThreeTaskHandle);			//3.3.2
	xTaskCreate(PriorityFourTask,"PriorityFourTask", 1000, NULL, 4, &PriorityFourTaskHandle);				//3.3.2
	xTaskCreate(PriorityOutputTask,"PriorityOutputTask", 1000, NULL, 4, &PriorityOutputTaskHandle);			//3.3.2


	// Create the task without using any dynamic memory allocation.
	CircleDisappearStaticHandle = xTaskCreateStatic(CircleDisappearStatic,"CircleDisappearStatic", STACK_SIZE, ( void * ) 1, 5, xStack, &xTaskBuffer );  // Variable to hold the task's data structure.


	// Start FreeRTOS Scheduler
	vTaskStartScheduler();
}
/*------------------------------------------------------------------------------------------------------------------------------*/

void TaskController() {
	int16_t FlagButtonA = 0;
	int16_t FlagButtonB = 0;
	int16_t FlagButtonC = 0;
	int16_t FlagButtonD = 0;
	int16_t FlagButtonE = 0;
	int16_t intActExerc = 1;
	int16_t intContrCounterOn = 0;
	int16_t SwitchScreenFlag = 0;

	while (TRUE) {

		/***********************
		 *	3.2 Switching Exercises 2, 3.2, 3.3
		 ***********************/
		if (!GPIO_ReadInputDataBit(ESPL_Register_Button_E, ESPL_Pin_Button_E) && !FlagButtonE) {
			if (intActExerc == 1) {
				intActExerc = 2;	//exercise 2
			} else if (intActExerc == 2) {
				intActExerc = 3;	//exercise 3.2
			} else if (intActExerc == 3) {
				intActExerc = 1;	//exercise 3.3
			}
			FlagButtonE = 1;
			SwitchScreenFlag = 0;
			vTaskDelay(200);
		} else if (GPIO_ReadInputDataBit(ESPL_Register_Button_E, ESPL_Pin_Button_E)) {
			FlagButtonE = 0;
		}

		switch (intActExerc) {
		//Screen #1: exercise 2
		case 1:
			if (SwitchScreenFlag == 0) /*launches only, if switching to this screen*/{
				vTaskSuspend(CircleAppearHandle);
				vTaskSuspend(CircleDisappearStaticHandle);
				vTaskSuspend(countButtonAHandle);
				vTaskSuspend(countButtonBHandle);
				vTaskSuspend(resetCountButtonHandle);
				vTaskSuspend(controllableCounterHandle);
				vTaskSuspend(PriorityOneTaskHandle);
				vTaskSuspend(PriorityTwoTaskHandle);
				vTaskSuspend(PriorityThreeTaskHandle);
				vTaskSuspend(PriorityFourTaskHandle);
				vTaskSuspend(PriorityOutputTaskHandle);
				vTaskSuspend(uartReceiveHandle);
				vTaskSuspend(DisplayFPSHandle);
				//vTaskSuspend(sendButtonsHandle);

				vTaskResume(drawTaskHandle);
				vTaskResume(checkJoystickHandle);

				SwitchScreenFlag = 1;		//indicates that screen just changed
			}
			//launches repeatedly when this screen is active:
			break;

			//Screen #2: exercise 3.2
		case 2:
			if (SwitchScreenFlag == 0) /*launches only, if switching to this screen*/{
				vTaskSuspend(drawTaskHandle);
				vTaskSuspend(countButtonAHandle);
				vTaskSuspend(countButtonBHandle);
				vTaskSuspend(resetCountButtonHandle);
				vTaskSuspend(checkJoystickHandle);
				vTaskSuspend(PriorityOneTaskHandle);
				vTaskSuspend(PriorityTwoTaskHandle);
				vTaskSuspend(PriorityThreeTaskHandle);
				vTaskSuspend(PriorityFourTaskHandle);
				vTaskSuspend(PriorityOutputTaskHandle);

				vTaskResume(controllableCounterHandle);
				intContrCounterOn = 1;			// Flag for ControllableCounter

				vTaskResume(CircleAppearHandle);
				vTaskResume(CircleDisappearStaticHandle);
				vTaskResume(countButtonAHandle);
				vTaskResume(countButtonBHandle);
				vTaskResume(resetCountButtonHandle);
				//vTaskResume(sendButtonsHandle);

				//vTaskResume(DisplayFPSHandle);		// --> not  working yet

				SwitchScreenFlag = 1;		//indicates that screen just changed
			}
			//launches repeatedly when this screen is active:

			//Buttons are Pulled-Up, setting a flag to increase it once per press, debouncing --> vTaskDelay
			//Button A
			if (!GPIO_ReadInputDataBit(ESPL_Register_Button_A, ESPL_Pin_Button_A) && !FlagButtonA) {
				xSemaphoreGive(CountButtonASemaphore); //giving the semaphore, so intCountButtonA can be increased
				FlagButtonA = 1;
				vTaskDelay(200);
			} else if (GPIO_ReadInputDataBit(ESPL_Register_Button_A,
			ESPL_Pin_Button_A)) {
				FlagButtonA = 0;
			}
			//Button B
			if (!GPIO_ReadInputDataBit(ESPL_Register_Button_B, ESPL_Pin_Button_B) && !FlagButtonB) {
				xTaskNotifyGive(countButtonBHandle); //notifying Task countButtonB
				FlagButtonB = 1;
				vTaskDelay(200);
			} else if (GPIO_ReadInputDataBit(ESPL_Register_Button_B,
			ESPL_Pin_Button_B)) {
				FlagButtonB = 0;
			}

			//Button C:
			//starting & suspending Controllable Counter, exercise 3.2.4
			if (!GPIO_ReadInputDataBit(ESPL_Register_Button_C, ESPL_Pin_Button_C) && !FlagButtonC && !intContrCounterOn) {
				vTaskResume(controllableCounterHandle);
				FlagButtonC = 1;				//Flag -> edge detection
				intContrCounterOn = 1; 			//intContrCounterOn = 1 -> ControllableCounter on
				vTaskDelay(200);					//Delay(50) -> debouncing
			} else if (!GPIO_ReadInputDataBit(ESPL_Register_Button_C, ESPL_Pin_Button_C) && !FlagButtonC && intContrCounterOn) {
				vTaskSuspend(controllableCounterHandle);
				FlagButtonC = 1;
				intContrCounterOn = 0;
				vTaskDelay(200);
			} else if (GPIO_ReadInputDataBit(ESPL_Register_Button_C, ESPL_Pin_Button_C)) {
				FlagButtonC = 0;
			}
			break;

			//Screen #3: exercise 3.3
		case 3:
			if (SwitchScreenFlag == 0){ //launches only, if switching to this screen
				vTaskSuspend(drawTaskHandle);
				vTaskSuspend(CircleAppearHandle);
				vTaskSuspend(CircleDisappearStaticHandle);
				vTaskSuspend(controllableCounterHandle);
				vTaskSuspend(checkJoystickHandle);
				vTaskSuspend(countButtonAHandle);
				vTaskSuspend(countButtonBHandle);
				vTaskSuspend(resetCountButtonHandle);
				vTaskSuspend(DisplayFPSHandle);
				//vTaskSuspend(sendButtonsHandle);

				vTaskResume(PriorityOneTaskHandle);
				vTaskResume(PriorityTwoTaskHandle);
				vTaskResume(PriorityThreeTaskHandle);
				vTaskResume(PriorityFourTaskHandle);
				vTaskResume(PriorityOutputTaskHandle);

				exercise_3_3_2_start = xTaskGetTickCount();
				SwitchScreenFlag = 1;		//indicates that screen just changed
			}
			//launches repeatedly when this screen is active:

			break;
		default:
			intActExerc = 1;
			break;
		}
	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void drawTask() {

	float alpha = 0;	//angle for rotation around triangle
	int16_t r = 50;		//radius for rotation around triangle

	int16_t intButtonA = 0,
			intButtonB = 0,
			intButtonC = 0,
			intButtonD = 0;

	int8_t FlagButtonA = 0,
			FlagButtonB = 0,
			FlagButtonC = 0,
			FlagButtonD = 0,
			FlagButtonK = 0;


	char str[100]; // buffer for messages to draw to display
	struct coord joystickPosition; // joystick queue input buffer

	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");

	//rectangle coordinates
	uint16_t caveX0 = 150,
			caveY0 = 125,
			caveSize = 20,
			caveX = caveX0,
			caveY = caveY0;
	//circle coordinates
	uint16_t circlePositionX0 = 150,
			circlePositionY0 = 125,
			circlePositionX = circlePositionX0,
			circlePositionY = circlePositionY0;
	//triangle coordinates
	uint16_t trianglePositionX0 = 150,
			trianglePositionY0 = 125,
			trianglePositionX = trianglePositionX0,
			trianglePositionY = trianglePositionY0;

	// Start endless loop
	while (TRUE) {
		while (xQueueReceive(JoystickQueue, &joystickPosition, 0) == pdTRUE);

		//Initializing coordinates to move screen, minus joystick offset
		int16_t ScreenPosX = joystickPosition.x - 129,
				ScreenPosY = joystickPosition.y - 127;

		/**********************
		 *	2.1 Display
		 ***********************/
		// Clear background
		gdispClear(White);

		//Getting new coordinates
		caveX = caveX0 + r * sin(alpha) + ScreenPosX;
		caveY = caveY0 + r * cos(alpha) + ScreenPosY;
		circlePositionX = circlePositionX0 + r * sin(alpha) + ScreenPosX;
		circlePositionY = circlePositionY0 - r * cos(alpha) + ScreenPosY;
		trianglePositionX = trianglePositionX0 + ScreenPosX;
		trianglePositionY = trianglePositionY0 + ScreenPosY;

		//Increasing the angle
		if (alpha > 359) {
			alpha = 0;
		}
		else {
			alpha = alpha + 0.1;
		}

		// draw rectangle
		gdispFillArea(caveX - 10, caveY - 10, caveSize + 20, caveSize + 20,	Red);
		// color inner white
		gdispFillArea(caveX, caveY, caveSize, caveSize, White);

		//draw circle
		gdispFillCircle(circlePositionX, circlePositionY, 10, Green);

		//draw triangle
		gdispDrawLine(trianglePositionX, trianglePositionY - 15, trianglePositionX + 15, trianglePositionY + 15, Red);
		gdispDrawLine(trianglePositionX + 15, trianglePositionY + 15, trianglePositionX - 15, trianglePositionY + 15, Red);
		gdispDrawLine(trianglePositionX - 15, trianglePositionY + 15, trianglePositionX, trianglePositionY - 15, Red);

		// Generate string
		sprintf(str, "Random Text 7");
		gdispDrawString(20 + ScreenPosX, 50 + ScreenPosY, str, font1, Black);

		// Generate moving string
		sprintf(str, "Random Text 2");
		gdispDrawString(alpha + ScreenPosX, 20 + ScreenPosY, str, font1, Black);

		/**********************
		 *	2.2 Buttons
		 ***********************/
		sprintf(str, "A: %d|B: %d|C %d|D: %d|", intButtonA, intButtonB,
				intButtonC, intButtonD);
		gdispDrawString(150 + ScreenPosX, 50 + ScreenPosY, str, font1, Black);

		//Buttons are Pulled-Up, setting a flag to increase it once per press, debouncing --> vTaskDelay
		//Button A
		if (!GPIO_ReadInputDataBit(ESPL_Register_Button_A, ESPL_Pin_Button_A) && !FlagButtonA) {
			intButtonA++;
			FlagButtonA = 1;
			vTaskDelay(50);
		}
		else if (GPIO_ReadInputDataBit(ESPL_Register_Button_A, ESPL_Pin_Button_A)) {
			FlagButtonA = 0;
		}
		//Button B
		if (!GPIO_ReadInputDataBit(ESPL_Register_Button_B, ESPL_Pin_Button_B) && !FlagButtonB) {
			intButtonB++;
			FlagButtonB = 1;
			vTaskDelay(50);
		}
		else if (GPIO_ReadInputDataBit(ESPL_Register_Button_B, ESPL_Pin_Button_B)) {
			FlagButtonB = 0;
		}
		//Button C
		if (!GPIO_ReadInputDataBit(ESPL_Register_Button_C, ESPL_Pin_Button_C) && !FlagButtonC) {
			intButtonC++;
			FlagButtonC = 1;
			vTaskDelay(50);
		}
		else if (GPIO_ReadInputDataBit(ESPL_Register_Button_C, ESPL_Pin_Button_C)) {
			FlagButtonC = 0;
		}
		//Button D
		if (!GPIO_ReadInputDataBit(ESPL_Register_Button_D, ESPL_Pin_Button_D) && !FlagButtonD) {
			intButtonD++;
			FlagButtonD = 1;
			vTaskDelay(50);
		}
		else if (GPIO_ReadInputDataBit(ESPL_Register_Button_D, ESPL_Pin_Button_D)) {
			FlagButtonD = 0;
		}
		//Button K
		if (!GPIO_ReadInputDataBit(ESPL_Register_Button_K, ESPL_Pin_Button_K) && !FlagButtonK) {
			intButtonA = 0;
			intButtonB = 0;
			intButtonC = 0;
			intButtonD = 0;
			FlagButtonK = 1;
			vTaskDelay(50);
		}
		else if (GPIO_ReadInputDataBit(ESPL_Register_Button_K, ESPL_Pin_Button_K)) {
			FlagButtonK = 0;
		}

		/***********************
		 *	2.3 Joystick
		 ***********************/
		sprintf(str, "Joystick Position X: %d Y: %d", joystickPosition.x, joystickPosition.y);
		gdispDrawString(15 + ScreenPosX, 225 + ScreenPosY, str, font1, Black);


		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// swap buffers
		ESPL_DrawLayer();

	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
/**
 * This task polls the joystick value every 20 ticks
 */
void checkJoystick() {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	struct coord joystickPosition = { 0, 0 };
	const TickType_t tickFramerate = 20;

	while (TRUE) {
		// Remember last joystick values
		joystickPosition.x = (uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4);
		joystickPosition.y = (uint8_t) 255 - (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4);

		xQueueSend(JoystickQueue, &joystickPosition, 100);

		// Execute every 20 Ticks
		vTaskDelayUntil(&xLastWakeTime, tickFramerate);
	}
}

/*------------------------------------------------------------------------------------------------------------------------------*/
/**
 * Example function to send data over UART
 *
 * Sends coordinates of a given position via UART.
 * Structure of a package:
 *  8 bit start byte
 *  8 bit x-coordinate
 *  8 bit y-coordinate
 *  8 bit checksum (= x-coord XOR y-coord)
 *  8 bit stop byte
 */
void sendPosition(struct coord position) {
	const uint8_t checksum = position.x ^ position.y;

	UART_SendData(startByte);
	UART_SendData(position.x);
	UART_SendData(position.y);
	UART_SendData(checksum);
	UART_SendData(stopByte);
}
/*------------------------------------------------------------------------------------------------------------------------------*/
/**
 * Example how to receive data over UART (see protocol above)
 */
void uartReceive() {
	char input;
	uint8_t pos = 0;
	char checksum;
	char buffer[5]; // Start byte,4* line byte, checksum (all xor), End byte
	struct coord position = { 0, 0 };
	while (TRUE) {
		// wait for data in queue
		xQueueReceive(ESPL_RxQueue, &input, portMAX_DELAY);

		// decode package by buffer position
		switch (pos) {
		// start byte
		case 0:
			if (input != startByte)
				break;
		case 1:
		case 2:
		case 3:
			// read received data in buffer
			buffer[pos] = input;
			pos++;
			break;
		case 4:
			// Check if package is corrupted
			checksum = buffer[1] ^ buffer[2];
			if (input == stopByte || checksum == buffer[3]) {
				// pass position to Joystick Queue
				position.x = buffer[1];
				position.y = buffer[2];
				xQueueSend(JoystickQueue, &position, 100);
			}
			pos = 0;
		}

		/* For exercise 3.2.5.1 take inside case 4
				// Clear background
				gdispClear(White);

				//Write out input
				sprintf(str, "Input 1: %d", buffer[1]);
				gdispDrawString(300, 20, str, font1, Black);
				//Write out input
				sprintf(str, "Input 2: %d", buffer[2]);
				gdispDrawString(300, 40, str, font1, Black);

				// Wait for display to stop writing
				xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
				// swap buffers
				ESPL_DrawLayer();
				*/
	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
/*void sendButtons(){
	//Button D
	if (!GPIO_ReadInputDataBit(ESPL_Register_Button_D, ESPL_Pin_Button_D) && !UartFlagButtonD) {
			const uint8_t checksum = 1;
			UART_SendData(startByte);
			UART_SendData(1);
			UART_SendData(checksum);
			UART_SendData(stopByte);
			UartFlagButtonD = 1;
			vTaskDelay(50);
		}
		else if (GPIO_ReadInputDataBit(ESPL_Register_Button_D, ESPL_Pin_Button_D)) {
			UartFlagButtonD = 0;
		}
}*/
/*------------------------------------------------------------------------------------------------------------------------------*/

void CircleAppear() {
	char str[100];
	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");

	while (TRUE) {
		// Clear background
		gdispClear(White);
		//draw circle
		gdispFillCircle(150,150, 20, Blue);

		//Counter Button A
		sprintf(str, "Counter Button A: %d", intCountButtonA);
		gdispDrawString(150, 20, str, font1, Black);
		//Counter Button B
		sprintf(str, "Counter Button B: %d", intCountButtonB);
		gdispDrawString(150, 40, str, font1, Black);
		//ControllableCounter
		sprintf(str, "Controllable Counter: %d", intContrCounter);
		gdispDrawString(150, 60, str, font1, Black);


		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// swap buffers
		ESPL_DrawLayer();

		vTaskDelay(500); //500 ticks 2 Hz
		}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void CircleDisappearStatic(void * pvParameters){
	configASSERT( ( uint32_t ) pvParameters == 1UL );
	char str[100];
	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");


	while (1) {
		// Clear background
		gdispClear(White);

		//Counter Button A
		sprintf(str, "Counter Button A: %d", intCountButtonA);
		gdispDrawString(150, 20, str, font1, Black);
		//Counter Button B
		sprintf(str, "Counter Button B: %d", intCountButtonB);
		gdispDrawString(150, 40, str, font1, Black);
		//ControllableCounter
		sprintf(str, "Controllable Counter: %d", intContrCounter);
		gdispDrawString(150, 60, str, font1, Black);

		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// swap buffers
		ESPL_DrawLayer();

		vTaskDelay(1000); //1000 ticks 1 Hz
	}
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void DisplayFPS() {
	char str[100]; // buffer for messages to draw to display
	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");
	int StartTick = xTaskGetTickCount();
	int TicksPerSec = 0;

	while (1)
	{
		if(xTaskGetTickCount() - StartTick < 1000)		// 1000ticks --> 1s
		{
			TicksPerSec++;
		}
		else
		{
			// Clear background
			gdispClear(White);
			sprintf(str, "FPS: %d", TicksPerSec);
			gdispDrawString(200, 350, str, font1, Black);
			// Wait for display to stop writing
			xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
			// swap buffers
			ESPL_DrawLayer();

			TicksPerSec = 0;
			StartTick = xTaskGetTickCount();
		}
	}
}


/*------------------------------------------------------------------------------------------------------------------------------*/
/***********************
*	3.2.3
***********************/
void countButtonA() {
	/* Attempt to create a semaphore. */
	CountButtonASemaphore = xSemaphoreCreateBinary();

	while (TRUE) {
		if (CountButtonASemaphore != NULL) {
			/* See if we can obtain the semaphore.  If the semaphore is not
			 available wait 10 ticks to see if it becomes free. */
			if (xSemaphoreTake(CountButtonASemaphore, 10) == pdTRUE) {
				/* We were able to obtain the semaphore and can now access the
				 shared resource. */
				intCountButtonA++;
				//Pressing Button A releases another time the semaphore
			} else {
				/* We could not obtain the semaphore and can therefore not access
				 the shared resource safely. */
			}
		}

	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void countButtonB() {

	while (TRUE) {
		if(ulTaskNotifyTake( pdTRUE, portMAX_DELAY )){
		intCountButtonB++;
		}
	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void resetCountButton() {

	while (TRUE) {
			intCountButtonA = 0;
			intCountButtonB = 0;
			vTaskDelay(15000); 			//deleting Counter A & Counter B after 15s
		}

}
/*------------------------------------------------------------------------------------------------------------------------------*/
/***********************
 *	3.2.4
 ***********************/
void controllableCounter() {

	while (TRUE) {
		intContrCounter++;
		vTaskDelay(1000); //increasing variable every second
	}
}

/*------------------------------------------------------------------------------------------------------------------------------*/
/***********************
 *	3.3.2
 ***********************/
void PriorityOneTask() {
	char str[100]; // buffer for messages to draw to display
	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");

	//max. 15 iterations, than leaving task
	//for (int i = 0; i<15; i++){

	while(1){
		// Clear background
		gdispClear(White);

		sprintf(str, "1");
		gdispDrawString(100, 40, str, font1, Black);

		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// swap buffers
		ESPL_DrawLayer();

	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void PriorityTwoTask() {
	char str[100]; // buffer for messages to draw to display
	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");

	//max. 15 iterations, than leaving task
	//for (int i = 0; i<15; i++) {

	while (1) {

		if (!((xTaskGetTickCount() - exercise_3_3_2_start) % 2)) {

			// Clear background
			gdispClear(White);

			sprintf(str, "2");
			gdispDrawString(100, 60, str, font1, Black);

			// Wait for display to stop writing
			xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
			// swap buffers
			ESPL_DrawLayer();

			xSemaphoreGive(CountButtonASemaphore); // giving semaphore for PriorityThreeTask
		}
	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void PriorityThreeTask() {

	PriorityThreeTaskSemaphore = xSemaphoreCreateBinary();
	char str[100]; // buffer for messages to draw to display
	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");

	//max. 15 iterations, than leaving task
	//for (int i = 0; i<15; i++) {

	while(1){
		if (PriorityThreeTaskSemaphore != NULL) {
			/* See if we can obtain the semaphore.  If the semaphore is not available wait 10 ticks to see if it becomes free. */
			if (xSemaphoreTake(PriorityThreeTaskSemaphore, 10) == pdTRUE) {
				/* We were able to obtain the semaphore and can now access the shared resource. */
				// Clear background
				gdispClear(White);

				sprintf(str, "3");
				gdispDrawString(100, 80, str, font1, Black);

				// Wait for display to stop writing
				xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
				// swap buffers
				ESPL_DrawLayer();
			} else {
				/* We could not obtain the semaphore and can therefore not access
				 the shared resource safely. */
			}
		}


	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void PriorityFourTask() {
	char str[100]; // buffer for messages to draw to display
	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");

	//max. 15 iterations, than leaving task
	//for (int i = 0; i<15; i++) {

	while (1) {
		if (!((xTaskGetTickCount() - exercise_3_3_2_start) % 4)) {
			// Clear background
			gdispClear(White);

			sprintf(str, "4");
			gdispDrawString(100, 100, str, font1, Black);

			// Wait for display to stop writing
			xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
			// swap buffers
			ESPL_DrawLayer();
		}
	}

}
/*------------------------------------------------------------------------------------------------------------------------------*/
void PriorityOutputTask() {

	while(1){

	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/

/*
 *  Hook definitions needed for FreeRTOS to function.
 */
void vApplicationIdleHook() {
	while (TRUE) {
	};
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void vApplicationMallocFailedHook() {
	while (TRUE) {
	};
}
/*------------------------------------------------------------------------------------------------------------------------------*/
/*
 * for static allocated Task
 */

/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
