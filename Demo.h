/**
 * Function definitions for the main project file.
 *
 * @author: Jonathan Müller-Boruttau,
 * 			Tobias Fuchs tobias.fuchs@tum.de,
 * 			Nadja Peters nadja.peters@tum.de (RCS, TUM)
 */

#ifndef Demo_INCLUDED
#define Demo_INCLUDED

struct coord {
	uint8_t x;
	uint8_t y;
};

void uartReceive();
void sendLine(struct coord coord_1, struct coord coord_2);
void checkJoystick();
void drawTask();
void CircleAppear();
void TaskController();
void countButtonA();
void countButtonB();
void DisplayFPS();
void resetCountButton();
void controllableCounter();
void PriorityOneTask();
void PriorityTwoTask();
void PriorityThreeTask();
void PriorityFourTask();
void PriorityOutputTask();
//void sendButtons();
void CircleDisappearStatic();


#endif
