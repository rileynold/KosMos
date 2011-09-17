/*
 * gamebrain.h
 *
 *  Created on: May 25, 2010
 *      Author: matthewguze
 */

#ifndef GAMEBRAIN_H_
#define GAMEBRAIN_H_

#define STARTING_DEPTH 6
#define MAX_DEPTH 20

#include "gamerules.h"

typedef struct
{
	int greenPoint;
	int otherPoint;
	int youPoint;
} PointValue;



Move getNextMove(Board* board);

#endif /* GAMEBRAIN_H_ */
