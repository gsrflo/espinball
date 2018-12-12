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

#define BALL_RADIUS 10
#define GRAVITY 200

#define COLLISION_OBJECT_RECT 0
#define COLLISION_OBJECT_CIRCLE 1
#define COLLISION_OBJECT_POLYGON 2

#define DIST(x1, y1, x2, y2) sqrt(((x1 - x2) * (x1 - x2)) + ((y1 - y2) * (y1 - y2)))
#define LEN(v) sqrt(v[0] * v[0] + v[1] * v[1])
#define DOT_PRODUCT(v1, v2) v1[0] * v2[0] + v1[1] * v2[1]

typedef struct collision_circle {
	uint16_t x;
	uint16_t y;
	uint8_t radius;
} collision_circle;

typedef struct collision_poly {
	uint8_t pointCount;
	uint16_t points[10];
} collision_poly;

void calculatePhysics(int deltaTime);

uint8_t checkCollision(uint16_t positionX, uint16_t positionY);
uint8_t checkRectangleCollision(uint16_t positionX, uint16_t positionY, collision_rect *rect);
uint8_t checkCircleCollision(uint16_t positionX, uint16_t positionY, collision_circle *circle);
uint8_t checkPolygonCollision(uint16_t positionX, uint16_t positionY, collision_poly *poly);
uint8_t checkLineCollision(uint16_t positionX, uint16_t positionY, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

float dist(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

void drawBall();
void drawBitmap(uint8_t bitmap[], uint16_t width, uint16_t height);

void registerCollisionCircle(uint16_t x, uint16_t y, uint8_t radius);
void registerCollisionRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void registerCollisionLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void registerCollisionPolygon(point *points, uint8_t pointCount);

uint8_t checkBallCollision(uint16_t newPositionX, uint16_t newPositionY);
//void drawTask();
//void calculatePhysics(int deltaTime);
//void drawBall();
//void drawBitmap(uint8_t bitmap[], uint16_t width, uint16_t height)
//uint8_t checkColissionWithBackground(uint16_t newPositionX, uint16_t newPositionY);

#endif
