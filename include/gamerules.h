/*
 * gamerules.h
 *
 *  Created on: May 10, 2010
 */

#ifndef GAMERULES_H_
#define GAMERULES_H_

#define WIN_NUMBER 4

typedef struct {
	int width;
	int height;
	int lastColumn;
	int totalGameTime;
	int playerOneTime;
	int lastMoveTime;
	char** boardMatrix; // Usage: boardMatrix[column][row]
} Board;

typedef struct {
	int column;
	char colour;
} Move;

int isWin(char* boardVector);
Board* readInBoard(char* boardVector);
Board* initializeBoard(int width, int height, int lastColumn, int totalGameTime, int playerOneTime, int lastMoveTime);
int checkBoardForWin(Board* board);
void deleteBoard(Board* board);
void printBoard(Board* board);
int findLastRow(Board *board);
int isSpaceInGrid(Board *board, int column, int row);
int emptyColumns(Board *board);

Move *getLegalMoves(Board *board, char playerTurn);

#endif /* GAMERULES_H_ */
