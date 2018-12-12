/**
 * This is the main file of the ESPinball.
 * *
 * @author: Simon Leier & Florian Geiser
 *
 *
 */

/* ToDo
 *
 *
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
TaskHandle_t	drawTaskHandle = NULL,
				checkJoystickHandle = NULL,
				checkButtonHandle = NULL,
				TaskControllerHandle = NULL,
				UserStatsHandle = NULL;



// creating of static handles
#define STACK_SIZE 200
StaticTask_t xTaskBuffer;
StackType_t xStack[ STACK_SIZE ];
TaskHandle_t CircleDisappearStaticHandle = NULL;


// creating of semaphores
SemaphoreHandle_t	CountButtonASemaphore;

// global variables
int8_t intDrawScreen = 1;
int8_t intSelectedMode = 1;

int8_t intTableStart = 2;
int8_t intTableLeft = 1;
int8_t intTableRight = 3;
int8_t intActTable = 1;

// global button variables
int8_t intButtonA = 0,
		intButtonB = 0,
		intButtonC = 0,
		intButtonD = 0,
		intButtonE = 0,
		intButtonK = 0;

// stats
int8_t intPlayerLevel = 1;
int32_t intScore = 0;
int32_t intScoreFirst = 0;
int32_t intScoreSecond = 0;
int32_t intScoreThird = 0;
int32_t intPassedTime = 0;
int8_t intFPS = 0;
int8_t intLifes = 3;


/*------------------------------------------------------------------------------------------------------------------------------*/
int main() {
	// Initialize Board functions and graphics
	ESPL_SystemInit();

	// Initializes Draw Queue with 100 lines buffer
	JoystickQueue = xQueueCreate(100, 2 * sizeof(char));

	// Initializes Tasks with their respective priority

	xTaskCreate(TaskController, "TaskController", 1000, NULL, 9, &TaskControllerHandle);
	xTaskCreate(UserStats, "UserStats", 1000, NULL, 2, &UserStatsHandle);
	//xTaskCreate(uartReceive, "uartReceive", 1000, NULL, 9, &uartReceiveHandle);

	// interface tasks
	xTaskCreate(checkJoystick, "checkJoystick", 1000, NULL, 5, &checkJoystickHandle);
	xTaskCreate(checkButton, "checkButton", 1000, NULL, 5, &checkButtonHandle);
	// drawing tasks
	xTaskCreate(drawTask, "drawTask", 1000, NULL, 4, &drawTaskHandle);

	// Start FreeRTOS Scheduler
	vTaskStartScheduler();
}
/*------------------------------------------------------------------------------------------------------------------------------*/

