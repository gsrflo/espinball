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
void checkButton();
void drawTask();
void TaskController();
void UserStats();



#endif
