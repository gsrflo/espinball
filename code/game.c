/**
 * This is the main file of ESPinball.
 * *
 * @author: Simon Leier & Florian Geiser
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
				UserActionsHandle = NULL,
				uartReceiveHandle = NULL,
				AnimationTimerTaskHandle = NULL,
				BallStuckTaskHandle = NULL;



// creating of static handles
#define STACK_SIZE 200
StaticTask_t xTaskBuffer;
StackType_t xStack[ STACK_SIZE ];



/*------------------------------------------------------------------------------------------------------------------------------*/
// GLOBAL VARIABLES
/*------------------------------------------------------------------------------------------------------------------------------*/
int8_t intDrawScreen = 1;
int8_t intSelectedMode = 1;

int8_t intTableThree = 3;
int8_t intTableOne = 1;
int8_t intTableTwo = 2;
int8_t intActTable = 2;		//select [1:3] for table on display
int8_t intScreenBeforePause = 0;
int16_t intGainStartLever = 0;
int8_t 	intStartAreaClosed = 0;

// global variables for animations
int8_t intAnimation = 0; 					//select 1 for animation1 or 2 for animation 2
int8_t coinRadiusHittable = 5;
color_t colorFillCircleTableTwo = Silver;
color_t colorDrawCircleTableTwo = Black;
color_t colorCoinsTableThree = Yellow;
int8_t animationCounterTableThree = 1;
int8_t bumperRadiusTableOne = 50;
int8_t startBigAnimationTableOne = 0;		// flag to start animation
int8_t startBigAnimationTableTwo = 0;		// flag to start animation
int8_t startBigAnimationTableThree = 0;		// flag to start animation

// global variables for button
int8_t  intButtonA = 0,
		intButtonB = 0,
		intButtonC = 0,
		intButtonD = 0,
		intButtonE = 0,
		intButtonK = 0;

// global variables for stats
int8_t intPlayerLevel = 1;

int intScoreSingle = 0;				// singleplayer high scores
int intScoreFirstSingle = 0;
int intScoreSecondSingle = 0;
int intScoreThirdSingle = 0;
int intScoreMulti = 0;				// multiplayer high scores
int intScoreFirstMulti = 0;
int intScoreSecondMulti = 0;
int intScoreThirdMulti = 0;
int intPassedTime = 0;
int8_t intFPS = 0;
int8_t intLifes = 3;
int8_t gameover = 0;
int8_t flagGameMode = 0;				// 1 for single player, 2 for multi player

// debug variables (blue: 0)
int lastActivePlayer = 0;

