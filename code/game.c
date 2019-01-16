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
				UserStatsHandle = NULL,
				UserActionsHandle = NULL;



// creating of static handles
#define STACK_SIZE 200
StaticTask_t xTaskBuffer;
StackType_t xStack[ STACK_SIZE ];
TaskHandle_t CircleDisappearStaticHandle = NULL;


// creating of semaphores
SemaphoreHandle_t	CountButtonASemaphore;
/*------------------------------------------------------------------------------------------------------------------------------*/
// GLOBAL VARIABLES

int8_t intDrawScreen = 1;
int8_t intSelectedMode = 1;

int8_t intTableStart = 2;
int8_t intTableLeft = 1;
int8_t intTableRight = 3;
int8_t intActTable = 2;		//select [1:3] for table on display
int8_t intScreenBeforePause = 0;
int16_t intGainStartLever = 0;
int8_t intAnimation = 0; //select 1 for animation1 or 2 for animation 2

// global button variables
int8_t  intButtonA = 0,
		intButtonB = 0,
		intButtonC = 0,
		intButtonD = 0,
		intButtonE = 0,
		intButtonK = 0;

// stats
int8_t intPlayerLevel = 1;

int32_t intScoreSingle = 0;				// singleplayer high scores
int32_t intScoreFirstSingle = 0;
int32_t intScoreSecondSingle = 0;
int32_t intScoreThirdSingle = 0;
int32_t intScoreMulti = 0;				// multiplayer high scores
int32_t intScoreFirstMulti = 0;
int32_t intScoreSecondMulti = 0;
int32_t intScoreThirdMulti = 0;
int32_t intPassedTime = 0;
int8_t intFPS = 0;
int8_t intLifes = 3;


/*------------------------------------------------------------------------------------------------------------------------------*/
// FUNCTIONS
void fillPinballCircle(uint16_t x, uint16_t y, uint8_t radius, color_t color ){
	gdispFillCircle(x, y, radius, color);
	registerCollisionCircle(x, y, radius);
}
void drawPinballCircle(uint16_t x, uint16_t y, uint8_t radius, color_t color ){
	gdispFillCircle(x, y, radius, color);
	registerCollisionCircle(x, y, radius);
}
void drawPinballThickLineRound(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, color_t color, uint16_t width){
	gdispDrawThickLine(x1, y1, x2, y2, color, width, TRUE);
	registerCollisionLine(x1, y1, x2, y2);

	registerCollisionCircle(x1, y1, 3);
	registerCollisionCircle(x2, y2, 3);

	if (DEBUG) {
		gdispFillCircle(x1, y1, 3, Red);
		gdispFillCircle(x2, y2, 3, Red);
	}
}
void drawPinballThickLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, color_t color, uint16_t width){
	gdispDrawThickLine(x1, y1, x2, y2, color, width, FALSE);
	registerCollisionLine(x1, y1, x2, y2);

	registerCollisionCircle(x1, y1, 3);
	registerCollisionCircle(x2, y2, 3);

	if (DEBUG) {
		gdispFillCircle(x1, y1, 3, Red);
		gdispFillCircle(x2, y2, 3, Red);
	}
}

