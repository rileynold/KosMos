/*
 * gamerules.c
 *
 *  Created on: May 10, 2010
 */

#include "include/globals.h"
#include "include/gamerules.h"

#define ABAB_WIN 3
#define ABBA_WIN 4
#define AABB_WIN 5

void checkLineForMatch(char* string, char activePiece);

// given a board vector that is passed in (such as through stdin), return if there is a win or not
int isWin(char* boardVector)
{
	Board* board = readInBoard(boardVector);
	int result = checkBoardForWin(board);
	deleteBoard(board);
	return result;
}

// given a board, print it out so we can visualize the board
void printBoard(Board* board)
{
	fprintf(stderr, "%d x %d board, last column played was %d\n", board->width, board->height, board->lastColumn + 1);
	fprintf(stderr, "Total game time:       %d\n", board->totalGameTime);
	fprintf(stderr, "Player one move time:  %d\n", board->playerOneTime);
	fprintf(stderr, "Last move time:        %d\n", board->lastMoveTime);

	int i, j;
	for (j = board->height - 1; j >= 0; j--)
	{
		for (i = 0; i < board->width; i++)
		{
			fprintf(stderr, "%c ", board->boardMatrix[i][j]);
		}
		fprintf(stderr, "\n");
	}

	for (i = 0; i < board->width; i++)
	{
		if (board->lastColumn == i)
		{
			fprintf(stderr, "^ ");
		}
		else
		{
			fprintf(stderr, "  ");
		}
	}
	fprintf(stderr, "\n");
}

// turn a board vector into a board struct
Board* readInBoard(char* boardVector)
{

	char* newBoardVector = malloc(sizeof(char) * strlen(boardVector));
	newBoardVector = strncpy(newBoardVector, boardVector + 1, strlen(boardVector) - 1);

	char *token = strtok(newBoardVector, ",");
	int width = atoi(token);
	token = strtok(NULL, ",");
	int height = atoi(token);
	token = strtok(NULL, ",");
	int lastColumn = atoi(token) - 1; // columns and rows are indexed from 1 in the input, 0 in the AI
	if (lastColumn < 0)
	{
		lastColumn = 0;
	}
	token = strtok(NULL, ",");
	int totalGameTime = atoi(token);
	token = strtok(NULL, ",");
	int playerOneTime = atoi(token);
	token = strtok(NULL, ",");
	int lastMoveTime = atoi(token);

	Board *board = initializeBoard(width, height, lastColumn, totalGameTime, playerOneTime, lastMoveTime);

	int i, j;

	for (i = -3; i < board->width + 3; i++)
	{
		for (j = -3; j < board->height + 3; j++)
		{
			if(i >= 0 && i < board->width && j >= 0 && j < board->height){
				token = strtok(NULL, ",");
				board->boardMatrix[i][j] = token[0];
			}else{
				board->boardMatrix[i][j] = 's';
			}
		}
	}

	free(newBoardVector);
	return board;
}

int totalRed, totalBlue, maxRed, maxBlue;

