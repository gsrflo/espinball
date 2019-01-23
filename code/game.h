/**
 * Function definitions for the main project file of ESPinball.
 *
 * @author: Simon Leier & Florian Geiser
 *
 *
 */

#ifndef Demo_INCLUDED
#define Demo_INCLUDED

#include "physics.h"

struct coord {
	uint8_t x;
	uint8_t y;
};

void uartReceive();
void sendLine(struct coord coord_1, struct coord coord_2);
void checkJoystick();
void checkButton();
void drawTask();
void TaskController();
void UserStats();
void UserActions();
void AnimationTimerTask();
void BallStuckTask();


#endif
