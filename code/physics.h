/**
 * Function definitions for the physics file of ESPinball.
 *
 * @author: Simon Leier & Florian Geiser
 *
 */
#ifndef Physics_INCLUDED
#define Physics_INCLUDED

#define M_PI 3.14159265358979323846

#include <math.h>

#define BALL_RADIUS 6
#define DEBUG TRUE

#define OBJECT_NONE -1
#define OBJECT_ENV 0
#define OBJECT_NORMAL 1
#define OBJECT_SMALL_BONUS 2
#define OBJECT_BIG_BONUS 3
#define OBJECT_CHANGE_TABLE_LEFT 4
#define OBJECT_CHANGE_TABLE_RIGHT 5
#define OBJECT_NORMAL_PLAYER_BLUE 6
#define OBJECT_NORMAL_PLAYER_GREEN 7
#define OBJECT_SMALL_BONUS_PLAYER_BLUE 8
#define OBJECT_SMALL_BONUS_PLAYER_GREEN 9
#define OBJECT_BIG_BONUS_PLAYER_BLUE 10
#define OBJECT_BIG_BONUS_PLAYER_GREEN 11

/**
 * Calculates the distance between two vectors.
 */
#define DIST(x1, y1, x2, y2) sqrt(((x1 - x2) * (x1 - x2)) + ((y1 - y2) * (y1 - y2)))

/**
 * Calculates the length of a vector.
 */
#define LEN(v) sqrt(v[0] * v[0] + v[1] * v[1])

/**
 * Calculates the dot product of a vector.
 */
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

extern double velocityMultiplier;
extern double velocity[];
extern double position[];
extern double startposition[];
extern double gravity;

void calculatePhysics(int deltaTime);

int checkCollision(uint16_t positionX, uint16_t positionY);
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