// see if there's a win on a given board
int checkBoardForWin(Board* board)
{
	//Uncomment the next line for a debug print of the board
	//printBoard(board);
	int lastRow = findLastRow(board);
	totalRed = 0;
	totalBlue = 0;
	maxRed = 0;
	maxBlue = 0;

	char activePiece = board->boardMatrix[board->lastColumn][lastRow];

	/* Strategy for checking for wins:
	 * 		In each of the directions (\, /, -, |), get the six pieces around the piece that was just
	 * 		placed. Put these into a string in the order they are in on the grid, and then find the different
	 * 		win patterns inside that string.
	 */
	// diagonally up and left (\)
	char lineString[WIN_NUMBER * 2];
	int i;

	for (i = -(WIN_NUMBER - 1); i <= (WIN_NUMBER - 1); i++)
	{
		lineString[i + (WIN_NUMBER - 1)] = board->boardMatrix[board->lastColumn - i][lastRow + i];
	}

	lineString[i + (WIN_NUMBER - 1)] = '\0';

	if (activePiece == 'g')
	{
		checkLineForMatch(lineString, 'r');
		checkLineForMatch(lineString, 'b');
	}
	else
	{
		checkLineForMatch(lineString, activePiece);
	}

	// diagonally up and right (/)
	for (i = -(WIN_NUMBER - 1); i <= (WIN_NUMBER - 1); i++)
	{
		lineString[i + (WIN_NUMBER - 1)] = board->boardMatrix[board->lastColumn + i][lastRow + i];
	}

	lineString[i + (WIN_NUMBER - 1)] = '\0';

	if (activePiece == 'g')
	{
		checkLineForMatch(lineString, 'r');
		checkLineForMatch(lineString, 'b');
	}
	else
	{
		checkLineForMatch(lineString, activePiece);
	}

	// left to right (-)
	for (i = -(WIN_NUMBER - 1); i <= (WIN_NUMBER - 1); i++)
	{
		lineString[i + (WIN_NUMBER - 1)] = board->boardMatrix[board->lastColumn + i][lastRow];
	}

	lineString[i + (WIN_NUMBER - 1)] = '\0';

	if (activePiece == 'g')
	{
		checkLineForMatch(lineString, 'r');
		checkLineForMatch(lineString, 'b');
	}
	else
	{
		checkLineForMatch(lineString, activePiece);
	}

	// down (|)
	if (lastRow >= (WIN_NUMBER - 1)) // don't bother if we're below the fourth row
	{
		//we don't need to get the pieces above it, since there are no pieces above the piece last played
		for (i = 0; i < WIN_NUMBER; i++)
		{
			lineString[i] = board->boardMatrix[board->lastColumn][lastRow - i];
		}

		lineString[i] = '\0';

		if (activePiece == 'g')
		{
			checkLineForMatch(lineString, 'r');
			checkLineForMatch(lineString, 'b');
		}
		else
		{
			checkLineForMatch(lineString, activePiece);
		}
	}

	//Uncomment to see the totals, for debugging purposes
	//fprintf(stderr, "Red: %d Blue: %d\n", totalRed, totalBlue);
	if (totalRed == totalBlue && totalRed != 0)
	{
		return 1;
	}
	else if (totalRed > totalBlue)
	{
		return maxRed;
	}
	else if (totalRed < totalBlue)
	{
		return maxBlue * -1;
	}
	else
	{
		return 0;
	}
}

// given a board and coordinates, see if the space is actually in the matrix so we don't go out of bounds
int isSpaceInGrid(Board *board, int column, int row)
{
	return (row >= 0 && row < board->height && column >= 0 && column < board->width);
}

// see if there's a win pattern in the string passed in
void checkLineForMatch(char* string, char activePiece)
{
	int i;
	char* lineSubString;// = malloc(sizeof(char) * (WIN_NUMBER + 1));

	int numberOfStrings = strlen(string) - WIN_NUMBER;
	for (i = 0; i <= numberOfStrings; i++)
	{
		lineSubString = string + i;

		if (lineSubString[0] + lineSubString[1] + lineSubString[2] + lineSubString[3] == 'g' + 'g' + activePiece + activePiece)
		{
			if (lineSubString[0] == activePiece)//x
			{
				if (lineSubString[1] == activePiece) //xx
				{
					if (lineSubString[2] == 'g')//xxg
					{
						if (activePiece == 'r')
						{
							maxRed = AABB_WIN;
							totalRed += AABB_WIN;
						}
						else
						{
							maxBlue = AABB_WIN;
							totalBlue += AABB_WIN;
						}
					}
				}
				else if (lineSubString[1] == 'g')//xg
				{
					if (lineSubString[2] == activePiece)//xgx
					{
						if (activePiece == 'r')
						{
							maxRed = MAX(ABAB_WIN, maxRed);
							totalRed += ABAB_WIN;
						}
						else
						{
							maxBlue = MAX(ABAB_WIN, maxBlue);
							totalBlue += ABAB_WIN;
						}

					}
					else if (lineSubString[2] == 'g') //xgg
					{

						if (activePiece == 'r')
						{
							maxRed = MAX(ABBA_WIN, maxRed);
							totalRed += ABBA_WIN;
						}
						else
						{
							maxBlue = MAX(ABBA_WIN, maxBlue);
							totalBlue += ABBA_WIN;
						}

					}
				}
			}
			else if (lineSubString[0] == 'g') //g
			{
				if (lineSubString[1] == activePiece) //gx
				{
					if (lineSubString[2] == activePiece) // gxx
					{

						if (activePiece == 'r')
						{
							maxRed = MAX(ABBA_WIN, maxRed);
							totalRed += ABBA_WIN;
						}
						else
						{
							maxBlue = MAX(ABBA_WIN, maxBlue);
							totalBlue += ABBA_WIN;
						}

					}
					else if (lineSubString[2] == 'g') //gxg
					{
						if (activePiece == 'r')
						{
							maxRed = MAX(ABAB_WIN, maxRed);
							totalRed += ABAB_WIN;
						}
						else
						{
							maxBlue = MAX(ABAB_WIN, maxBlue);
							totalBlue += ABAB_WIN;
						}
					}
				}
				else if (lineSubString[1] == 'g') // gg
				{
					if (lineSubString[2] == activePiece) // ggx
					{
						if (activePiece == 'r')
						{
							maxRed = AABB_WIN;
							totalRed += AABB_WIN;
						}
						else
						{
							maxBlue = AABB_WIN;
							totalBlue += AABB_WIN;
						}
					}
				}
			}
		}
	}
}