void TaskController() {
	int8_t intActScreen = 0;
	int8_t SwitchScreenFlag = 0;
	int8_t intDeviceStart = 0;

	while (TRUE) {

		/***********************
		 *	Switching Screens
		 ***********************/
		if (intDeviceStart == 0) {
			intActScreen = 1;
			intDeviceStart = 1;
			SwitchScreenFlag = 0;		//Screen is now possible to change
		}
		else if(intButtonE){
			intActScreen = 2;
			SwitchScreenFlag = 0;		//Screen is now possible to change
		}

		//Screens get activated here
		if (SwitchScreenFlag == 0) {
			switch (intActScreen) {

			case 1://Screen #1: Start Screen
				vTaskResume(drawTaskHandle);
				intDrawScreen = 1;

				SwitchScreenFlag = 1;		//indicates that screen just changed
				break;


			case 2://Screen #2: Choose Mode
				intDrawScreen = 2;

				SwitchScreenFlag = 1;		//indicates that screen just changed
				break;

			case 3:		//Screen #3: Single Player


				SwitchScreenFlag = 1;		//indicates that screen just changed
				break;
			default:
				SwitchScreenFlag = 1;		//indicates that screen just changed
				break;
			}
		}

		vTaskDelay(100);
	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void drawTask() {

	char str[100]; // buffer for messages to draw to display
	struct coord joystickPosition; // joystick queue input buffer

	// FPS
	TickType_t xLastWakeTime;
	int8_t intFrameCounter = 0;

	font_t font1, font2, font3; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");
	font2 = gdispOpenFont("DejaVuSans16*");
	font3 = gdispOpenFont("DejaVuSans12*");


	// LEVEL
	const int8_t thickLever = 3;
	const int16_t startAreaSize = 30;
	const int16_t coordStartAreaX = displaySizeX - startAreaSize; // 320-290=30 pixel size of start area
	const int16_t coordGameAreaX1 = 0;
	const int16_t coordGameAreaX2 = coordStartAreaX;
	const int16_t coordGameAreaY1 = 0;
	const int16_t coordGameAreaY2 = displaySizeY;
	//items
	const int16_t coinRadius = 5;
	const int16_t coordHoleLeftX = coordGameAreaX2/2 - 60;
	const int16_t coordHoleLeftY = coordGameAreaY1 + 50;
	const int16_t coordHoleRightX = coordGameAreaX2/2 + 60;
	const int16_t coordHoleRightY = coordGameAreaY1 + 50;



	//Moving parts
	 int coordRightLeverY1Triggered = 170;
	 int coordRightLeverY1Idle = 210;
	 float coordRightLeverX1 = coordGameAreaX2 / 2 + 20; //displaySizeX/2 + 10;
	 int coordRightLeverY1 = 220; //coordRightLeverY1Idle;
	 int coordRightLeverX2 = coordGameAreaX2 / 2 + 70; //displaySizeX/2 + 60;
	 int coordRightLeverY2 = 200;

	 int coordLeftLeverY2Triggered = 170;
	 int coordLeftLeverY2Idle = 210;
	 int coordLeftLeverX1 = coordGameAreaX2 / 2 - 70;
	 int coordLeftLeverY1 = 200;
	 int coordLeftLeverX2 = coordGameAreaX2 / 2 - 20;
	 int coordLeftLeverY2 = 220; //coordLeftLeverY2Idle;



	// Start endless loop
	while (TRUE) {
		while (xQueueReceive(JoystickQueue, &joystickPosition, 0) == pdTRUE);
		//FPS
		/*
		if(xLastWakeTime - xTaskGetTickCount > 1000){
			xLastWakeTime = xTaskGetTickCount();
			intFPS = intFrameCounter;
			intFrameCounter = 0;
		}
		else {
			intFrameCounter++;
		}*/


		// Clear background
		gdispClear(White);

		switch (intDrawScreen){
		// draw start screen
		case 1:
			// Generate string
			sprintf(str, "Welcome to ESPinball");
			gdispDrawString(40, 80, str, font1, Black);
			sprintf(str, "Press E to continue");
			gdispDrawString(40, 120, str, font2, Black);

			break;

		// draw menu
		case 2:

			// Mode Selection on second screen
			if (intButtonA && intSelectedMode > 1) {
				intSelectedMode--;
				vTaskDelay(100);
			} else if (intButtonC && intSelectedMode < 3) {
				intSelectedMode++;
				vTaskDelay(100);
			} else if (intButtonB && intSelectedMode == 1) {
				intDrawScreen = 3; 		// singleplayer mode chosen
			} else if (intButtonB && intSelectedMode == 2) {
				intDrawScreen = 4; 		// multiplayer mode chosen
			} else if (intButtonB && intSelectedMode == 3) {
				intDrawScreen = 5; 		// high score mode chosen
			} else if (intButtonD){
				intDrawScreen = 1;
			}

			sprintf(str, "Choose an option:");
			gdispDrawString(80, 50, str, font1, Black);

			switch (intSelectedMode) {
					// Singleplayer Mode selected
					case 1:
						sprintf(str, "Singleplayer");
						gdispDrawString(80, 100, str, font1, Blue);
						sprintf(str, "Multiplayer");
						gdispDrawString(80, 130, str, font1, Black);
						sprintf(str, "High Score");
						gdispDrawString(80, 160, str, font1, Black);
						break;
					// Multiplayer Mode selected
					case 2:
						sprintf(str, "Singleplayer");
						gdispDrawString(80, 100, str, font1, Black);
						sprintf(str, "Multiplayer");
						gdispDrawString(80, 130, str, font1, Blue);
						sprintf(str, "High Score");
						gdispDrawString(80, 160, str, font1, Black);
						break;
					// Highscore Mode selected
					case 3:
						sprintf(str, "Singleplayer");
						gdispDrawString(80, 100, str, font1, Black);
						sprintf(str, "Multiplayer");
						gdispDrawString(80, 130, str, font1, Black);
						sprintf(str, "High Score");
						gdispDrawString(80, 160, str, font1, Blue);
						break;
					default:
						break;
					}

			break;
		case 3: // singleplayer  mode

			//example thickline
			//gdispDrawThickLine(50, 50, 60, 50, Red, 5, TRUE);

			/****** LEVEL *****/
			if (intActTable == intTableStart) {

				// red holes to change table
				gdispFillCircle(coordHoleLeftX, coordHoleLeftY, 10, Red);
				gdispFillCircle(coordHoleRightX, coordHoleRightY, 10, Red);
				gdispDrawThickLine(coordHoleLeftX - 20, 50, coordHoleLeftX, 70, Black, 5, TRUE); 		// left bowl
				gdispDrawThickLine(coordHoleLeftX, 70, coordHoleLeftX + 20, 50, Black, 5, TRUE);		// left bowl
				gdispDrawThickLine(coordHoleRightX - 20, 50, coordHoleRightX,70, Black, 5, TRUE);		// right bowl
				gdispDrawThickLine(coordHoleRightX, 70, coordHoleRightX + 20,50, Black, 5, TRUE);		// right bowl
				// bumper
				gdispDrawThickLine(coordHoleLeftX + 2, 70,coordHoleLeftX + 20 + 2, 50, Green, 2, FALSE);// left bumper bowl
				gdispDrawThickLine(coordHoleRightX - 20 - 2, 50, coordHoleRightX - 2, 70, Green, 2, FALSE);// right bumper bowl
				// upper bumper
				gdispDrawThickLine(coordGameAreaX1, 40, coordGameAreaX1 + 15,60, Black, 5, TRUE); 		// left upper bumper
				gdispDrawThickLine(coordGameAreaX1 + 2, 40,coordGameAreaX1 + 15 + 2, 60, Blue, 2, FALSE);
				gdispDrawThickLine(coordGameAreaX2 - 15, 60, coordGameAreaX2,40, Black, 5, TRUE);		// right upper bumper
				gdispDrawThickLine(coordGameAreaX2 - 15 - 2, 60, coordGameAreaX2 - 2, 40, Blue, 2, FALSE);

				// coins
				gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 + 60, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 + 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 - 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 - 60, coinRadius, Yellow);

				gdispFillCircle(coordGameAreaX2 / 2 - 30, coordGameAreaY2 / 2 + 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 + 30, coordGameAreaY2 / 2 + 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 + 30, coordGameAreaY2 / 2 - 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 - 30, coordGameAreaY2 / 2 - 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 - 60, coordGameAreaY2 / 2 + 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 + 60, coordGameAreaY2 / 2 + 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 + 60, coordGameAreaY2 / 2 - 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 - 60, coordGameAreaY2 / 2 - 30, coinRadius, Yellow);

				gdispFillCircle(coordGameAreaX2 / 2 + 90, coordGameAreaY2 / 2, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 - 90, coordGameAreaY2 / 2, coinRadius, Yellow);


			} else if (intActTable == intTableLeft) {
				// red holes to change table
				gdispFillCircle(coordHoleLeftX, coordHoleLeftY, 10, Red);
				gdispFillCircle(coordHoleRightX, coordHoleRightY, 10, Red);
				gdispDrawThickLine(coordHoleLeftX - 20, 50, coordHoleLeftX, 70, Black, 5, TRUE); 		// left bowl
				gdispDrawThickLine(coordHoleLeftX, 70, coordHoleLeftX + 20, 50, Black, 5, TRUE);		// left bowl
				gdispDrawThickLine(coordHoleRightX - 20, 50, coordHoleRightX,70, Black, 5, TRUE);		// right bowl
				gdispDrawThickLine(coordHoleRightX, 70, coordHoleRightX + 20,50, Black, 5, TRUE);		// right bowl
				// bumper
				gdispDrawThickLine(coordHoleLeftX + 2, 70,coordHoleLeftX + 20 + 2, 50, Green, 2, FALSE);// left bumper bowl
				gdispDrawThickLine(coordHoleRightX - 20 - 2, 50, coordHoleRightX - 2, 70, Green, 2, FALSE);// right bumper bowl
				// upper bumper
				gdispDrawThickLine(coordGameAreaX1, 40, coordGameAreaX1 + 15,60, Black, 5, TRUE); 		// left upper bumper
				gdispDrawThickLine(coordGameAreaX1 + 2, 40,coordGameAreaX1 + 15 + 2, 60, Blue, 2, FALSE);
				gdispDrawThickLine(coordGameAreaX2 - 15, 60, coordGameAreaX2,40, Black, 5, TRUE);		// right upper bumper
				gdispDrawThickLine(coordGameAreaX2 - 15 - 2, 60, coordGameAreaX2 - 2, 40, Blue, 2, FALSE);

				// coins
				gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 + 60, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 + 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 - 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 - 60, coinRadius, Yellow);

				gdispFillCircle(coordGameAreaX2 / 2 - 30, coordGameAreaY2 / 2 + 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 + 30, coordGameAreaY2 / 2 + 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 + 30, coordGameAreaY2 / 2 - 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 - 30, coordGameAreaY2 / 2 - 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 - 60, coordGameAreaY2 / 2 + 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 + 60, coordGameAreaY2 / 2 + 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 + 60, coordGameAreaY2 / 2 - 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 - 60, coordGameAreaY2 / 2 - 30, coinRadius, Yellow);

				gdispFillCircle(coordGameAreaX2 / 2 + 90, coordGameAreaY2 / 2, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 - 90, coordGameAreaY2 / 2, coinRadius, Yellow);

			} else if (intActTable == intTableRight) {
				// red holes to change table
				gdispFillCircle(coordHoleLeftX, coordHoleLeftY, 10, Red);
				gdispFillCircle(coordHoleRightX, coordHoleRightY, 10, Red);
				gdispDrawThickLine(coordHoleLeftX - 20, 50, coordHoleLeftX, 70, Black, 5, TRUE); 		// left bowl
				gdispDrawThickLine(coordHoleLeftX, 70, coordHoleLeftX + 20, 50, Black, 5, TRUE);		// left bowl
				gdispDrawThickLine(coordHoleRightX - 20, 50, coordHoleRightX,70, Black, 5, TRUE);		// right bowl
				gdispDrawThickLine(coordHoleRightX, 70, coordHoleRightX + 20,50, Black, 5, TRUE);		// right bowl
				// bumper
				gdispDrawThickLine(coordHoleLeftX + 2, 70,coordHoleLeftX + 20 + 2, 50, Green, 2, FALSE);// left bumper bowl
				gdispDrawThickLine(coordHoleRightX - 20 - 2, 50, coordHoleRightX - 2, 70, Green, 2, FALSE);// right bumper bowl
				// upper bumper
				gdispDrawThickLine(coordGameAreaX1, 40, coordGameAreaX1 + 15,60, Black, 5, TRUE); 		// left upper bumper
				gdispDrawThickLine(coordGameAreaX1 + 2, 40,coordGameAreaX1 + 15 + 2, 60, Blue, 2, FALSE);
				gdispDrawThickLine(coordGameAreaX2 - 15, 60, coordGameAreaX2,40, Black, 5, TRUE);		// right upper bumper
				gdispDrawThickLine(coordGameAreaX2 - 15 - 2, 60, coordGameAreaX2 - 2, 40, Blue, 2, FALSE);

				// coins
				gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 + 60, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 + 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 - 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 - 60, coinRadius, Yellow);

				gdispFillCircle(coordGameAreaX2 / 2 - 30, coordGameAreaY2 / 2 + 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 + 30, coordGameAreaY2 / 2 + 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 + 30, coordGameAreaY2 / 2 - 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 - 30, coordGameAreaY2 / 2 - 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 - 60, coordGameAreaY2 / 2 + 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 + 60, coordGameAreaY2 / 2 + 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 + 60, coordGameAreaY2 / 2 - 30, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 - 60, coordGameAreaY2 / 2 - 30, coinRadius, Yellow);

				gdispFillCircle(coordGameAreaX2 / 2 + 90, coordGameAreaY2 / 2, coinRadius, Yellow);
				gdispFillCircle(coordGameAreaX2 / 2 - 90, coordGameAreaY2 / 2, coinRadius, Yellow);
			}

			/***********************************/
			/****necessary for each table *****/
			/***********************************/
			// lever
			gdispDrawThickLine(coordRightLeverX1, coordRightLeverY1,
					coordRightLeverX2, coordRightLeverY2, Gray, thickLever,
					TRUE);
			gdispDrawThickLine(coordLeftLeverX1, coordLeftLeverY1,
					coordLeftLeverX2, coordLeftLeverY2, Gray, thickLever,
					TRUE);
			// obstacles horizontal
			gdispDrawThickLine(coordRightLeverX2, coordRightLeverY2,
					coordRightLeverX2 + 50,
					coordRightLeverY2
							- (coordRightLeverY1Idle - coordRightLeverY2),
					Black, thickLever, TRUE);
			gdispDrawThickLine(coordLeftLeverX1 - 50,
					coordLeftLeverY1
							- (coordLeftLeverY2Idle - coordLeftLeverY1),
					coordLeftLeverX1, coordLeftLeverY1, Black, thickLever,
					TRUE);
			// obstacles vertical
			gdispDrawThickLine(coordRightLeverX2 + 50,
					coordRightLeverY2
							- (coordRightLeverY1Idle - coordRightLeverY2),
					coordRightLeverX2 + 50,
					coordRightLeverY2
							- (coordRightLeverY1Idle - coordRightLeverY2) - 50,
					Black, thickLever, TRUE);
			gdispDrawThickLine(coordLeftLeverX1 - 50,
					coordLeftLeverY1
							- (coordLeftLeverY2Idle - coordLeftLeverY1),
					coordLeftLeverX1 - 50,
					coordLeftLeverY1 - (coordLeftLeverY2Idle - coordLeftLeverY1)
							- 50, Black, thickLever, TRUE);


			// boundaries
			gdispDrawThickLine(coordRightLeverX1 + 10, coordGameAreaY2,
					coordGameAreaX2, coordRightLeverY2 + 10, Black, 5,
					FALSE);
			gdispDrawThickLine(0, coordLeftLeverY1 + 10,
					coordLeftLeverX2 - 10, coordGameAreaY2, Black, 5,
					FALSE);

			// level frame
			gdispDrawThickLine(0, 0, displaySizeX, 0, Black, 3, FALSE);
			gdispDrawThickLine(0, 0, 0, displaySizeY, Black, 3, FALSE);
			// start area
			gdispDrawThickLine(coordStartAreaX, 0, coordStartAreaX, 60,
					Black, 5, FALSE); //upper lines
			gdispDrawThickLine(coordStartAreaX, 58,
					coordStartAreaX + startAreaSize / 3, 63, Black, 5,
					FALSE); //upper lines curve
			gdispDrawThickLine(coordStartAreaX + startAreaSize / 3, 63,
					coordStartAreaX + 2 * startAreaSize / 3, 73, Black, 5,
					FALSE); //upper lines curve
			gdispDrawThickLine(coordStartAreaX + 2 * startAreaSize / 3, 73,
					coordStartAreaX + 3 * startAreaSize / 3, 90, Black, 5,
					FALSE); //upper lines curve
			gdispDrawThickLine(coordStartAreaX, 90, coordStartAreaX,
					displaySizeY, Black, 5, FALSE); //lower lines
			gdispDrawThickLine(displaySizeX, 0, displaySizeX, displaySizeY,
					Black, 5, FALSE);


			//***STATS***
			sprintf(str, "LEVEL: %d", intPlayerLevel);
			gdispDrawString(coordGameAreaX1 + 5, coordGameAreaY1 + 5, str,
					font3, Black);
			sprintf(str, "SCORE: %d", intScore);
			gdispDrawString(coordGameAreaX1 + 5, coordGameAreaY1 + 20, str,
					font3, Black);
			sprintf(str, "SEC: %d", intPassedTime);
			gdispDrawString(coordGameAreaX2 - 60, coordGameAreaY2 - 10, str,
					font3, Black);
			sprintf(str, "FPS: %d", intFPS);
			gdispDrawString(coordGameAreaX1 + 5, coordGameAreaY2 - 10, str,
					font3, Black);
			sprintf(str, "Lifes: %d", intLifes);
			gdispDrawString(coordGameAreaX2 - 50, coordGameAreaY1 + 10, str,
					font3, Black);

			// display size for debug
			sprintf(str, "displaySizeX %d", displaySizeX);
			gdispDrawString(70, 100, str, font2, Black);
			sprintf(str, "displaySizeY %d", displaySizeY);
			gdispDrawString(70, 120, str, font2, Black);

			//***INPUTS***
			if (intButtonB) {
				coordRightLeverY1 = coordRightLeverY1Triggered;
			} else {
				coordRightLeverY1 = coordRightLeverY1Idle;
			}
			if (intButtonD) {
				coordLeftLeverY2 = coordLeftLeverY2Triggered;
			} else {
				coordLeftLeverY2 = coordLeftLeverY2Idle;
			}


			break;

		case 4: // multiplayer mode
			sprintf(str, "Multiplayer mode");
			gdispDrawString(70, 70, str, font1, Black);
			break;
		case 5: // high score mode
			if (intButtonD){
				intDrawScreen = 2;
				vTaskDelay(50);
			}
			sprintf(str, "High Scores:");
			gdispDrawString(70, 70, str, font1, Black);

			sprintf(str, "1: %d", intScoreFirst);
			gdispDrawString(70, 100, str, font3, Black);
			sprintf(str, "2: %d", intScoreSecond);
			gdispDrawString(70, 120, str, font3, Black);
			sprintf(str, "3: %d", intScoreThird);
			gdispDrawString(70, 140, str, font3, Black);

			break;
		default:
			break;
		}


		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// swap buffers
		ESPL_DrawLayer();

	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/

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

void checkButton() {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t tickFramerate = 20;

	int8_t FlagButtonA = 0;
	int8_t FlagButtonB = 0;
	int8_t FlagButtonC = 0;
	int8_t FlagButtonD = 0;
	int8_t FlagButtonE = 0;
	int8_t FlagButtonK = 0;

	while (TRUE) {

		//Buttons are Pulled-Up, setting a flag to increase it once per press, debouncing --> vTaskDelay
		//Button A
		if (!GPIO_ReadInputDataBit(ESPL_Register_Button_A, ESPL_Pin_Button_A) && !FlagButtonA) {
			intButtonA = 1;
			FlagButtonA = 1;
		} else if (GPIO_ReadInputDataBit(ESPL_Register_Button_A, ESPL_Pin_Button_A)) {
			intButtonA = 0;
			FlagButtonA = 0;
		}
		//Button B
		if (!GPIO_ReadInputDataBit(ESPL_Register_Button_B, ESPL_Pin_Button_B) && !FlagButtonB) {
			intButtonB = 1;
			FlagButtonB = 1;
		} else if (GPIO_ReadInputDataBit(ESPL_Register_Button_B, ESPL_Pin_Button_B)) {
			intButtonB = 0;
			FlagButtonB = 0;
		}
		//Button C
		if (!GPIO_ReadInputDataBit(ESPL_Register_Button_C, ESPL_Pin_Button_C) && !FlagButtonC) {
			intButtonC = 1;
			FlagButtonC = 1;
		} else if (GPIO_ReadInputDataBit(ESPL_Register_Button_C, ESPL_Pin_Button_C)) {
			intButtonC = 0;
			FlagButtonC = 0;
		}
		//Button D
		if (!GPIO_ReadInputDataBit(ESPL_Register_Button_D, ESPL_Pin_Button_D) && !FlagButtonD) {
			intButtonD = 1;
			FlagButtonD = 1;
		} else if (GPIO_ReadInputDataBit(ESPL_Register_Button_D, ESPL_Pin_Button_D)) {
			intButtonD = 0;
			FlagButtonD = 0;
		}
		//Button E
		if (!GPIO_ReadInputDataBit(ESPL_Register_Button_E, ESPL_Pin_Button_E) && !FlagButtonE) {
			intButtonE = 1;
			FlagButtonE = 1;
		} else if (GPIO_ReadInputDataBit(ESPL_Register_Button_E, ESPL_Pin_Button_E)) {
			intButtonE = 0;
			FlagButtonE = 0;
		}
		//Button K
		if (!GPIO_ReadInputDataBit(ESPL_Register_Button_K, ESPL_Pin_Button_K) && !FlagButtonK) {
			intButtonK = 1;
			FlagButtonK = 1;
		} else if (GPIO_ReadInputDataBit(ESPL_Register_Button_K, ESPL_Pin_Button_K)) {
			intButtonK = 0;
			FlagButtonK = 0;
		}


		vTaskDelayUntil(&xLastWakeTime, tickFramerate);		//checks buttons every  20 ticks
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
	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void UserStats() {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	while(TRUE){
		if (intScore >= 1000 && intPlayerLevel <= 1) {
			intPlayerLevel = 2;
		} else if (intScore >= 3000 && intPlayerLevel <= 2) {
			intPlayerLevel = 3;
		} else {
			intPlayerLevel = 1;
		}

		if (intDrawScreen == 3 || intDrawScreen == 4) { //if single player or multi player selected, start timer
			intPassedTime++;
			vTaskDelayUntil(&xLastWakeTime, 1000);
		} else {
			intPassedTime = 0;
			intScore = 0;
			intPlayerLevel = 1;

		}


	// game ends
		if (intLifes <= 0) {
			//ToDo: Write On Display "Game Over"

			// refresh high score
			if (intScore >= intScoreFirst) {			//replace first
				intScoreThird = intScoreSecond;
				intScoreSecond = intScoreFirst;
				intScoreFirst = intScore;
			} else if (intScore >= intScoreSecond) {	//replace second
				intScoreThird = intScoreSecond;
				intScoreSecond = intScore;
			} else if (intScore >= intScoreThird) {		//replace third
				intScoreThird = intScore;
			}

		}

	}
}

/*------------------------------------------------------------------------------------------------------------------------------*/
/* Static Task Example
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


/*------------------------------------------------------------------------------------------------------------------------------*/
/* Semaphore Task Example
void countButtonA() {
	/// Attempt to create a semaphore.
	CountButtonASemaphore = xSemaphoreCreateBinary();

	while (TRUE) {
		if (CountButtonASemaphore != NULL) {
			// See if we can obtain the semaphore.  If the semaphore is not available wait 10 ticks to see if it becomes free.
			if (xSemaphoreTake(CountButtonASemaphore, 10) == pdTRUE) {
				//We were able to obtain the semaphore and can now access the shared resource.
				intCountButtonA++;
				//Pressing Button A releases another time the semaphore
			} else {
				//We could not obtain the semaphore and can therefore not access the shared resource safely.
			}
		}

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
