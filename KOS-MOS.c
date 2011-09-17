/*
 ============================================================================
 Name        : KOS-MOS.c
 Author      : Matthew Guze, Riley Nold
 Version     : 0.1
 Copyright   : 2010
 Description : 2-Connect-2 AI for SENG 420
 ============================================================================
 */

#include "include/globals.h"
#include "include/gamerules.h"
#include "include/gameio.h"
#include "include/gamebrain.h"

int main(void)
{
	// example string for testing: (7,6,6,600,290,20,r,b,s,s,s,s,b,b,r,g,s,s,b,r,b,b,g,s,r,g,r,b,s,s,r,r,g,r,r,s,r,s,s,s,s,s,b,s,s,s,s,s)
	// example string for testing: (7,6,6,600,290,20,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s,s)

	char* inputVector = readInBoardVector();
	Board *board = readInBoard(inputVector);
	//printBoard(board);
	Move move = getNextMove(board);
	outputMove(move);
	free(inputVector);
	return EXIT_SUCCESS;
}