// given a board, find the row of the last piece that was played (it's not passed in)
int findLastRow(Board *board)
{
	int i;
	for (i = 0; i < board->height; i++)
	{
		if (board->boardMatrix[board->lastColumn][i] == 's')
		{
			break;
		}
	}
	return i - 1;
}

// initialize a board struct, constructor-style
Board* initializeBoard(int width, int height, int lastColumn, int totalGameTime, int playerOneTime, int lastMoveTime)
{
	Board *board = malloc(sizeof(Board));

	board->width = width;
	board->height = height;
	board->lastColumn = lastColumn;
	board->totalGameTime = totalGameTime;
	board->playerOneTime = playerOneTime;
	board->lastMoveTime = lastMoveTime;

	int i;
	board->boardMatrix = malloc(sizeof(char*) * (board->width + 6));

	for (i = 0; i < board->width + 6; i++)
	{
		board->boardMatrix[i] = malloc(sizeof(char) * (board->height + 6));
		board->boardMatrix[i] += 3;
	}
	board->boardMatrix += 3;
	return board;
}

// delete a board
void deleteBoard(Board* board)
{
	int i;
	board->boardMatrix -= 3;
	for (i = 0; i < board->width + 6; i++)
	{
		board->boardMatrix[i] -= 3;
		free(board->boardMatrix[i]);
	}
	free(board->boardMatrix);
	free(board);
}

Move* getLegalMoves(Board* board, char playerTurn)
{
	Move* moveList = malloc((sizeof(Move) * board->width * 2) + 1);
	int numberOfMoves = 0;
	int i;
	int winValue = checkBoardForWin(board);
	if (winValue == 0)
	{
		int j = 1;
		for (i = (board->width - 1) / 2; j <= board->width; j++)
		{
			if (board->boardMatrix[i][board->height - 1] == 's')
			{
				moveList[numberOfMoves].colour = playerTurn;
				moveList[numberOfMoves++].column = i;
				moveList[numberOfMoves].colour = 'g';
				moveList[numberOfMoves++].column = i;
			}
			if (j % 2 == 0)
			{
				i -= j;
			}
			else
			{
				i += j;
			}
		}
		moveList[numberOfMoves++].colour = 'x';
	}
	else
	{
		moveList[numberOfMoves].colour = 'w';
		moveList[numberOfMoves].column = winValue * (playerTurn == 'b' ? -1 : 1);
	}
	return moveList;
}

int emptyColumns(Board *board)
{
	int total = 0;
	int i;
	for (i = 0; i < board->width; i++)
	{
		if (board->boardMatrix[i][board->height - 1] == 's')
		{
			total++;
		}
	}
	return total;
}