/*------------------------------------------------------------------------------------------------------------------------------*/
// FUNCTIONS
/*------------------------------------------------------------------------------------------------------------------------------*/
void fillPinballCircleWithId(uint16_t x, uint16_t y, uint8_t radius, color_t color, uint16_t id){
	gdispFillCircle(x, y, radius, color);
	registerCollisionCircle(x, y, radius, id);
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void fillPinballCircle(uint16_t x, uint16_t y, uint8_t radius, color_t color) {
	fillPinballCircleWithId(x, y, radius, color, OBJECT_ENV);
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void drawPinballThickLineRound(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, color_t color, uint16_t width){
	gdispDrawThickLine(x1, y1, x2, y2, color, width, TRUE);
	registerCollisionLine(x1, y1, x2, y2, OBJECT_ENV);

	registerCollisionCircle(x1, y1, 2, OBJECT_ENV);
	registerCollisionCircle(x2, y2, 2, OBJECT_ENV);

	if (DEBUG) {
		gdispFillCircle(x1, y1, 2, Red);
		gdispFillCircle(x2, y2, 2, Red);
	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void drawPinballThickLineWithId(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, color_t color, uint16_t width, uint16_t id){
	gdispDrawThickLine(x1, y1, x2, y2, color, width, FALSE);
	registerCollisionLine(x1, y1, x2, y2, id);

	registerCollisionCircle(x1, y1, 2, id);
	registerCollisionCircle(x2, y2, 2, id);

	if (DEBUG) {
		gdispFillCircle(x1, y1, 2, Red);
		gdispFillCircle(x2, y2, 2, Red);
	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void drawPinballThickLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, color_t color, uint16_t width){
	drawPinballThickLineWithId(x1, y1, x2, y2, color, width, OBJECT_ENV);
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void drawTableEssentials(int coordStartAreaX, int startAreaSize, int coordGameAreaY2, int coordGameAreaX2,
		int coordRightLeverX1, int coordRightLeverX2, int coordRightLeverY1, int coordRightLeverY2, int coordLeftLeverX1, int coordLeftLeverX2,
		int coordLeftLeverY1, int coordLeftLeverY2, int coordRightLeverY1Idle, int coordLeftLeverY2Idle, int thickLever){
	/***********************************/
	/**** necessary for each table *****/
	/***********************************/
	// lever
	drawPinballThickLineRound(coordRightLeverX1, coordRightLeverY1, coordRightLeverX2, coordRightLeverY2, Gray, thickLever);
	drawPinballThickLineRound(coordLeftLeverX1, coordLeftLeverY1, coordLeftLeverX2, coordLeftLeverY2, Gray, thickLever);

	// obstacles horizontal
	drawPinballThickLineRound(coordRightLeverX2, coordRightLeverY2, coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2), Black, thickLever);
	drawPinballThickLineRound(coordLeftLeverX1 - 50, coordLeftLeverY1 - (coordLeftLeverY2Idle - coordLeftLeverY1), coordLeftLeverX1, coordLeftLeverY1, Black, thickLever);

	// obstacles vertical
	drawPinballThickLineRound(coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2), coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2) - 50, Black, thickLever);
	drawPinballThickLineRound(coordLeftLeverX1 - 50, coordLeftLeverY1 - (coordLeftLeverY2Idle - coordLeftLeverY1), coordLeftLeverX1 - 50, coordLeftLeverY1 - (coordLeftLeverY2Idle - coordLeftLeverY1) - 50, Black, thickLever);

	registerCollisionCircle(coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2), 5, OBJECT_ENV); //additional circle to avoid stuck ball
	registerCollisionCircle(coordLeftLeverX1 - 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2), 5, OBJECT_ENV);  //additional circle to avoid stuck ball
	if (DEBUG) {
		gdispFillCircle(coordRightLeverX2 + 50,	coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2), 5, Red);
		gdispFillCircle(coordLeftLeverX1 - 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2), 5, Red);
	}

	// boundaries
	drawPinballThickLine(coordRightLeverX1 + 10, coordGameAreaY2, coordGameAreaX2, coordRightLeverY2 + 10, Black, 5);
	drawPinballThickLine(0, coordLeftLeverY1 + 10, coordLeftLeverX2 - 10, coordGameAreaY2, Black, 5);

	// level frame
	drawPinballThickLine(0, 0, displaySizeX, 0, Black, 3);
	drawPinballThickLine(0, 0, 0, displaySizeY, Black, 3);

	// start area
	drawPinballThickLine(coordStartAreaX, 0, coordStartAreaX, 60,Black, 5); //upper lines
	drawPinballThickLine(coordStartAreaX, 58,coordStartAreaX + startAreaSize / 3, 63, Black, 5); //upper lines curve
	drawPinballThickLine(coordStartAreaX + startAreaSize / 3, 63,coordStartAreaX + 2 * startAreaSize / 3, 73, Black, 5); //upper lines curve
	drawPinballThickLine(coordStartAreaX + 2 * startAreaSize / 3, 73,coordStartAreaX + 3 * startAreaSize / 3, 90, Black, 5); //upper lines curve
	drawPinballThickLine(coordStartAreaX, 90, coordStartAreaX,displaySizeY, Black, 5); //lower lines
	drawPinballThickLine(displaySizeX, 0, displaySizeX, displaySizeY,Black, 5);

	//start lever
	drawPinballThickLineRound(coordStartAreaX + 4, 200 + intGainStartLever, coordStartAreaX + 24, 200 + intGainStartLever, Red, 3);
	drawPinballThickLine(coordStartAreaX + 14, 200 + intGainStartLever, coordStartAreaX + 14, coordGameAreaY2, Black, 1);
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void checkCloseStartArea(int coordStartAreaX){
	if (position[0] < coordStartAreaX - 10 || intStartAreaClosed == 1) {
		drawPinballThickLine(coordStartAreaX, 60, coordStartAreaX, 90, Black, 5); //upper lines
		intStartAreaClosed = 1;
	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void checkCollisionObject(uint8_t id){

	if(intDrawScreen == 3) {			// single player score
		switch (id) {
		case 0:
			break;
		case 1:				// normal
			intScoreSingle = intScoreSingle + 100;
			break;
		case 2:				// small bonus
			intScoreSingle = intScoreSingle + 200;
			// start small animation
			if (intActTable == intTableTwo) {
				// lets circle bumper disappear & appear
				if (colorFillCircleTableTwo == Silver) {
					colorFillCircleTableTwo = White;
					colorDrawCircleTableTwo = White;
				}else if(colorFillCircleTableTwo == White){
					colorFillCircleTableTwo = Silver;
					colorDrawCircleTableTwo = Black;
				}
			} else if (intActTable == intTableOne) {
				// bumper radius decreases
				if (bumperRadiusTableOne >= 5) {
					bumperRadiusTableOne = bumperRadiusTableOne - 5;
				}
			} else if (intActTable == intTableThree) {
				// coins change color
				if(colorCoinsTableThree == Yellow){
					colorCoinsTableThree =Pink;
				}else if (colorCoinsTableThree == Pink){
					colorCoinsTableThree = Green;
				}else if (colorCoinsTableThree == Green){
					colorCoinsTableThree = Blue;
				}else if(colorCoinsTableThree == Blue){
					colorCoinsTableThree = Yellow;
				}
			}
			break;
		case 3:				// big bonus
			intScoreSingle = intScoreSingle + 300;
			//start big animation
			if(intActTable == intTableTwo){
				startBigAnimationTableTwo = 1;						// flag for starting animation
			} else if(intActTable == intTableOne){
				startBigAnimationTableOne = 1;						// flag for starting animation
			} else if(intActTable == intTableThree){
				startBigAnimationTableThree = 1;					// flag for starting animation
			}
			break;
		case 4:				// change table left
			if(intActTable == intTableOne){
				intActTable = intTableThree;
				intStartAreaClosed = 0;
				position[0] = startposition[0];		//setting ball to start area
				position[1] = startposition[1];		//setting ball to start area
			} else if(intActTable == intTableTwo){
				intActTable = intTableOne;
				intStartAreaClosed = 0;
				position[0] = startposition[0];		//setting ball to start area
				position[1] = startposition[1];		//setting ball to start area
			} else if (intActTable == intTableThree){
				intActTable = intTableTwo;
				intStartAreaClosed = 0;
				position[0] = startposition[0];		//setting ball to start area
				position[1] = startposition[1];		//setting ball to start area
			}
			break;
		case 5:				// change table right
			if (intActTable == intTableOne) {
				intActTable = intTableTwo;
				intStartAreaClosed = 0;
				position[0] = startposition[0];		//setting ball to start area
				position[1] = startposition[1];		//setting ball to start area
			} else if (intActTable == intTableTwo) {
				intActTable = intTableThree;
				intStartAreaClosed = 0;
				position[0] = startposition[0];		//setting ball to start area
				position[1] = startposition[1];		//setting ball to start area
			} else if (intActTable == intTableThree) {
				intActTable = intTableOne;
				intStartAreaClosed = 0;
				position[0] = startposition[0];		//setting ball to start area
				position[1] = startposition[1];		//setting ball to start area
			}
			break;
		default:
			break;
		}
	} else if(intDrawScreen == 4){		// multi player score
		switch (id) {
		case 6:				// normal player blue
			if (lastActivePlayer == 0) {
				intScoreMulti = intScoreMulti + 100;
			}
			break;
		case 7:				// normal player green
			if (lastActivePlayer == 1) {
				intScoreMulti = intScoreMulti + 100;
			}
			break;
		case 8:				// small bonus player blue
			if (lastActivePlayer == 0) {
				intScoreMulti = intScoreMulti + 200;
			}
			break;
		case 9:				// small bonus player green
			if (lastActivePlayer == 1) {
				intScoreMulti = intScoreMulti + 200;
			}
			break;
		case 10:			// big bonus player blue
			if (lastActivePlayer == 0) {
				intScoreMulti = intScoreMulti + 300;
			}
			break;
		case 11:			// big bonus player green
			if (lastActivePlayer == 1) {
				intScoreMulti = intScoreMulti + 300;
			}
			break;
		default:
			break;
		}
	}

}
/*------------------------------------------------------------------------------------------------------------------------------*/
// single player tables functions
/*------------------------------------------------------------------------------------------------------------------------------*/
void drawAdditionalLeverSP(int coordRightLeverX1, int coordRightLeverY1, int coordRightLeverX2, int coordRightLeverY2, int coordLeftLeverX1,
		int coordLeftLeverY1, int coordLeftLeverX2, int coordLeftLeverY2, int coordRightLeverY1Idle, int coordLeftLeverY2Idle, int thickLever){

	//additional pair of lever
	// lever
	drawPinballThickLineRound(coordRightLeverX1, coordRightLeverY1 - 90, coordRightLeverX2, coordRightLeverY2 - 90, Gray, thickLever);
	drawPinballThickLineRound(coordLeftLeverX1, coordLeftLeverY1 - 90, coordLeftLeverX2, coordLeftLeverY2 - 90, Gray, thickLever);

	// obstacles horizontal
	drawPinballThickLineRound(coordRightLeverX2, coordRightLeverY2 - 90, coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2) - 90, Black, thickLever);
	drawPinballThickLineRound(coordLeftLeverX1 - 50, coordLeftLeverY1 - (coordLeftLeverY2Idle - coordLeftLeverY1) - 90, coordLeftLeverX1, coordLeftLeverY1 - 90, Black, thickLever);

}
/*------------------------------------------------------------------------------------------------------------------------------*/
void drawTableThree(int coordHoleLeftX, int coordHoleLeftY, int coordHoleRightX, int coordHoleRightY, int coordGameAreaX1, int coordGameAreaX2, int coordGameAreaY1, int coordGameAreaY2, int coinRadius){

	// red holes to change table
	fillPinballCircleWithId(coordHoleLeftX, coordHoleLeftY, 10, Red, OBJECT_CHANGE_TABLE_LEFT);
	gdispDrawCircle(coordHoleLeftX, coordHoleLeftY, 10, Black);
	fillPinballCircleWithId(coordHoleRightX, coordHoleRightY, 10, Red, OBJECT_CHANGE_TABLE_RIGHT);
	gdispDrawCircle(coordHoleRightX, coordHoleRightY, 10, Black);

	drawPinballThickLineRound(coordHoleLeftX - 20, 38, coordHoleLeftX, 58, Black, 5); 		// left bowl
	drawPinballThickLineRound(coordHoleLeftX, 58, coordHoleLeftX + 20, 38, Black, 5);
	drawPinballThickLineRound(coordHoleRightX - 20, 38, coordHoleRightX, 58, Black, 5);
	drawPinballThickLineRound(coordHoleRightX, 58, coordHoleRightX + 20, 38, Black, 5);

	// bumper
	drawPinballThickLineWithId(coordHoleLeftX + 2, 58,coordHoleLeftX + 20 + 2, 38, Green, 2, OBJECT_SMALL_BONUS); // left bumper bowl
	drawPinballThickLineWithId(coordHoleRightX - 20 - 2, 38, coordHoleRightX - 2, 58, Green, 2, OBJECT_SMALL_BONUS); // right bumper bowl

	// upper bumper
	drawPinballThickLineRound(coordGameAreaX1, 40, coordGameAreaX1 + 15,60, Black, 5); 		// left upper bumper
	drawPinballThickLineWithId(coordGameAreaX1 + 2, 40,coordGameAreaX1 + 15 + 2, 60, Blue, 2, OBJECT_BIG_BONUS);
	drawPinballThickLineRound(coordGameAreaX2 - 15, 60, coordGameAreaX2,40, Black, 5);		// right upper bumper
	drawPinballThickLineWithId(coordGameAreaX2 - 15 - 2, 60, coordGameAreaX2 - 2, 40, Blue, 2, OBJECT_BIG_BONUS);

	// coins
	bigAnimationTableThree(coordGameAreaX2, coordGameAreaY2, coinRadius);


}
/*------------------------------------------------------------------------------------------------------------------------------*/
void bigAnimationTableThree(int coordGameAreaX2, int coordGameAreaY2, int coinRadius){
	if (animationCounterTableThree == 1) {
		gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 + 60,coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 + 30, coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2, coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 - 30, coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 - 60, coinRadius, colorCoinsTableThree);

		gdispFillCircle(coordGameAreaX2 / 2 - 30, coordGameAreaY2 / 2 + 30,	coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2 + 30, coordGameAreaY2 / 2 + 30, coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2 + 30, coordGameAreaY2 / 2 - 30, coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2 - 30, coordGameAreaY2 / 2 - 30, coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2 - 60, coordGameAreaY2 / 2 + 30, coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2 + 60, coordGameAreaY2 / 2 + 30, coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2 + 60, coordGameAreaY2 / 2 - 30, coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2 - 60, coordGameAreaY2 / 2 - 30, coinRadius, colorCoinsTableThree);

		gdispFillCircle(coordGameAreaX2 / 2 + 90, coordGameAreaY2 / 2, coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2 - 90, coordGameAreaY2 / 2, coinRadius, colorCoinsTableThree);

	} else if (animationCounterTableThree == 2) {
		gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 + 60,coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 + 30, coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2, coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 - 30, coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 - 60, coinRadius, colorCoinsTableThree);

		gdispFillCircle(coordGameAreaX2 / 2 - 30, coordGameAreaY2 / 2 + 30,	coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2 + 30, coordGameAreaY2 / 2 + 30, coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2 + 30, coordGameAreaY2 / 2 - 30, coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2 - 30, coordGameAreaY2 / 2 - 30, coinRadius, colorCoinsTableThree);

		gdispFillCircle(coordGameAreaX2 / 2 + 60, coordGameAreaY2 / 2, coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2 - 60, coordGameAreaY2 / 2, coinRadius, colorCoinsTableThree);
	} else if (animationCounterTableThree == 3) {
		gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 + 60,coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 + 30, coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2, coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 - 30, coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 - 60, coinRadius, colorCoinsTableThree);

		gdispFillCircle(coordGameAreaX2 / 2 + 30, coordGameAreaY2 / 2, coinRadius, colorCoinsTableThree);
		gdispFillCircle(coordGameAreaX2 / 2 - 30, coordGameAreaY2 / 2, coinRadius, colorCoinsTableThree);
	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void drawTableOne(int coordHoleLeftX, int coordHoleLeftY, int coordHoleRightX, int coordHoleRightY,
		int coordGameAreaX1, int coordGameAreaX2, int coordGameAreaY1, int coordGameAreaY2, int coinRadius){

	// red holes to change table
	fillPinballCircleWithId(coordHoleLeftX, coordHoleLeftY, 10, Red, OBJECT_CHANGE_TABLE_LEFT);
	gdispDrawCircle(coordHoleRightX, coordHoleRightY, 10, Black);

	fillPinballCircleWithId(coordHoleRightX, coordHoleRightY, 10, Red, OBJECT_CHANGE_TABLE_RIGHT);
	gdispDrawCircle(coordHoleRightX, coordHoleRightY, 10, Black);

	drawPinballThickLineRound(coordHoleLeftX - 30, 32, coordHoleLeftX + 30, 35, Black, 5); 		// left bowl
	drawPinballThickLineRound(coordHoleLeftX + 30, 35, coordHoleLeftX + 30, 60, Black, 5);		// left bowl
	drawPinballThickLineRound(coordHoleRightX - 30, 35, coordHoleRightX + 30, 32, Black, 5);		// right bowl
	drawPinballThickLineRound(coordHoleRightX - 30 , 35, coordHoleRightX - 30, 60, Black, 5);		// right bowl

	// upper green bumper
	drawPinballThickLineWithId(coordHoleLeftX + 30 + 2, 35 + 2, coordHoleLeftX + 30 + 2, 60 - 2, Green, 2, OBJECT_BIG_BONUS);// left bumper bowl
	drawPinballThickLineWithId(coordHoleRightX - 30 - 3, 35 + 2, coordHoleRightX - 30 - 3, 60 - 2, Green, 2, OBJECT_BIG_BONUS);// right bumper bowl

	// lower blue bumper
	drawPinballThickLineRound(coordGameAreaX1 + 50, coordGameAreaY2 - 75, coordGameAreaX1 + 50 + 40, coordGameAreaY2 - 55 , Black, 5); 		// left upper bumper
	drawPinballThickLineWithId(coordGameAreaX1 + 50 + 3, coordGameAreaY2 - 75, coordGameAreaX1 + 50 + 40, coordGameAreaY2 - 55 -2, Blue, 2, OBJECT_NORMAL);
	drawPinballThickLineRound(coordGameAreaX2 - 50 - 40, coordGameAreaY2 - 55, coordGameAreaX2 - 50, coordGameAreaY2 - 75, Black, 5);		// right upper bumper
	drawPinballThickLineWithId(coordGameAreaX2 - 50 - 40, coordGameAreaY2 - 55 - 2, coordGameAreaX2 - 50 - 3, coordGameAreaY2 - 75, Blue, 2, OBJECT_NORMAL);

	// circle bumper
	fillPinballCircleWithId(coordGameAreaX2 / 2, coordGameAreaY2 / 2, bumperRadiusTableOne, Orange, OBJECT_SMALL_BONUS);
	gdispDrawCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2, bumperRadiusTableOne, Black);


	bigAnimationTableOne(coordHoleLeftX, coordHoleRightX);			// draws boundaries, which disappear after hitting green bumper
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void bigAnimationTableOne(int coordHoleLeftX, int coordHoleRightX){
	if(startBigAnimationTableOne == 0){
		// closed holes for table changing
		drawPinballThickLineRound(coordHoleLeftX - 30, 63, coordHoleLeftX + 30, 60, Gray, 3); 		// left bowl
		drawPinballThickLineRound(coordHoleLeftX - 30, 35, coordHoleLeftX - 30, 63, Gray, 3);		// left bowl
		drawPinballThickLineRound(coordHoleRightX - 30, 60, coordHoleRightX + 30, 63, Gray, 3);		// right bowl
		drawPinballThickLineRound(coordHoleRightX + 30 , 35, coordHoleRightX + 30, 63, Gray, 3);		// right bowl


		drawPinballThickLineRound(0,100, 26, 140, Gray, 3);	// left gate
		drawPinballThickLineRound(265, 140, 290, 100, Gray, 3); // right gate

	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void drawTableTwo(int coordHoleLeftX, int coordHoleLeftY, int coordHoleRightX, int coordHoleRightY,
		int coordGameAreaX1, int coordGameAreaX2, int coordGameAreaY1, int coordGameAreaY2, int coinRadius){

	// red holes to change table
	fillPinballCircleWithId(coordGameAreaX2/2 - 20, coordGameAreaY1 + 20, 10, Red, OBJECT_CHANGE_TABLE_LEFT);
	gdispDrawCircle(coordGameAreaX2/2 - 20, coordGameAreaY1 + 20, 10, Black);
	fillPinballCircleWithId(coordGameAreaX2/2 + 20, coordGameAreaY1 + 20, 10, Red, OBJECT_CHANGE_TABLE_RIGHT);
	gdispDrawCircle(coordGameAreaX2/2 + 20, coordGameAreaY1 + 20, 10, Black);

	drawPinballThickLineRound(coordGameAreaX2/2, coordGameAreaY1, coordGameAreaX2/2, coordGameAreaY1 + 40, Black, 5);
	drawPinballThickLineRound(coordGameAreaX2/2 - 30, coordGameAreaY1, coordGameAreaX2/2 - 50, coordGameAreaY1 + 40, Black, 5);
	drawPinballThickLineRound(coordGameAreaX2/2 + 30, coordGameAreaY1, coordGameAreaX2/2 + 50, coordGameAreaY1 + 40, Black, 5);

	// lower bumper
	drawPinballThickLineRound(coordGameAreaX1 + 50, coordGameAreaY2 - 75, coordGameAreaX1 + 50 + 40, coordGameAreaY2 - 55 , Black, 5); 		// left upper bumper2s
	drawPinballThickLineWithId(coordGameAreaX1 + 50 + 3, coordGameAreaY2 - 75, coordGameAreaX1 + 50 + 40, coordGameAreaY2 - 55 -2, Blue, 2, OBJECT_NORMAL);
	drawPinballThickLineRound(coordGameAreaX2 - 50 - 40, coordGameAreaY2 - 55, coordGameAreaX2 - 50, coordGameAreaY2 - 75, Black, 5);		// right upper bumper
	drawPinballThickLineWithId(coordGameAreaX2 - 50 - 40, coordGameAreaY2 - 55 - 2, coordGameAreaX2 - 50 - 3, coordGameAreaY2 - 75, Blue, 2, OBJECT_NORMAL);

	// circle bumper
	fillPinballCircleWithId(coordGameAreaX2 / 2 - 50, coordGameAreaY2 / 2, 15, colorFillCircleTableTwo, OBJECT_SMALL_BONUS);
	gdispDrawCircle(coordGameAreaX2 / 2 - 50, coordGameAreaY2 / 2, 15, colorDrawCircleTableTwo);
	fillPinballCircleWithId(coordGameAreaX2 / 2 + 50, coordGameAreaY2 / 2, 15, colorFillCircleTableTwo, OBJECT_SMALL_BONUS);
	gdispDrawCircle(coordGameAreaX2 / 2 + 50, coordGameAreaY2 / 2, 15, colorDrawCircleTableTwo);


	// coins
	fillPinballCircleWithId(coordGameAreaX2 / 2, coordGameAreaY2 / 2 - 60, coinRadiusHittable, Yellow, OBJECT_BIG_BONUS);	//hittable coin

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
/*------------------------------------------------------------------------------------------------------------------------------*/
// multi player table functions
/*------------------------------------------------------------------------------------------------------------------------------*/
void drawAdditionalLeverMP(int coordRightLeverX1, int coordRightLeverY1, int coordRightLeverX2, int coordRightLeverY2, int coordLeftLeverX1,
		int coordLeftLeverY1, int coordLeftLeverX2, int coordLeftLeverY2, int coordRightLeverY1Idle, int coordLeftLeverY2Idle, int thickLever){

	//additional pair of lever
	// lever
	drawPinballThickLineRound(coordRightLeverX1, coordRightLeverY1 - 90, coordRightLeverX2, coordRightLeverY2 - 90, Green, thickLever);
	drawPinballThickLineRound(coordLeftLeverX1, coordLeftLeverY1 - 90, coordLeftLeverX2, coordLeftLeverY2 - 90, Blue, thickLever);

	// obstacles horizontal
	drawPinballThickLineRound(coordRightLeverX2, coordRightLeverY2 - 90, coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2) - 90, Black, thickLever);
	drawPinballThickLineRound(coordLeftLeverX1 - 50, coordLeftLeverY1 - (coordLeftLeverY2Idle - coordLeftLeverY1) - 90, coordLeftLeverX1, coordLeftLeverY1 - 90, Black, thickLever);

}
/*------------------------------------------------------------------------------------------------------------------------------*/
void drawTableMultiPlayer(int coordHoleLeftX, int coordHoleLeftY, int coordHoleRightX, int coordHoleRightY,
		int coordGameAreaX1, int coordGameAreaX2, int coordGameAreaY1, int coordGameAreaY2, int coinRadius){

	drawPinballThickLineRound(coordGameAreaX2 / 2 - 80, 50, coordGameAreaX2 / 2, 10, Black, 5);		// left side big bowl
	drawPinballThickLineRound(coordGameAreaX2 / 2, 10, coordGameAreaX2 / 2 + 80, 50, Black, 5);		// right side big bowl
	// bowl bumper
	drawPinballThickLineWithId(coordGameAreaX2 / 2 - 80 + 2, 50,coordGameAreaX2 / 2 + 2, 10, Blue, 2, OBJECT_NORMAL_PLAYER_BLUE);// left bumper
	drawPinballThickLineWithId(coordGameAreaX2 / 2 - 2, 10, coordGameAreaX2 / 2 + 80 - 2, 50, Green, 2, OBJECT_NORMAL_PLAYER_GREEN);// right bumper

	// upper bumper
	drawPinballThickLineRound(coordGameAreaX1, 40, coordGameAreaX1 + 15,60, Black, 5); 		// left upper bumper
	drawPinballThickLineWithId(coordGameAreaX1 + 2, 40,coordGameAreaX1 + 15 + 2, 60, Green, 2, OBJECT_SMALL_BONUS_PLAYER_GREEN);
	drawPinballThickLineRound(coordGameAreaX2 - 15, 60, coordGameAreaX2,40, Black, 5);		// right upper bumper
	drawPinballThickLineWithId(coordGameAreaX2 - 15 - 2, 60, coordGameAreaX2 - 2, 40, Blue, 2, OBJECT_SMALL_BONUS_PLAYER_BLUE);

	// round bumper
	fillPinballCircleWithId(coordGameAreaX2 / 2 - 80 , 20, 10, Blue, OBJECT_BIG_BONUS_PLAYER_BLUE);		//left bumper
	gdispDrawCircle(coordGameAreaX2 / 2 - 80, 20, 10, Black);

	fillPinballCircleWithId(coordGameAreaX2 / 2 + 80, 20, 10, Green, OBJECT_BIG_BONUS_PLAYER_GREEN);		//right bumper
	gdispDrawCircle(coordGameAreaX2 / 2 + 80, 20, 10, Black);

	// coins
	gdispFillCircle(coordGameAreaX2 / 2, coordGameAreaY2 / 2 + 60, coinRadius, Yellow);
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
	gdispFillCircle(coordGameAreaX2 / 2 + 60, coordGameAreaY2 / 2 + 60, coinRadius, Yellow);
	gdispFillCircle(coordGameAreaX2 / 2 - 60, coordGameAreaY2 / 2 + 60, coinRadius, Yellow);
	gdispFillCircle(coordGameAreaX2 / 2 + 60, coordGameAreaY2 / 2 - 60, coinRadius, Yellow);
	gdispFillCircle(coordGameAreaX2 / 2 - 60, coordGameAreaY2 / 2 - 60, coinRadius, Yellow);

}
/*------------------------------------------------------------------------------------------------------------------------------*/
void drawTableMultiPlayerEssentials(int coordStartAreaX, int startAreaSize, int coordGameAreaY2, int coordGameAreaX2,
		int coordRightLeverX1, int coordRightLeverX2, int coordRightLeverY1, int coordRightLeverY2, int coordLeftLeverX1, int coordLeftLeverX2,
		int coordLeftLeverY1, int coordLeftLeverY2, int coordRightLeverY1Idle, int coordLeftLeverY2Idle, int thickLever){

	// lever
	drawPinballThickLineRound(coordRightLeverX1, coordRightLeverY1, coordRightLeverX2, coordRightLeverY2, Green, thickLever);
	drawPinballThickLineRound(coordLeftLeverX1, coordLeftLeverY1, coordLeftLeverX2, coordLeftLeverY2, Blue, thickLever);

	// obstacles horizontal
	drawPinballThickLineRound(coordRightLeverX2, coordRightLeverY2, coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2), Black, thickLever);
	drawPinballThickLineRound(coordLeftLeverX1 - 50, coordLeftLeverY1 - (coordLeftLeverY2Idle - coordLeftLeverY1), coordLeftLeverX1, coordLeftLeverY1, Black, thickLever);

	// obstacles vertical
	drawPinballThickLineRound(coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2), coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2) - 50, Black, thickLever);
	drawPinballThickLineRound(coordLeftLeverX1 - 50, coordLeftLeverY1 - (coordLeftLeverY2Idle - coordLeftLeverY1), coordLeftLeverX1 - 50, coordLeftLeverY1 - (coordLeftLeverY2Idle - coordLeftLeverY1) - 50, Black, thickLever);
	registerCollisionCircle(coordRightLeverX2 + 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2), 5, OBJECT_ENV); //additional circle to avoid stuck ball
	registerCollisionCircle(coordLeftLeverX1 - 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2), 5, OBJECT_ENV);  //additional circle to avoid stuck ball
		if (DEBUG) {
			gdispFillCircle(coordRightLeverX2 + 50,	coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2), 5, Red);
			gdispFillCircle(coordLeftLeverX1 - 50, coordRightLeverY2 - (coordRightLeverY1Idle - coordRightLeverY2), 5, Red);
		}
	// boundaries
	drawPinballThickLine(coordRightLeverX1 + 10, coordGameAreaY2, coordGameAreaX2, coordRightLeverY2 + 10, Black, 5);
	drawPinballThickLine(0, coordLeftLeverY1 + 10, coordLeftLeverX2 - 10, coordGameAreaY2, Black, 5);

	// level frame
	drawPinballThickLine(0, 0, displaySizeX, 0, Black, 3);
	drawPinballThickLine(0, 0, 0, displaySizeY, Black, 3);

	// start area
	drawPinballThickLine(coordStartAreaX, 0, coordStartAreaX, 60,Black, 5); //upper lines
	drawPinballThickLine(coordStartAreaX, 58,coordStartAreaX + startAreaSize / 3, 63, Black, 5); //upper lines curve
	drawPinballThickLine(coordStartAreaX + startAreaSize / 3, 63,coordStartAreaX + 2 * startAreaSize / 3, 73, Black, 5); //upper lines curve
	drawPinballThickLine(coordStartAreaX + 2 * startAreaSize / 3, 73,coordStartAreaX + 3 * startAreaSize / 3, 90, Black, 5); //upper lines curve
	drawPinballThickLine(coordStartAreaX, 90, coordStartAreaX,displaySizeY, Black, 5); //lower lines
	drawPinballThickLine(displaySizeX, 0, displaySizeX, displaySizeY,Black, 5);

	//start lever
	drawPinballThickLineRound(coordStartAreaX + 4, 200 + intGainStartLever, coordStartAreaX + 24, 200 + intGainStartLever, Red, 3);
	drawPinballThickLine(coordStartAreaX + 14, 200 + intGainStartLever, coordStartAreaX + 14, coordGameAreaY2, Black, 1);
}
/*------------------------------------------------------------------------------------------------------------------------------*/
// main draw functions
/*------------------------------------------------------------------------------------------------------------------------------*/
void drawMenu(){
	char str[100]; // buffer for messages to draw to display

	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");


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
/*------------------------------------------------------------------------------------------------------------------------------*/
void drawStats(int coordGameAreaX1, int coordGameAreaX2, int coordGameAreaY1, int coordGameAreaY2){
	char str[100]; // buffer for messages to draw to display

	font_t font1, font3; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");
	font3 = gdispOpenFont("DejaVuSans12*");

	//***STATS***
	sprintf(str, "LEVEL: %d", intPlayerLevel);
	gdispDrawString(coordGameAreaX1 + 5, coordGameAreaY1 + 5, str, font3,Black);

	sprintf(str, "SEC: %d", intPassedTime);
	gdispDrawString(coordGameAreaX2 - 60, coordGameAreaY2 - 10, str, font3,Black);
	sprintf(str, "FPS: %d", intFPS);
	gdispDrawString(coordGameAreaX1 + 5, coordGameAreaY2 - 10, str, font3,Black);
	sprintf(str, "LIFES: %d", intLifes);
	gdispDrawString(coordGameAreaX2 - 55, coordGameAreaY1 + 10, str, font3,Black);

	if(flagGameMode == 1){					// single player flag
		sprintf(str, "SCORE: %d", intScoreSingle);
		gdispDrawString(coordGameAreaX1 + 5, coordGameAreaY1 + 20, str, font3,Black);
	}else if (flagGameMode ==2){			// multi player flag
		sprintf(str, "SCORE: %d", intScoreMulti);
		gdispDrawString(coordGameAreaX1 + 5, coordGameAreaY1 + 20, str, font3,Black);
	}
}

/*------------------------------------------------------------------------------------------------------------------------------*/
// MAIN PROGRAM CODE STARTS HERE
/*------------------------------------------------------------------------------------------------------------------------------*/

int main() {
	// Initialize Board functions and graphics
	ESPL_SystemInit();

	// Initializes Draw Queue with 100 lines buffer
	JoystickQueue = xQueueCreate(100, 2 * sizeof(char));

	// Initializes Tasks with their respective priority

	xTaskCreate(TaskController, "TaskController", 1000, NULL, 9, &TaskControllerHandle);
	xTaskCreate(UserStats, "UserStats", 1000, NULL, 7, &UserStatsHandle);
	xTaskCreate(uartReceive, "uartReceive", 1000, NULL, 8, &uartReceiveHandle);

	// interface tasks
	xTaskCreate(checkJoystick, "checkJoystick", 1000, NULL, 6, &checkJoystickHandle);
	xTaskCreate(checkButton, "checkButton", 1000, NULL, 6, &checkButtonHandle);
	xTaskCreate(UserActions, "UserActions", 1000, NULL, 7, &UserActionsHandle);
	// drawing tasks
	xTaskCreate(drawTask, "drawTask", 1000, NULL, 8, &drawTaskHandle);
	// animation tasks
	xTaskCreate(AnimationTimerTask, "AnimationTimerTask", 1000, NULL, 3, &AnimationTimerTaskHandle);
	xTaskCreate(BallStuckTask, "BallStuckTask", 1000, NULL, 5,  &BallStuckTaskHandle);

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
void checkStart(int coordStartAreaX) {
	if (position[0] > coordStartAreaX + 2) {
		if (position[1] + BALL_RADIUS > 200 + intGainStartLever) {
			position[1] = 200 + intGainStartLever - BALL_RADIUS;
			position[0] = coordStartAreaX + 15;
			velocity[1] = -400;
		}
	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void checkLever(int coordX1, int coordX2, int coordY, int coordYIdle, int coordYTriggered, int coordY2, int id) {
	if (coordY != coordYTriggered) {
		for (int i = 0; i < 20; i++) {
			double range = coordYTriggered - coordYIdle;
			double y1 = coordYIdle + range / i;
			if (checkLineCollision(position[0], position[1], coordX1, y1, coordX2, coordY2)) {
				position[1] = coordYTriggered - 10;
				velocity[1] = -400;
				velocity[0] = 100;
				if (id != -1) {
					lastActivePlayer = id;
				}
			}
		}
	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/

//Moving parts
int coordRightLeverY1Triggered;
int coordRightLeverY1Idle;
float coordRightLeverX1;
int coordRightLeverY1;
int coordRightLeverX2;
int coordRightLeverY2;

int coordLeftLeverY2Triggered;
int coordLeftLeverY2Idle;
int coordLeftLeverX1;
int coordLeftLeverY1;
int coordLeftLeverX2;
int coordLeftLeverY2;


int multiplayerMaster = TRUE;

void sendArmCommand(uint8_t high) {
	UART_SendData(startByte);
	UART_SendData(1);
	UART_SendData(high);
	UART_SendData(stopByte);
}

void sendBallCommand() {
	UART_SendData(startByte);
	UART_SendData(2);
	UART_SendData(((uint16_t)position[0]) & 0xFF);
	UART_SendData((((uint16_t)position[0]) >> 8) & 0xFF);
	UART_SendData(((uint16_t)position[1]) & 0xFF);
	UART_SendData((((uint16_t)position[1]) >> 8) & 0xFF);
	UART_SendData(stopByte);
}

int statsCounter = 0;

void sendStatsCommand() {
	if (statsCounter++ > 5) {
		UART_SendData(startByte);
		UART_SendData(3);
		UART_SendData(((uint16_t)intScoreMulti) & 0xFF);
		UART_SendData((((uint16_t)intScoreMulti) >> 8) & 0xFF);
		UART_SendData(intPlayerLevel);
		UART_SendData(stopByte);
		statsCounter = 0;
	}
}

void sendPauseCommand(uint8_t screenBefore) {
	UART_SendData(startByte);
	UART_SendData(4);
	UART_SendData(screenBefore);
	UART_SendData(stopByte);
}

void sendStopPauseCommand(uint8_t screenBefore) {
	UART_SendData(startByte);
	UART_SendData(5);
	UART_SendData(screenBefore);
	UART_SendData(stopByte);
}

int heartbeatCounter = 0;
volatile TickType_t lastHeartbeat = 0;
int timeoutPause = 0;

void sendHeartbeat() {
	if (heartbeatCounter++ > 4) {
		UART_SendData(startByte);
		UART_SendData(6);
		UART_SendData(stopByte);
		heartbeatCounter = 0;
	}
}

void drawTask() {

	char str[100]; // buffer for messages to draw to display
	struct coord joystickPosition; // joystick queue input buffer

	// FPS
	TickType_t xLastWakeTime;
	TickType_t xWakeTime;
	xWakeTime = xTaskGetTickCount();
	xLastWakeTime = xWakeTime;


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
	coordRightLeverY1Triggered = 170;
	coordRightLeverY1Idle = 210;
	coordRightLeverX1 = coordGameAreaX2 / 2 + 20; //displaySizeX/2 + 10;
	coordRightLeverY1 = coordRightLeverY1Idle;
	coordRightLeverX2 = coordGameAreaX2 / 2 + 70; //displaySizeX/2 + 60;
	coordRightLeverY2 = 200;

	coordLeftLeverY2Triggered = 170;
	coordLeftLeverY2Idle = 210;
	coordLeftLeverX1 = coordGameAreaX2 / 2 - 70;
	coordLeftLeverY1 = 200;
	coordLeftLeverX2 = coordGameAreaX2 / 2 - 20;
	coordLeftLeverY2 = coordLeftLeverY2Idle;

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
			sprintf(str, "Credits: Simon Leier & Florian Geiser");
			gdispDrawString(20, 220, str, font2, Black);
			break;
		// draw menu
		case 2:
			drawMenu();
			break;
		case 3: // singleplayer  mode

			flagGameMode = 1; 				// sets game mode flag to single player

			/****** LEVEL *****/
			if (intActTable == intTableThree) {
				drawTableThree(coordHoleLeftX, coordHoleLeftY - 12, coordHoleRightX, coordHoleRightY - 12, coordGameAreaX1, coordGameAreaX2, coordGameAreaY1, coordGameAreaY2, coinRadius);
				drawAdditionalLeverSP(coordRightLeverX1, coordRightLeverY1, coordRightLeverX2, coordRightLeverY2, coordLeftLeverX1, coordLeftLeverY1, coordLeftLeverX2, coordLeftLeverY2, coordRightLeverY1Idle, coordLeftLeverY2Idle, thickLever);
				checkCloseStartArea(coordStartAreaX);

			} else if (intActTable == intTableOne) {
				drawTableOne(coordHoleLeftX, coordHoleLeftY, coordHoleRightX, coordHoleRightY, coordGameAreaX1, coordGameAreaX2, coordGameAreaY1, coordGameAreaY2, coinRadius);
				checkCloseStartArea(coordStartAreaX);
			} else if (intActTable == intTableTwo) {
				drawTableTwo(coordHoleLeftX, coordHoleLeftY, coordHoleRightX, coordHoleRightY, coordGameAreaX1, coordGameAreaX2, coordGameAreaY1, coordGameAreaY2, coinRadius);
				checkCloseStartArea(coordStartAreaX);
			}
			// necessary for each table
			drawTableEssentials(coordStartAreaX, startAreaSize, coordGameAreaY2, coordGameAreaX2,
					coordRightLeverX1, coordRightLeverX2, coordRightLeverY1, coordRightLeverY2, coordLeftLeverX1, coordLeftLeverX2, coordLeftLeverY1, coordLeftLeverY2, coordRightLeverY1Idle, coordLeftLeverY2Idle, thickLever);
			checkStart(coordStartAreaX);
			calculatePhysics(xWakeTime - xLastWakeTime);
			drawBall();


			drawStats(coordGameAreaX1, coordGameAreaX2, coordGameAreaY1, coordGameAreaY2);

			//***INPUTS***
			if (intButtonB) {
				checkLever(coordRightLeverX1, coordRightLeverX2, coordRightLeverY1, coordRightLeverY1Idle,coordRightLeverY1Triggered, coordRightLeverY2, -1);
				if (intActTable == 3) {
					checkLever(coordRightLeverX1, coordRightLeverX2, coordRightLeverY1 - 90, coordRightLeverY1Idle - 90, coordRightLeverY1Triggered - 90, coordRightLeverY2 - 90, -1);
				}
				coordRightLeverY1 = coordRightLeverY1Triggered;
			} else {
				coordRightLeverY1 = coordRightLeverY1Idle;
			}

			if (intButtonD) {
				checkLever(coordLeftLeverX1, coordLeftLeverX2, coordLeftLeverY2, coordLeftLeverY2Idle, coordLeftLeverY2Triggered, coordRightLeverY1, -1);
				if (intActTable == 3) {
					checkLever(coordLeftLeverX1, coordLeftLeverX2, coordLeftLeverY2 - 90, coordLeftLeverY2Idle - 90, coordLeftLeverY2Triggered - 90, coordRightLeverY1 - 90, -1);
				}
				coordLeftLeverY2 = coordLeftLeverY2Triggered;
			} else {
				coordLeftLeverY2 = coordLeftLeverY2Idle;
			}

			break;


		case 4: // multiplayer mode

			flagGameMode = 2; 				// sets game mode flat to multi player

			/****** LEVEL *****/
			drawTableMultiPlayer(coordHoleLeftX, coordHoleLeftY, coordHoleRightX, coordHoleRightY, coordGameAreaX1, coordGameAreaX2, coordGameAreaY1, coordGameAreaY2, coinRadius);

			drawTableMultiPlayerEssentials(coordStartAreaX, startAreaSize, coordGameAreaY2, coordGameAreaX2,
								coordRightLeverX1, coordRightLeverX2, coordRightLeverY1, coordRightLeverY2, coordLeftLeverX1, coordLeftLeverX2, coordLeftLeverY1, coordLeftLeverY2, coordRightLeverY1Idle, coordLeftLeverY2Idle, thickLever);
			drawAdditionalLeverMP(coordRightLeverX1, coordRightLeverY1, coordRightLeverX2, coordRightLeverY2, coordLeftLeverX1, coordLeftLeverY1, coordLeftLeverX2, coordLeftLeverY2, coordRightLeverY1Idle, coordLeftLeverY2Idle, thickLever);
			drawStats(coordGameAreaX1, coordGameAreaX2, coordGameAreaY1, coordGameAreaY2);
			checkStart(coordStartAreaX);
			checkCloseStartArea(coordStartAreaX);
			if (multiplayerMaster) {
				calculatePhysics(xWakeTime - xLastWakeTime);

				sendBallCommand();
				sendStatsCommand();
			} else {
				sendHeartbeat();
			}
			drawBall();

			//***INPUTS***
			if (intButtonB || intButtonD) {
				if (multiplayerMaster && coordRightLeverY1 != coordRightLeverY1Triggered) {
					sendArmCommand(1);
					checkLever(coordRightLeverX1, coordRightLeverX2, coordRightLeverY1, coordRightLeverY1Idle,
											coordRightLeverY1Triggered, coordRightLeverY2, 1);
					checkLever(coordRightLeverX1, coordRightLeverX2, coordRightLeverY1 - 90, coordRightLeverY1Idle - 90,
											coordRightLeverY1Triggered - 90, coordRightLeverY2 - 90, 1);
					coordRightLeverY1 = coordRightLeverY1Triggered;
				} else if (!multiplayerMaster && coordLeftLeverY2 != coordLeftLeverY2Triggered) {
					sendArmCommand(1);
					coordLeftLeverY2 = coordLeftLeverY2Triggered;
				}
			} else {
				if (multiplayerMaster && coordRightLeverY1 != coordRightLeverY1Idle) {
					sendArmCommand(0);
					coordRightLeverY1 = coordRightLeverY1Idle;
				} else if (!multiplayerMaster && coordLeftLeverY2 != coordLeftLeverY2Idle) {
					sendArmCommand(0);
					coordLeftLeverY2 = coordLeftLeverY2Idle;
				}
			}

			int diff = xWakeTime - lastHeartbeat;
			if (diff > 200) {
				timeoutPause = 1;
				intScreenBeforePause = intDrawScreen;
				intDrawScreen = 6;
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
			gdispClear(Gray);
			sprintf(str, "PAUSE");
			gdispDrawString(coordGameAreaX2 / 2 - 10, coordGameAreaY2 / 2 - 10,	str, font1, Red);
			sendHeartbeat();
			int pauseDiff = xWakeTime - lastHeartbeat;
			if (timeoutPause == 1 && pauseDiff < 200) {
				timeoutPause = 0;
				intDrawScreen = intScreenBeforePause;
			}
			break;
		default:
			break;
		}

		// draw game over screen
		if(gameover){
			gdispClear(Black);
			sprintf(str, "GAME OVER");
			gdispDrawString(coordGameAreaX2 / 2 - 40, coordGameAreaY2 / 2 - 10,	str, font1, Red);
			sprintf(str, "Press E to continue");
			gdispDrawString(coordGameAreaX2 / 2 - 40, 220, str, font2, Red);
		}

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

void checkButton() {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t tickFramerate = 100;

	int8_t FlagButtonA = 0;
	int8_t FlagButtonB = 0;
	int8_t FlagButtonC = 0;
	int8_t FlagButtonD = 0;
	int8_t FlagButtonE = 0;
	int8_t FlagButtonK = 0;

	while (TRUE) {
		vTaskDelayUntil(&xLastWakeTime, tickFramerate);		//checks buttons every 100 ticks

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

	}
}

/*------------------------------------------------------------------------------------------------------------------------------*/
void uartReceive() {
	char input;
	uint8_t pos = 0;
	uint8_t command = 0;
	char buffer[12];
	TickType_t xWakeTime;

	while (TRUE) {
		// wait for data in queue
		xQueueReceive(ESPL_RxQueue, &input, portMAX_DELAY);
		xWakeTime = xTaskGetTickCount();

		lastHeartbeat = xWakeTime;
		if (command == 1) {
			if (multiplayerMaster) {
				if (input) {
					checkLever(coordLeftLeverX2, coordLeftLeverX1, coordLeftLeverY2, coordLeftLeverY2Idle,
											coordLeftLeverY2Triggered, coordRightLeverY1, 0);
					checkLever(coordLeftLeverX2, coordLeftLeverX1, coordLeftLeverY2 - 90, coordLeftLeverY2Idle - 90,
											coordLeftLeverY2Triggered - 90, coordRightLeverY1 - 90, 0);
					coordLeftLeverY2 = coordLeftLeverY2Triggered;
				} else {
					coordLeftLeverY2 = coordLeftLeverY2Idle;
				}
			} else {
				if (input) {
					coordRightLeverY1 = coordRightLeverY1Triggered;
				} else {
					coordRightLeverY1 = coordRightLeverY1Idle;
				}
			}
			pos = 0;
			command = 0;
		} else if (command == 2) {
			if (pos < 6) {
				buffer[pos++ - 2] = input;
			} else {
				position[0] = ((uint16_t)(buffer[1]) << 8) | (uint16_t)(buffer[0]);
				position[1] = ((uint16_t)(buffer[3]) << 8) | (uint16_t)(buffer[2]);
				pos = 0;
				command = 0;
			}

		} else if (command == 3) {
			if (pos < 5) {
				buffer[pos++ - 2] = input;
			} else {
				intScoreMulti = ((uint16_t)(buffer[1]) << 8) | (uint16_t)(buffer[0]);
				intPlayerLevel = buffer[2];
				pos = 0;
				command = 0;
			}
		} else if (command == 4) {
			intScreenBeforePause = input;
			intDrawScreen = 6;
			pos = 0;
			command = 0;
		} else if (command == 5) {
			intDrawScreen = input;
			pos = 0;
			command = 0;
		} else if (command == 6) {
			pos = 0;
			command = 0;
		} else if (pos == 1) {
			command = input;
			pos++;
		} else if (command == 0 && input == startByte) {
			pos = 1;
		} else {
			pos++;
			if (input == stopByte || pos > 10) {
				pos = 0;
				command = 0;
			}
		}
	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
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
				sendPauseCommand(intScreenBeforePause);
			}
			else {
				intDrawScreen = intScreenBeforePause;
				sendStopPauseCommand(intScreenBeforePause);
			}

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
			velocityMultiplier = 1.5;			// increasing ball speed
		} else if (intScoreSingle >= 3000 && intPlayerLevel <= 2) {
			intPlayerLevel = 3;
			velocityMultiplier = 2;				// increasing ball speed
		}

		// time
		vTaskDelayUntil(&xLastWakeTime, 1000);
		if ((intDrawScreen == 3 || intDrawScreen == 4) && gameover != 1) { //if single player or multi player selected, start timer
			intPassedTime++;
		}

		// reset ball position after switching to menu
		if (intDrawScreen != 3 && intDrawScreen != 4 && intDrawScreen != 6){
			position[0] = 310; 		// start position x
			position[1] = 150;		// start position y
			velocity[0] = 0;		// start velocity x
			velocity[1] = 0;		// start velocity y
			intStartAreaClosed = 0;
		}

		// life decreasing
		if((position[0] > 320 || position[0] < 0 || position[1] > 240 || position[1] < 0) && intLifes > 0){
			intLifes--;
			coinRadiusHittable = 5;	// big animation table 2, reset bumper size
			position[0] = 310; 		// start position x
			position[1] = 150;		// start position y
			velocity[0] = 0;		// start velocity x
			velocity[1] = 0;		// start velocity y
			intStartAreaClosed = 0;
		}

		// game ends
		if (intLifes <= 0) { //if no life left and we're in a game
			gameover = 1;		// game over flag

			// to get back to the menu after single player game ended
			if(intButtonE && flagGameMode == 1){

				gameover = 0;
				intLifes = 3;

				// refresh high score
				if (intScoreSingle >= intScoreFirstSingle) {	//replace first
					intScoreThirdSingle = intScoreSecondSingle;
					intScoreSecondSingle = intScoreFirstSingle;
					intScoreFirstSingle = intScoreSingle;
				} else if (intScoreSingle >= intScoreSecondSingle) {//replace second
					intScoreThirdSingle = intScoreSecondSingle;
					intScoreSecondSingle = intScoreSingle;
				} else if (intScoreSingle >= intScoreThirdSingle) {	//replace third
					intScoreThirdSingle = intScoreSingle;
				}

				intScoreSingle = 0;			// reset stats
				intPassedTime = 0;
				intPlayerLevel = 1;
				coinRadiusHittable = 5;
				flagGameMode = 0;
				velocityMultiplier = 1;
				startBigAnimationTableOne = 0;
				startBigAnimationTableTwo = 0;
				startBigAnimationTableThree = 0;

			}else if(intButtonE && flagGameMode == 2){

				gameover = 0;
				intLifes = 3;

				// refresh high score
				if (intScoreMulti >= intScoreFirstMulti) {	//replace first
					intScoreThirdMulti = intScoreSecondMulti;
					intScoreSecondMulti = intScoreFirstMulti;
					intScoreFirstMulti = intScoreMulti;
				} else if (intScoreMulti >= intScoreSecondMulti) {//replace second
					intScoreThirdMulti = intScoreSecondMulti;
					intScoreSecondMulti = intScoreMulti;
				} else if (intScoreMulti >= intScoreThirdSingle) {	//replace third
					intScoreThirdMulti = intScoreMulti;
				}

				intScoreMulti = 0;			// reset stats
				intPassedTime = 0;
				intPlayerLevel = 1;
				flagGameMode = 0;
				velocityMultiplier = 1;
			}


		}
	}
}

/*------------------------------------------------------------------------------------------------------------------------------*/
void AnimationTimerTask(){
	while (1) {
		// big animation for table two
		if(startBigAnimationTableTwo){
			vTaskDelay(50);
			coinRadiusHittable = coinRadiusHittable + 5;		// coin size on table increases
			startBigAnimationTableTwo = 0;
		}
		// big animation for table three
		if(startBigAnimationTableThree){
			for (int i = 0; i < 9; i++) {
				vTaskDelay(500);
				if (animationCounterTableThree == 3) {
					animationCounterTableThree = 1;
				} else if (animationCounterTableThree == 1) {
					animationCounterTableThree = 2;
				} else if (animationCounterTableThree == 2) {
					animationCounterTableThree = 3;
				}
			}
			startBigAnimationTableThree = 0;
		}

		vTaskDelay(100);
	}

}
/*------------------------------------------------------------------------------------------------------------------------------*/
void BallStuckTask(){

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	double velocityOld[] = {0, 0};
	double positionOld[] = {310, 150};

	while (TRUE) {
		//vTaskDelayUntil(&xLastWakeTime, 100);

		//if no lever triggered, position doesn't change and not in start area (290px x-coord)
		if (positionOld[0] == position[0] && positionOld[1] == position[1] && position[0] < 290 && !intButtonD && !intButtonB ){
			//set ball back to the last moving position

			if(position[1] < 45){					//increase y if ball is near the top of the table
				position[0] = positionOld[0] - 1;
				position[1] = positionOld[1] + 1;
			}
			else if (position[0] >= 145){			//decrease y & decrease x if ball is on the right table side
				position[0] = positionOld[0] - 1;
				position[1] = positionOld[1] - 1;
			} else if(position[0] < 145){			//decrease y & increase x if ball is on the left table side
				position[0] = positionOld[0] + 1;
				position[1] = positionOld[1] - 1;
			}
			velocity[0] = -velocityOld[0] * 0.5;
			velocity[1] = -velocityOld[1] * 0.5;

		} else{
			positionOld[0] = position[0];
			positionOld[1] = position[1];
			velocityOld[0] = velocity[0];
			velocityOld[1] = velocity[1];
		}
		vTaskDelay(150);
	};
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
