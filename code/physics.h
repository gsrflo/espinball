/**
 * Function definitions for the main project file.
 *
 * @author: Jonathan Müller-Boruttau,
 * 			Tobias Fuchs tobias.fuchs@tum.de,
 * 			Nadja Peters nadja.peters@tum.de (RCS, TUM)
 */

#ifndef Physics_INCLUDED
#define Physics_INCLUDED

#define M_PI 3.14159265358979323846

#include <math.h>

#define BALL_RADIUS 6
#define GRAVITY 280
#define DEBUG TRUE

#define OBJECT_ENV 0
#define OBJECT_NORMAL 1
#define OBJECT_BONUS 2

#define DIST(x1, y1, x2, y2) sqrt(((x1 - x2) * (x1 - x2)) + ((y1 - y2) * (y1 - y2)))
#define LEN(v) sqrt(v[0] * v[0] + v[1] * v[1])
#define DOT_PRODUCT(v1, v2) v1[0] * v2[0] + v1[1] * v2[1]

typedef struct collision_circle {
	uint8_t id;
	uint16_t x;
	uint16_t y;
	uint8_t radius;
} collision_circle;

typedef struct collision_poly {
	uint8_t id;
	uint8_t pointCount;
	uint16_t points[10];
} collision_poly;

extern double velocity[];
extern double position[];

void calculatePhysics(int deltaTime);

uint8_t checkCollision(uint16_t positionX, uint16_t positionY);
uint8_t checkCircleCollision(uint16_t positionX, uint16_t positionY, collision_circle *circle);
uint8_t checkPolygonCollision(uint16_t positionX, uint16_t positionY, collision_poly *poly);
uint8_t checkLineCollision(uint16_t positionX, uint16_t positionY, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

void drawBall();

void registerCollisionCircle(uint16_t x, uint16_t y, uint8_t radius, uint8_t id);
void registerCollisionRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t id);
void registerCollisionLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t id);
void registerCollisionPolygon(point *points, uint8_t pointCount, uint8_t id);
void resetCollisionObjects();

#endif
