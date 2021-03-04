#ifndef RAY_H
#define RAY_H

#include <stdbool.h>
#include <limits.h>
#include "defs.h"
#include "player.h"
#include "graphics.h"

typedef struct {
	float rayAngle;
	float wallHitX;
	float wallHitY;
	float distance;
	int wallHitContent;
	bool wasHitVertical;
} ray_t;

//　Not initialized　Variable.
extern ray_t rays[NUM_RAYS];

void normalizeAngle(float *angle);
float distanceBetweenPoints(float x1, float y1, float x2, float y2);
void castAllRays(void);
void castRay(float rayAngle, int stripId);
void renderRays(void);

#endif