/*
 * gameio.c
 *
 *  Created on: May 11, 2010
 */

#include "include/globals.h"
#include "include/gameio.h"

char* readInBoardVector()
{
	char* inputVector = malloc(sizeof(char) * MAX_VECTOR_LENGTH);
	int i = 0;
	char c = getchar();
	while (c != ')')
	{
		inputVector[i++] = c;
		c = getchar();
	}
	inputVector[i++] = '\0';
	return inputVector;
}

void outputMove(Move move)
{
	// add 1 to the column since the AI indexes columns from zero, but the game indexes from 1
	fprintf(stdout, "(%d,%c)", move.column + 1, move.colour);
}