void drawTableEssentials(int coordStartAreaX, int startAreaSize, int coordGameAreaY2, int coordGameAreaX2,
		int coordRightLeverX1, int coordRightLeverX2, int coordRightLeverY1, int coordRightLeverY2, int coordLeftLeverX1, int coordLeftLeverX2,
		int coordLeftLeverY1, int coordLeftLeverY2, int coordRightLeverY1Idle, int coordLeftLeverY2Idle, int thickLever){
	/***********************************/
	/**** necessary for each table *****/
	/***********************************/
	// lever
	/*gdispDrawThickLine(coordRightLeverX1, coordRightLeverY1, coordRightLeverX2, coordRightLeverY2, Gray, thickLever, TRUE);
	gdispDrawThickLine(coordLeftLeverX1, coordLeftLeverY1, coordLeftLeverX2, coordLeftLeverY2, Gray, thickLever, TRUE);*/
	drawPinballThickLineRound(coordRightLeverX1, coordRightLeverY1, coordRightLeverX2, coordRightLeverY2, Gray, thickLever);
	drawPinballThickLineRound(coordLeftLeverX1, coordLeftLeverY1, coordLeftLeverX2, coordLeftLeverY2, Gray, thickLever);

	// obstacles horizontal
	/*gdispDrawThickLine(coordRightLeverX2, coordRightLeverY2, coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2), Black, thickLever, TRUE);
	gdispDrawThickLine(coordLeftLeverX1 - 50, coordLeftLeverY1 - (coordLeftLeverY2Idle - coordLeftLeverY1), coordLeftLeverX1, coordLeftLeverY1, Black, thickLever, TRUE);*/
	drawPinballThickLineRound(coordRightLeverX2, coordRightLeverY2, coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2), Black, thickLever);
	drawPinballThickLineRound(coordLeftLeverX1 - 50, coordLeftLeverY1 - (coordLeftLeverY2Idle - coordLeftLeverY1), coordLeftLeverX1, coordLeftLeverY1, Black, thickLever);

	// obstacles vertical
	/*gdispDrawThickLine(coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2), coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2) - 50, Black, thickLever, TRUE);
	gdispDrawThickLine(coordLeftLeverX1 - 50, coordLeftLeverY1 - (coordLeftLeverY2Idle - coordLeftLeverY1), coordLeftLeverX1 - 50, coordLeftLeverY1 - (coordLeftLeverY2Idle - coordLeftLeverY1) - 50, Black, thickLever, TRUE);*/
	drawPinballThickLineRound(coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2), coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2) - 50, Black, thickLever);
	drawPinballThickLineRound(coordLeftLeverX1 - 50, coordLeftLeverY1 - (coordLeftLeverY2Idle - coordLeftLeverY1), coordLeftLeverX1 - 50, coordLeftLeverY1 - (coordLeftLeverY2Idle - coordLeftLeverY1) - 50, Black, thickLever);

	// boundaries
	/*gdispDrawThickLine(coordRightLeverX1 + 10, coordGameAreaY2, coordGameAreaX2, coordRightLeverY2 + 10, Black, 5, FALSE);
	gdispDrawThickLine(0, coordLeftLeverY1 + 10, coordLeftLeverX2 - 10, coordGameAreaY2, Black, 5, FALSE);*/
	drawPinballThickLine(coordRightLeverX1 + 10, coordGameAreaY2, coordGameAreaX2, coordRightLeverY2 + 10, Black, 5);
	drawPinballThickLine(0, coordLeftLeverY1 + 10, coordLeftLeverX2 - 10, coordGameAreaY2, Black, 5);

	// level frame
	/*gdispDrawThickLine(0, 0, displaySizeX, 0, Black, 3, FALSE);
	gdispDrawThickLine(0, 0, 0, displaySizeY, Black, 3, FALSE);*/
	drawPinballThickLine(0, 0, displaySizeX, 0, Black, 3);
	drawPinballThickLine(0, 0, 0, displaySizeY, Black, 3);

	// start area
	/*gdispDrawThickLine(coordStartAreaX, 0, coordStartAreaX, 60,Black, 5, FALSE); //upper lines
	gdispDrawThickLine(coordStartAreaX, 58,coordStartAreaX + startAreaSize / 3, 63, Black, 5,FALSE); //upper lines curve
	gdispDrawThickLine(coordStartAreaX + startAreaSize / 3, 63,coordStartAreaX + 2 * startAreaSize / 3, 73, Black, 5,FALSE); //upper lines curve
	gdispDrawThickLine(coordStartAreaX + 2 * startAreaSize / 3, 73,coordStartAreaX + 3 * startAreaSize / 3, 90, Black, 5,FALSE); //upper lines curve
	gdispDrawThickLine(coordStartAreaX, 90, coordStartAreaX,displaySizeY, Black, 5, FALSE); //lower lines
	gdispDrawThickLine(displaySizeX, 0, displaySizeX, displaySizeY,Black, 5, FALSE);*/
	drawPinballThickLine(coordStartAreaX, 0, coordStartAreaX, 60,Black, 5); //upper lines
	drawPinballThickLine(coordStartAreaX, 58,coordStartAreaX + startAreaSize / 3, 63, Black, 5); //upper lines curve
	drawPinballThickLine(coordStartAreaX + startAreaSize / 3, 63,coordStartAreaX + 2 * startAreaSize / 3, 73, Black, 5); //upper lines curve
	drawPinballThickLine(coordStartAreaX + 2 * startAreaSize / 3, 73,coordStartAreaX + 3 * startAreaSize / 3, 90, Black, 5); //upper lines curve
	drawPinballThickLine(coordStartAreaX, 90, coordStartAreaX,displaySizeY, Black, 5); //lower lines
	drawPinballThickLine(displaySizeX, 0, displaySizeX, displaySizeY,Black, 5);

	//start lever
	/*gdispDrawThickLine(coordStartAreaX + 4, 200 + intGainStartLever, coordStartAreaX + 24, 200 + intGainStartLever, Red, 3, TRUE);
	gdispDrawThickLine(coordStartAreaX + 14, 200 + intGainStartLever, coordStartAreaX + 14, coordGameAreaY2, Black, 1, FALSE);*/
	drawPinballThickLineRound(coordStartAreaX + 4, 200 + intGainStartLever, coordStartAreaX + 24, 200 + intGainStartLever, Red, 3);
	drawPinballThickLine(coordStartAreaX + 14, 200 + intGainStartLever, coordStartAreaX + 14, coordGameAreaY2, Black, 1);
}
// single player tables
void drawTableStart(int coordHoleLeftX, int coordHoleLeftY, int coordHoleRightX, int coordHoleRightY,
		int coordGameAreaX1, int coordGameAreaX2, int coordGameAreaY1, int coordGameAreaY2, int coinRadius){

	// red holes to change table
	/*gdispFillCircle(coordHoleLeftX, coordHoleLeftY, 10, Red);
	gdispFillCircle(coordHoleRightX, coordHoleRightY, 10, Red);
	gdispDrawThickLine(coordHoleLeftX - 20, 50, coordHoleLeftX, 70, Black, 5, TRUE); 		// left bowl
	gdispDrawThickLine(coordHoleLeftX, 70, coordHoleLeftX + 20, 50, Black, 5, TRUE);		// left bowl
	gdispDrawThickLine(coordHoleRightX - 20, 50, coordHoleRightX,70, Black, 5, TRUE);		// right bowl
	gdispDrawThickLine(coordHoleRightX, 70, coordHoleRightX + 20,50, Black, 5, TRUE);		// right bowl
	*/
	fillPinballCircle(coordHoleLeftX, coordHoleLeftY, 10, Red);
	fillPinballCircle(coordHoleRightX, coordHoleRightY, 10, Red);
	drawPinballThickLineRound(coordHoleLeftX - 20, 50, coordHoleLeftX, 70, Black, 5); 		// left bowl
	drawPinballThickLineRound(coordHoleLeftX, 70, coordHoleLeftX + 20, 50, Black, 5);
	drawPinballThickLineRound(coordHoleRightX - 20, 50, coordHoleRightX, 70, Black, 5);
	drawPinballThickLineRound(coordHoleRightX, 70, coordHoleRightX + 20,50, Black, 5);

	// bumper
	/*gdispDrawThickLine(coordHoleLeftX + 2, 70,coordHoleLeftX + 20 + 2, 50, Green, 2, FALSE);// left bumper bowl
	gdispDrawThickLine(coordHoleRightX - 20 - 2, 50, coordHoleRightX - 2, 70, Green, 2, FALSE);// right bumper bowl
	*/
	drawPinballThickLine(coordHoleLeftX + 2, 70,coordHoleLeftX + 20 + 2, 50, Green, 2); // left bumper bowl
	drawPinballThickLine(coordHoleRightX - 20 - 2, 50, coordHoleRightX - 2, 70, Green, 2); // right bumper bowl

	// upper bumper
	/*gdispDrawThickLine(coordGameAreaX1, 40, coordGameAreaX1 + 15,60, Black, 5, TRUE); 		// left upper bumper
	gdispDrawThickLine(coordGameAreaX1 + 2, 40,coordGameAreaX1 + 15 + 2, 60, Blue, 2, FALSE);
	gdispDrawThickLine(coordGameAreaX2 - 15, 60, coordGameAreaX2,40, Black, 5, TRUE);		// right upper bumper
	gdispDrawThickLine(coordGameAreaX2 - 15 - 2, 60, coordGameAreaX2 - 2, 40, Blue, 2, FALSE);*/
	drawPinballThickLineRound(coordGameAreaX1, 40, coordGameAreaX1 + 15,60, Black, 5); 		// left upper bumper
	drawPinballThickLine(coordGameAreaX1 + 2, 40,coordGameAreaX1 + 15 + 2, 60, Blue, 2);
	drawPinballThickLineRound(coordGameAreaX2 - 15, 60, coordGameAreaX2,40, Black, 5);		// right upper bumper
	drawPinballThickLine(coordGameAreaX2 - 15 - 2, 60, coordGameAreaX2 - 2, 40, Blue, 2);

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
void drawTableLeft(int coordHoleLeftX, int coordHoleLeftY, int coordHoleRightX, int coordHoleRightY,
		int coordGameAreaX1, int coordGameAreaX2, int coordGameAreaY1, int coordGameAreaY2, int coinRadius){
	// red holes to change table
	/*gdispFillCircle(coordHoleLeftX, coordHoleLeftY, 10, Red);
	gdispFillCircle(coordHoleRightX, coordHoleRightY, 10, Red);
	gdispDrawThickLine(coordHoleLeftX - 30, 32, coordHoleLeftX + 30, 35, Black, 5, TRUE); 		// left bowl
	gdispDrawThickLine(coordHoleLeftX + 30, 35, coordHoleLeftX + 30, 60, Black, 5, TRUE);		// left bowl
	gdispDrawThickLine(coordHoleRightX - 30, 35, coordHoleRightX + 30, 32, Black, 5, TRUE);		// right bowl
	gdispDrawThickLine(coordHoleRightX - 30 , 35, coordHoleRightX - 30, 60, Black, 5, TRUE);		// right bowl
	*/
	fillPinballCircle(coordHoleLeftX, coordHoleLeftY, 10, Red);
	fillPinballCircle(coordHoleRightX, coordHoleRightY, 10, Red);
	drawPinballThickLineRound(coordHoleLeftX - 30, 32, coordHoleLeftX + 30, 35, Black, 5); 		// left bowl
	drawPinballThickLineRound(coordHoleLeftX + 30, 35, coordHoleLeftX + 30, 60, Black, 5);		// left bowl
	drawPinballThickLineRound(coordHoleRightX - 30, 35, coordHoleRightX + 30, 32, Black, 5);		// right bowl
	drawPinballThickLineRound(coordHoleRightX - 30 , 35, coordHoleRightX - 30, 60, Black, 5);		// right bowl

	// bumper
	/*gdispDrawThickLine(coordHoleLeftX + 30 + 2, 35 + 2, coordHoleLeftX + 30 + 2, 60 - 2, Green, 2, FALSE);// left bumper bowl
	gdispDrawThickLine(coordHoleRightX - 30 - 3, 35 + 2, coordHoleRightX - 30 - 3, 60 - 2, Green, 2, FALSE);// right bumper bowl
	*/
	drawPinballThickLine(coordHoleLeftX + 30 + 2, 35 + 2, coordHoleLeftX + 30 + 2, 60 - 2, Green, 2);// left bumper bowl
	drawPinballThickLine(coordHoleRightX - 30 - 3, 35 + 2, coordHoleRightX - 30 - 3, 60 - 2, Green, 2);// right bumper bowl

	// lower bumper
	/*gdispDrawThickLine(coordGameAreaX1 + 50, coordGameAreaY2 - 75, coordGameAreaX1 + 50 + 40, coordGameAreaY2 - 55 , Black, 5, TRUE); 		// left upper bumper2s
	gdispDrawThickLine(coordGameAreaX1 + 50 + 3, coordGameAreaY2 - 75, coordGameAreaX1 + 50 + 40, coordGameAreaY2 - 55 -2, Blue, 2, FALSE);
	gdispDrawThickLine(coordGameAreaX2 - 50 - 40, coordGameAreaY2 - 55, coordGameAreaX2 - 50, coordGameAreaY2 - 75, Black, 5, TRUE);		// right upper bumper
	gdispDrawThickLine(coordGameAreaX2 - 50 - 40, coordGameAreaY2 - 55 - 2, coordGameAreaX2 - 50 - 3, coordGameAreaY2 - 75, Blue, 2, FALSE);
	*/
	drawPinballThickLineRound(coordGameAreaX1 + 50, coordGameAreaY2 - 75, coordGameAreaX1 + 50 + 40, coordGameAreaY2 - 55 , Black, 5); 		// left upper bumper
	drawPinballThickLine(coordGameAreaX1 + 50 + 3, coordGameAreaY2 - 75, coordGameAreaX1 + 50 + 40, coordGameAreaY2 - 55 -2, Blue, 2);
	drawPinballThickLineRound(coordGameAreaX2 - 50 - 40, coordGameAreaY2 - 55, coordGameAreaX2 - 50, coordGameAreaY2 - 75, Black, 5);		// right upper bumper
	drawPinballThickLine(coordGameAreaX2 - 50 - 40, coordGameAreaY2 - 55 - 2, coordGameAreaX2 - 50 - 3, coordGameAreaY2 - 75, Blue, 2);

	// coins
	/*gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 + 60, coinRadius, Yellow);
	gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 + 30, coinRadius, Yellow);
	gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2, coinRadius, Yellow);
	gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 - 30, coinRadius, Yellow);
	gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 - 60, coinRadius, Yellow);

	gdispFillCircle(coordGameAreaX2 / 2 - 10, coordGameAreaY1 + 10, coinRadius, Yellow);
	gdispFillCircle(coordGameAreaX2 / 2 + 10, coordGameAreaY1 + 10, coinRadius, Yellow);
	gdispFillCircle(coordGameAreaX2 / 2 + 30, coordGameAreaY1 + 10, coinRadius, Yellow);
	gdispFillCircle(coordGameAreaX2 / 2 - 30, coordGameAreaY1 + 10, coinRadius, Yellow);

	gdispFillCircle(coordGameAreaX2 / 2 - 15, coordGameAreaY1 + 25, coinRadius, Yellow);
	gdispFillCircle(coordGameAreaX2 / 2 + 15, coordGameAreaY1 + 25, coinRadius, Yellow);
	gdispFillCircle(coordGameAreaX2 / 2 + 60, coordGameAreaY1 + 25, coinRadius, Yellow);
	gdispFillCircle(coordGameAreaX2 / 2 - 60, coordGameAreaY1 + 25, coinRadius, Yellow);
	*/
	fillPinballCircle(coordGameAreaX2 / 2 - 15, coordGameAreaY1 + 25, coinRadius, Yellow);
	fillPinballCircle(coordGameAreaX2 / 2 + 15, coordGameAreaY1 + 25, coinRadius, Yellow);
	fillPinballCircle(coordGameAreaX2 / 2 + 60, coordGameAreaY1 + 25, coinRadius, Yellow);
	fillPinballCircle(coordGameAreaX2 / 2 - 60, coordGameAreaY1 + 25, coinRadius, Yellow);

}
void drawTableRight(int coordHoleLeftX, int coordHoleLeftY, int coordHoleRightX, int coordHoleRightY,
		int coordGameAreaX1, int coordGameAreaX2, int coordGameAreaY1, int coordGameAreaY2, int coinRadius){

	// red holes to change table
	/*gdispFillCircle(coordGameAreaX2/2 - 20, coordGameAreaY1 + 20, 10, Red);
	gdispFillCircle(coordGameAreaX2/2 + 20, coordGameAreaY1 + 20, 10, Red);
	gdispDrawThickLine(coordGameAreaX2/2, coordGameAreaY1, coordGameAreaX2/2, coordGameAreaY1 + 40, Black, 5, TRUE);
	gdispDrawThickLine(coordGameAreaX2/2 - 30, coordGameAreaY1, coordGameAreaX2/2 - 50, coordGameAreaY1 + 40, Black, 5, TRUE);
	gdispDrawThickLine(coordGameAreaX2/2 + 30, coordGameAreaY1, coordGameAreaX2/2 + 50, coordGameAreaY1 + 40, Black, 5, TRUE);
	*/
	fillPinballCircle(coordGameAreaX2/2 - 20, coordGameAreaY1 + 20, 10, Red);
	fillPinballCircle(coordGameAreaX2/2 + 20, coordGameAreaY1 + 20, 10, Red);
	drawPinballThickLineRound(coordGameAreaX2/2, coordGameAreaY1, coordGameAreaX2/2, coordGameAreaY1 + 40, Black, 5);
	drawPinballThickLineRound(coordGameAreaX2/2 - 30, coordGameAreaY1, coordGameAreaX2/2 - 50, coordGameAreaY1 + 40, Black, 5);
	drawPinballThickLineRound(coordGameAreaX2/2 + 30, coordGameAreaY1, coordGameAreaX2/2 + 50, coordGameAreaY1 + 40, Black, 5);

	// lower bumper
	/*gdispDrawThickLine(coordGameAreaX1 + 50, coordGameAreaY2 - 75, coordGameAreaX1 + 50 + 40, coordGameAreaY2 - 55 , Black, 5, TRUE); 		// left upper bumper2s
	gdispDrawThickLine(coordGameAreaX1 + 50 + 3, coordGameAreaY2 - 75, coordGameAreaX1 + 50 + 40, coordGameAreaY2 - 55 -2, Blue, 2, FALSE);
	gdispDrawThickLine(coordGameAreaX2 - 50 - 40, coordGameAreaY2 - 55, coordGameAreaX2 - 50, coordGameAreaY2 - 75, Black, 5, TRUE);		// right upper bumper
	gdispDrawThickLine(coordGameAreaX2 - 50 - 40, coordGameAreaY2 - 55 - 2, coordGameAreaX2 - 50 - 3, coordGameAreaY2 - 75, Blue, 2, FALSE);
	*/
	drawPinballThickLineRound(coordGameAreaX1 + 50, coordGameAreaY2 - 75, coordGameAreaX1 + 50 + 40, coordGameAreaY2 - 55 , Black, 5); 		// left upper bumper2s
	drawPinballThickLine(coordGameAreaX1 + 50 + 3, coordGameAreaY2 - 75, coordGameAreaX1 + 50 + 40, coordGameAreaY2 - 55 -2, Blue, 2);
	drawPinballThickLineRound(coordGameAreaX2 - 50 - 40, coordGameAreaY2 - 55, coordGameAreaX2 - 50, coordGameAreaY2 - 75, Black, 5);		// right upper bumper
	drawPinballThickLine(coordGameAreaX2 - 50 - 40, coordGameAreaY2 - 55 - 2, coordGameAreaX2 - 50 - 3, coordGameAreaY2 - 75, Blue, 2);

	// circle bumper
	/*gdispFillCircle(coordGameAreaX2 / 2 + 50, coordGameAreaY2 / 2, 15, Silver);
	gdispDrawCircle(coordGameAreaX2 / 2 + 50, coordGameAreaY2 / 2, 15, Black);
	gdispFillCircle(coordGameAreaX2 / 2 - 50, coordGameAreaY2 / 2, 15, Silver);
	gdispDrawCircle(coordGameAreaX2 / 2 - 50, coordGameAreaY2 / 2, 15, Black);*/
	fillPinballCircle(coordGameAreaX2 / 2 + 50, coordGameAreaY2 / 2, 15, Silver);
	gdispDrawCircle(coordGameAreaX2 / 2 + 50, coordGameAreaY2 / 2, 15, Black);
	fillPinballCircle(coordGameAreaX2 / 2 - 50, coordGameAreaY2 / 2, 15, Silver);
	gdispDrawCircle(coordGameAreaX2 / 2 - 50, coordGameAreaY2 / 2, 15, Black);

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
}
// multi player tables
void drawTableMultiPlayer(int coordHoleLeftX, int coordHoleLeftY, int coordHoleRightX, int coordHoleRightY,
		int coordGameAreaX1, int coordGameAreaX2, int coordGameAreaY1, int coordGameAreaY2, int coinRadius){

	gdispDrawThickLine(coordGameAreaX2 / 2 - 80, 50, coordGameAreaX2 / 2, 10, Black, 5, TRUE);		// left bowl
	gdispDrawThickLine(coordGameAreaX2 / 2, 10, coordGameAreaX2 / 2 + 80, 50, Black, 5, TRUE);		// right bowl
	// bumper
	gdispDrawThickLine(coordGameAreaX2 / 2 - 80 + 2, 50,coordGameAreaX2 / 2 + 2, 10, Blue, 2, FALSE);// left bumper bowl
	gdispDrawThickLine(coordGameAreaX2 / 2 - 2, 10, coordGameAreaX2 / 2 + 80 - 2, 50, Green, 2, FALSE);// right bumper bowl
	// upper bumper
	gdispDrawThickLine(coordGameAreaX1, 40, coordGameAreaX1 + 15,60, Black, 5, TRUE); 		// left upper bumper
	gdispDrawThickLine(coordGameAreaX1 + 2, 40,coordGameAreaX1 + 15 + 2, 60, Green, 2, FALSE);
	gdispDrawThickLine(coordGameAreaX2 - 15, 60, coordGameAreaX2,40, Black, 5, TRUE);		// right upper bumper
	gdispDrawThickLine(coordGameAreaX2 - 15 - 2, 60, coordGameAreaX2 - 2, 40, Blue, 2, FALSE);

	// round bumper
	gdispFillCircle(coordGameAreaX2 / 2 + 30, coordGameAreaY2 / 2 + 30, 15, Green);
	gdispFillCircle(coordGameAreaX2 / 2 - 30 , coordGameAreaY2 / 2 + 30, 15, Blue);
	gdispFillCircle(coordGameAreaX2 / 2 - 30, coordGameAreaY2 / 2 - 30, 15, Green);
	gdispFillCircle(coordGameAreaX2 / 2 + 30, coordGameAreaY2 / 2 - 30, 15, Blue);

}

void drawTableMultiPlayerEssentials(int coordStartAreaX, int startAreaSize, int coordGameAreaY2, int coordGameAreaX2,
		int coordRightLeverX1, int coordRightLeverX2, int coordRightLeverY1, int coordRightLeverY2, int coordLeftLeverX1, int coordLeftLeverX2,
		int coordLeftLeverY1, int coordLeftLeverY2, int coordRightLeverY1Idle, int coordLeftLeverY2Idle, int thickLever){

	// lever
	/*gdispDrawThickLine(coordRightLeverX1, coordRightLeverY1, coordRightLeverX2, coordRightLeverY2, Gray, thickLever, TRUE);
	gdispDrawThickLine(coordLeftLeverX1, coordLeftLeverY1, coordLeftLeverX2, coordLeftLeverY2, Gray, thickLever, TRUE);*/
	drawPinballThickLineRound(coordRightLeverX1, coordRightLeverY1, coordRightLeverX2, coordRightLeverY2, Green, thickLever);
	drawPinballThickLineRound(coordLeftLeverX1, coordLeftLeverY1, coordLeftLeverX2, coordLeftLeverY2, Blue, thickLever);

	// obstacles horizontal
	/*gdispDrawThickLine(coordRightLeverX2, coordRightLeverY2, coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2), Black, thickLever, TRUE);
	gdispDrawThickLine(coordLeftLeverX1 - 50, coordLeftLeverY1 - (coordLeftLeverY2Idle - coordLeftLeverY1), coordLeftLeverX1, coordLeftLeverY1, Black, thickLever, TRUE);*/
	drawPinballThickLineRound(coordRightLeverX2, coordRightLeverY2, coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2), Black, thickLever);
	drawPinballThickLineRound(coordLeftLeverX1 - 50, coordLeftLeverY1 - (coordLeftLeverY2Idle - coordLeftLeverY1), coordLeftLeverX1, coordLeftLeverY1, Black, thickLever);

	// obstacles vertical
	/*gdispDrawThickLine(coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2), coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2) - 50, Black, thickLever, TRUE);
	gdispDrawThickLine(coordLeftLeverX1 - 50, coordLeftLeverY1 - (coordLeftLeverY2Idle - coordLeftLeverY1), coordLeftLeverX1 - 50, coordLeftLeverY1 - (coordLeftLeverY2Idle - coordLeftLeverY1) - 50, Black, thickLever, TRUE);*/
	drawPinballThickLineRound(coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2), coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2) - 50, Black, thickLever);
	drawPinballThickLineRound(coordLeftLeverX1 - 50, coordLeftLeverY1 - (coordLeftLeverY2Idle - coordLeftLeverY1), coordLeftLeverX1 - 50, coordLeftLeverY1 - (coordLeftLeverY2Idle - coordLeftLeverY1) - 50, Black, thickLever);

	// boundaries
	/*gdispDrawThickLine(coordRightLeverX1 + 10, coordGameAreaY2, coordGameAreaX2, coordRightLeverY2 + 10, Black, 5, FALSE);
	gdispDrawThickLine(0, coordLeftLeverY1 + 10, coordLeftLeverX2 - 10, coordGameAreaY2, Black, 5, FALSE);*/
	drawPinballThickLine(coordRightLeverX1 + 10, coordGameAreaY2, coordGameAreaX2, coordRightLeverY2 + 10, Black, 5);
	drawPinballThickLine(0, coordLeftLeverY1 + 10, coordLeftLeverX2 - 10, coordGameAreaY2, Black, 5);

	// level frame
	/*gdispDrawThickLine(0, 0, displaySizeX, 0, Black, 3, FALSE);
	gdispDrawThickLine(0, 0, 0, displaySizeY, Black, 3, FALSE);*/
	drawPinballThickLine(0, 0, displaySizeX, 0, Black, 3);
	drawPinballThickLine(0, 0, 0, displaySizeY, Black, 3);

	// start area
	/*gdispDrawThickLine(coordStartAreaX, 0, coordStartAreaX, 60,Black, 5, FALSE); //upper lines
	gdispDrawThickLine(coordStartAreaX, 58,coordStartAreaX + startAreaSize / 3, 63, Black, 5,FALSE); //upper lines curve
	gdispDrawThickLine(coordStartAreaX + startAreaSize / 3, 63,coordStartAreaX + 2 * startAreaSize / 3, 73, Black, 5,FALSE); //upper lines curve
	gdispDrawThickLine(coordStartAreaX + 2 * startAreaSize / 3, 73,coordStartAreaX + 3 * startAreaSize / 3, 90, Black, 5,FALSE); //upper lines curve
	gdispDrawThickLine(coordStartAreaX, 90, coordStartAreaX,displaySizeY, Black, 5, FALSE); //lower lines
	gdispDrawThickLine(displaySizeX, 0, displaySizeX, displaySizeY,Black, 5, FALSE);*/
	drawPinballThickLine(coordStartAreaX, 0, coordStartAreaX, 60,Black, 5); //upper lines
	drawPinballThickLine(coordStartAreaX, 58,coordStartAreaX + startAreaSize / 3, 63, Black, 5); //upper lines curve
	drawPinballThickLine(coordStartAreaX + startAreaSize / 3, 63,coordStartAreaX + 2 * startAreaSize / 3, 73, Black, 5); //upper lines curve
	drawPinballThickLine(coordStartAreaX + 2 * startAreaSize / 3, 73,coordStartAreaX + 3 * startAreaSize / 3, 90, Black, 5); //upper lines curve
	drawPinballThickLine(coordStartAreaX, 90, coordStartAreaX,displaySizeY, Black, 5); //lower lines
	drawPinballThickLine(displaySizeX, 0, displaySizeX, displaySizeY,Black, 5);

	//start lever
	/*gdispDrawThickLine(coordStartAreaX + 4, 200 + intGainStartLever, coordStartAreaX + 24, 200 + intGainStartLever, Red, 3, TRUE);
	gdispDrawThickLine(coordStartAreaX + 14, 200 + intGainStartLever, coordStartAreaX + 14, coordGameAreaY2, Black, 1, FALSE);*/
	drawPinballThickLineRound(coordStartAreaX + 4, 200 + intGainStartLever, coordStartAreaX + 24, 200 + intGainStartLever, Red, 3);
	drawPinballThickLine(coordStartAreaX + 14, 200 + intGainStartLever, coordStartAreaX + 14, coordGameAreaY2, Black, 1);
}
void drawMenu(){
	char str[100]; // buffer for messages to draw to display

	font_t font1, font2, font3; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");
	font2 = gdispOpenFont("DejaVuSans16*");
	font3 = gdispOpenFont("DejaVuSans12*");

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
}
void drawStats(int coordGameAreaX1, int coordGameAreaX2, int coordGameAreaY1, int coordGameAreaY2){
	char str[100]; // buffer for messages to draw to display

	font_t font1, font2, font3; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");
	font2 = gdispOpenFont("DejaVuSans16*");
	font3 = gdispOpenFont("DejaVuSans12*");

	//***STATS***
	sprintf(str, "LEVEL: %d", intPlayerLevel);
	gdispDrawString(coordGameAreaX1 + 5, coordGameAreaY1 + 5, str, font3,Black);
	sprintf(str, "SCORE: %d", intScoreSingle);
	gdispDrawString(coordGameAreaX1 + 5, coordGameAreaY1 + 20, str, font3,Black);
	sprintf(str, "SEC: %d", intPassedTime);
	gdispDrawString(coordGameAreaX2 - 60, coordGameAreaY2 - 10, str, font3,Black);
	sprintf(str, "FPS: %d", intFPS);
	gdispDrawString(coordGameAreaX1 + 5, coordGameAreaY2 - 10, str, font3,Black);
	sprintf(str, "LIFES: %d", intLifes);
	gdispDrawString(coordGameAreaX2 - 55, coordGameAreaY1 + 10, str, font3,Black);
}
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
	xTaskCreate(UserActions, "UserActions", 1000, NULL, 6, &UserActionsHandle);
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
	TickType_t xWakeTime;
	xWakeTime = xTaskGetTickCount();
	xLastWakeTime = xWakeTime;
	int8_t intFrameCounter = 0;
	int16_t intTickOnLastCall = 0;

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

	//animation
	int32_t intAnimationTime = 0;
	int coordAnimationOneX = 0;
	int coordAnimationOneY = 50;
	int coordAnimationTwoX1 = 0;
	int coordAnimationTwoY1 = 0;
	int coordAnimationTwoX2 = 150;
	int coordAnimationTwoY2 = 50;


	// Start endless loop
	while (TRUE) {
		while (xQueueReceive(JoystickQueue, &joystickPosition, 0) == pdTRUE);

		resetCollisionObjects();

		//FPS
		xLastWakeTime = xWakeTime;
		//50Hz
		vTaskDelayUntil(&xWakeTime, 1000/50);
		gdispClear(White);
		//Calculate and show FPS
		uint16_t delay = xWakeTime - xLastWakeTime;
		intFPS = 1000 / delay;



		// Clear background
		gdispClear(White);

		switch (intDrawScreen){
		// draw start screen
		case 1:
			sprintf(str, "Welcome to ESPinball");
			gdispDrawString(40, 80, str, font1, Black);
			sprintf(str, "Press E to continue");
			gdispDrawString(40, 120, str, font2, Black);
			break;
		// draw menu
		case 2:
			drawMenu();
			break;
		case 3: // singleplayer  mode

			/****** LEVEL *****/
			if (intActTable == intTableStart) {
				drawTableStart(coordHoleLeftX, coordHoleLeftY, coordHoleRightX, coordHoleRightY, coordGameAreaX1, coordGameAreaX2, coordGameAreaY1, coordGameAreaY2, coinRadius);

			} else if (intActTable == intTableLeft) {
				drawTableLeft(coordHoleLeftX, coordHoleLeftY, coordHoleRightX, coordHoleRightY, coordGameAreaX1, coordGameAreaX2, coordGameAreaY1, coordGameAreaY2, coinRadius);

			} else if (intActTable == intTableRight) {
				drawTableRight(coordHoleLeftX, coordHoleLeftY, coordHoleRightX, coordHoleRightY, coordGameAreaX1, coordGameAreaX2, coordGameAreaY1, coordGameAreaY2, coinRadius);
			}
			// necessary for each table
			drawTableEssentials(coordStartAreaX, startAreaSize, coordGameAreaY2, coordGameAreaX2,
					coordRightLeverX1, coordRightLeverX2, coordRightLeverY1, coordRightLeverY2, coordLeftLeverX1, coordLeftLeverX2, coordLeftLeverY1, coordLeftLeverY2, coordRightLeverY1Idle, coordLeftLeverY2Idle, thickLever);

			calculatePhysics(xWakeTime - xLastWakeTime);
			drawBall();


			//***ANIMATIONS***
			// animation 1 stops after 1000 calls
			// animation 2 stops after 3000 calls
			switch(intAnimation){
			case 1:
				if (intAnimationTime < 1000)
				{
					sprintf(str, "NICE!");
					gdispDrawString(coordAnimationOneX, coordAnimationOneY, str, font1, Blue);
					coordAnimationOneX++;
					intAnimationTime++;
				}
				else {
					intAnimationTime = 0;
					coordAnimationOneX = 0;
				}
				break;
			case 2:
				if (intAnimationTime < 3000) {
					if (intAnimationTime % 9){
						gdispFillArea(coordAnimationTwoX1, coordAnimationTwoY1, coordAnimationTwoX2, coordAnimationTwoY2, Lime);
						sprintf(str, "INSANE!!!");
						gdispDrawString(coordAnimationTwoX1 + 10, coordAnimationTwoY1 + 15, str, font1, Red);
					}
					else if (intAnimationTime % 8) {
						gdispFillArea(coordAnimationTwoX1, coordAnimationTwoY1, coordAnimationTwoX2, coordAnimationTwoY2, Blue);
						sprintf(str, "INSANE!!!");
						gdispDrawString(coordAnimationTwoX1 + 10, coordAnimationTwoY1 + 15, str, font1, Red);
					}
					gdispDrawBox(coordAnimationTwoX1, coordAnimationTwoY1, coordAnimationTwoX2, coordAnimationTwoY2, Black);

					coordAnimationTwoX1++;
					coordAnimationTwoY1++;
					intAnimationTime++;
				} else {
					intAnimationTime = 0;
					coordAnimationTwoX1 = 0;
					coordAnimationTwoY1 = 0;
				}
				break;
			case 3:
				break;
			default:
				break;
			}

			drawStats(coordGameAreaX1, coordGameAreaX2, coordGameAreaY1, coordGameAreaY2);

			//***INPUTS***
			if (intButtonB) {
				if (coordRightLeverY1 != coordRightLeverY1Triggered) {
					for (int i = 0; i < 20; i++) {
						double range = coordRightLeverY1Triggered - coordRightLeverY1Idle;
						double y1 = coordRightLeverY1Idle + range / i;
						if (checkLineCollision(position[0], position[1], coordRightLeverX1, y1, coordRightLeverX2, coordRightLeverY2)) {
							position[1] = coordRightLeverY1Triggered;
							velocity[1] = -400;
						}
					}
				}
				coordRightLeverY1 = coordRightLeverY1Triggered;
			} else {
				coordRightLeverY1 = coordRightLeverY1Idle;
			}

			if (intButtonD) {
				if (coordLeftLeverY2 != coordLeftLeverY2Triggered) {
					for (int i = 0; i < 20; i++) {
						double range = coordLeftLeverY2Triggered - coordLeftLeverY2Idle;
						double y1 = coordLeftLeverY2Idle + range / i;
						if (checkLineCollision(position[0], position[1], coordLeftLeverX1, y1, coordLeftLeverX2, coordLeftLeverY2)) {
							position[1] = coordRightLeverY1Triggered;
							velocity[1] = -400;
						}
					}
				}
				coordLeftLeverY2 = coordLeftLeverY2Triggered;
			} else {
				coordLeftLeverY2 = coordLeftLeverY2Idle;
			}

			break;


		case 4: // multiplayer mode
			sprintf(str, "Multiplayer mode");
			gdispDrawString(50, 110, str, font1, Black);

			drawTableMultiPlayer(coordHoleLeftX, coordHoleLeftY, coordHoleRightX, coordHoleRightY, coordGameAreaX1, coordGameAreaX2, coordGameAreaY1, coordGameAreaY2, coinRadius);

			drawTableMultiPlayerEssentials(coordStartAreaX, startAreaSize, coordGameAreaY2, coordGameAreaX2,
								coordRightLeverX1, coordRightLeverX2, coordRightLeverY1, coordRightLeverY2, coordLeftLeverX1, coordLeftLeverX2, coordLeftLeverY1, coordLeftLeverY2, coordRightLeverY1Idle, coordLeftLeverY2Idle, thickLever);
			drawStats(coordGameAreaX1, coordGameAreaX2, coordGameAreaY1, coordGameAreaY2);

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
		case 5: // high score mode
			if (intButtonD){
				intDrawScreen = 2;
				vTaskDelay(50);
			}

			sprintf(str, "High Scores:");
			gdispDrawString(70, 20, str, font1, Black);
			// singleplayer high scores
			sprintf(str, "Singleplayer:");
			gdispDrawString(70, 50, str, font3, Black);
			sprintf(str, "1: %d", intScoreFirstSingle);
			gdispDrawString(70, 70, str, font3, Black);
			sprintf(str, "2: %d", intScoreSecondSingle);
			gdispDrawString(70, 90, str, font3, Black);
			sprintf(str, "3: %d", intScoreThirdSingle);
			gdispDrawString(70, 110, str, font3, Black);
			// multiplayer high scores
			sprintf(str, "Multiplayer:");
			gdispDrawString(70, 150, str, font3, Black);
			sprintf(str, "1: %d", intScoreFirstMulti);
			gdispDrawString(70, 170, str, font3, Black);
			sprintf(str, "2: %d", intScoreSecondMulti);
			gdispDrawString(70, 190, str, font3, Black);
			sprintf(str, "3: %d", intScoreThirdMulti);
			gdispDrawString(70, 210, str, font3, Black);

			break;
		case 6: //pause mode
			gdispClear(Black);
			sprintf(str, "PAUSE");
			gdispDrawString(coordGameAreaX2 / 2 - 10, coordGameAreaY2 / 2 - 10,	str, font1, Red);
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
	const TickType_t tickFramerate = 50;

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

void UserActions(){
	struct coord joystickPosition; // joystick queue input buffer
	while (TRUE) {
		while (xQueueReceive(JoystickQueue, &joystickPosition, 0) == pdTRUE);

		// game starts
		// joystick Position Y on  127 if idle, 0 if up directed, 255 if down directed
		if (joystickPosition.y > 127) {
			intGainStartLever = (joystickPosition.y - 127) / 4;
		} else if (joystickPosition.y <= 127) {
			intGainStartLever = 0;
		}
		// to pause the game
		if (intButtonK && (intDrawScreen == 3 || intDrawScreen == 4 || intDrawScreen == 6)) {
			if (intDrawScreen != 6){
				intScreenBeforePause = intDrawScreen;
				intDrawScreen = 6;
			}
			else {
				intDrawScreen = intScreenBeforePause;
			}
			//stop time
			//stop gravity
			vTaskDelay(500);
		}

	vTaskDelay(10);
	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void UserStats() {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();


	while(TRUE){

		// user stats
		if (intScoreSingle >= 1000 && intPlayerLevel <= 1) {
			intPlayerLevel = 2;
		} else if (intScoreSingle >= 3000 && intPlayerLevel <= 2) {
			intPlayerLevel = 3;
		} else {
			intPlayerLevel = 1;
		}

		if (intDrawScreen == 3 || intDrawScreen == 4) { //if single player or multi player selected, start timer
			intPassedTime++;
			vTaskDelayUntil(&xLastWakeTime, 1000);
		} else if (intDrawScreen != 6) { //reset stats except pause screen is active
			intPassedTime = 0;
			intScoreSingle = 0;
			intPlayerLevel = 1;
		}


		// game ends
		if (intLifes <= 0) {
			//ToDo: Write On Display "Game Over"

			// refresh high score
			if (intScoreSingle >= intScoreFirstSingle) {			//replace first
				intScoreThirdSingle = intScoreSecondSingle;
				intScoreSecondSingle = intScoreFirstSingle;
				intScoreFirstSingle = intScoreSingle;
			} else if (intScoreSingle >= intScoreSecondSingle) {	//replace second
				intScoreThirdSingle = intScoreSecondSingle;
				intScoreSecondSingle = intScoreSingle;
			} else if (intScoreSingle >= intScoreThirdSingle) {		//replace third
				intScoreThirdSingle = intScoreSingle;
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
