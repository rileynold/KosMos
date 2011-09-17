/*
 * gamebrain.c
 *
 *  Created on: May 25, 2010
 *      Author: matthewguze
 */

#include "include/globals.h"
#include "include/gamebrain.h"

#define BLUE -1
#define RED 1
#define BIGNUM INT_MAX - 1
#define SMALLNUM INT_MIN + 1

void makeMove(Move move);
void undoMove();

int minMax(int depth, char color, int alpha, int beta, int total, int totalThreats);
int calculatePosition(char color, int total);
int calculateAdjacentStep(char color, int column, int row);
int calculateAdjacent(char color);
int checkLineForThreat(char* lineString, char color, int checkForBlock, int row, int horizontal);
int findGoodSquares(int column, int row);
void printSillyRPGStartText();
void printSillyRPGEndText(Move moveToMake, int alpha);

Move moveHistory[MAX_DEPTH];
int movesMade = 0;
int firstColumn = -1;
int currentDepth;
char currentTurnChar = 'b';

char realColor = 'b';

time_t startTime;
Board *board;

// these numbers may look magic, but they're not. They figure out approximately the same thing
// that the previous calculate function figured out, it just does it move by move and
// ignores pieces that haven't changed, so it's faster

// green, other, you
PointValue green =
{ 4, -9, 5 };
PointValue you =
{ 5, 6, 4 };
PointValue other =
{ -9, -12, 6 };

Move getNextMove(Board* inputBoard)
{
	srand(time(NULL));

	startTime = time(NULL);

	board = inputBoard;

	int spaceCount = 0;
	int x, y;
	int totalThreats = 0;
	for (x = 0; x < board->width; x++)
	{
		for (y = 0; y < board->height; y++)
		{
			if (board->boardMatrix[x][y] == 's')
			{
				totalThreats += findGoodSquares(x, y);
				spaceCount++;
			}
		}
	}
	//fprintf(stderr, "%d\n", totalThreats);

	if (spaceCount % 2 != 0)
	{
		realColor = 'r';
	}

	printSillyRPGStartText();

	int additionalDepth = board->width - emptyColumns(board);
	currentDepth = STARTING_DEPTH + MAX(0, additionalDepth);

	// if we're taking too much time, turn the depth down so we don't go over
	if (board->totalGameTime > 900000)
	{
		currentDepth--;
	}
	if (board->totalGameTime > 1000000)
	{
		currentDepth--;
	}

	firstColumn = board->lastColumn;

	Move moveToMake =
	{ -1, 'r' };
	int i;

	int alpha = SMALLNUM;
	int beta = BIGNUM;

	Move* legalMoves = getLegalMoves(board, currentTurnChar);

	int currentValue;

	for (i = 0; legalMoves[i].colour != 'x'; i++)
	{
		//		if (difftime(time(NULL), startTime) > 170.0)
		//		{
		//			fprintf(stderr, "KOS-MOS woke up!\n");
		//			break;
		//		}
		makeMove(legalMoves[i]);
		currentValue = -minMax(currentDepth, 'b', -beta, -alpha, 0, totalThreats);
		if (currentValue == 1000)
		{
			currentValue = -1000;
		}
		//		int difference = 0;
		//		if (currentValue > 3000 && currentDepth > 0)
		//		{
		//			difference = currentDepth - (currentValue % 1000);
		//			currentDepth -= difference;
		//			currentValue -= difference;
		//			alpha -= difference;
		//			currentDepth = MAX(currentDepth, 0);
		//		}

		// KEEP for debugging purposes
		fprintf(stderr, "(%d, %c), %d\n", legalMoves[i].column + 1, legalMoves[i].colour, currentValue);

		undoMove();

		if (currentValue >= beta)
		{
			moveToMake = legalMoves[i];
			break;
		}

		if (currentValue > alpha)
		{
			moveToMake = legalMoves[i];
			alpha = currentValue;
		}
	}
	free(legalMoves);
	printSillyRPGEndText(moveToMake, alpha);
	return moveToMake;
}

int minMax(int depth, char color, int alpha, int beta, int total, int totalThreats)
{
	color = (color == 'b') ? 'r' : 'b';

	int lastRow = findLastRow(board);

	int lastMoveValue = calculateAdjacentStep('b', board->lastColumn, lastRow);

	int threatLevel = findGoodSquares(board->lastColumn, lastRow);

	totalThreats += threatLevel;
	//fprintf(stderr, "Value: %d\n", totalThreats);

	if (color == 'r')
	{
		total -= (lastMoveValue);
		//		if (threatLevel > 0 && totalThreats < 2)
		//		{
		total -= threatLevel * 25 - depth;
		//		}
		//		else if (threatLevel < 0 && totalThreats > -2)
		//		{
		//			total -= threatLevel * 50;
		//		}
		//if (totalThreats > -1)
		//{
		//total -= threatLevel * 50;
		//}

	}
	else
	{
		total += (lastMoveValue);
		//if (threatLevel > 0 && totalThreats < 1)
		//{

		total += threatLevel * 25 + depth;
		//}
		//else if (threatLevel < 0 && totalThreats > -1)
		//{
		//		total += threatLevel * 50;
		//	}
		//if (totalThreats < 1)
		//{
		//total += threatLevel * 50;
		///}
	}

	if (depth <= 0)
	{

		return calculatePosition(color, total);
	}

	Move* legalMoves = getLegalMoves(board, color);
	if (legalMoves[0].colour == 'x')
	{
		return 0;
	}

	int currentValue;
	int i;
	for (i = 0; legalMoves[i].colour != 'x'; i++)
	{
		//		if (difftime(time(NULL), startTime) > 170.0)
		//		{
		//			return -1000;
		//		}

		if (legalMoves[i].colour == 'w')
		{
			if (legalMoves[i].column >= 0)
			{
				alpha = 1000 * legalMoves[i].column + depth;
			}
			else
			{
				alpha = 1000 * legalMoves[i].column - depth;
			}
			break;
		}
		makeMove(legalMoves[i]);

		currentValue = -minMax(depth - 1, color, -beta, -alpha, -total, totalThreats);
		undoMove();
		if (currentValue >= beta)
		{
			return beta;
		}
		alpha = MAX(currentValue, alpha);
	}
	free(legalMoves);
	return alpha;

}

int calculatePosition(char color, int total)
{
	// returns a value based on the current position
	int winValue = checkBoardForWin(board);
	if (winValue != 0)
	{
		if (winValue == 1)
		{
			return -1000;
		}

		if (color == 'b')
		{
			winValue *= -1;
		}

		return winValue * 1000;
	}
	//int value = calculateAdjacent(color) - calculateAdjacent((color == 'b') ? 'r':'b') * 3;
	return total;

}
void makeMove(Move move)
{
	moveHistory[movesMade++] = move;
	board->lastColumn = move.column;
	board->boardMatrix[board->lastColumn][findLastRow(board) + 1] = move.colour;
}

void undoMove()
{
	board->boardMatrix[board->lastColumn][findLastRow(board)] = 's';
	if (movesMade > 1)
	{
		board->lastColumn = moveHistory[movesMade - 2].column;
	}
	else
	{
		board->lastColumn = firstColumn;
	}
	--movesMade;
}

int calculateAdjacent(char color)
{
	int total = 0;
	int i, j, k, l;
	for (i = 0; i < board->width; i++)
	{
		for (j = 0; j < board->height; j++)
		{
			if (board->boardMatrix[i][j] == color || board->boardMatrix[i][j] == 'g')
			{
				int multiplier = 1;
				if (board->boardMatrix[i][j] == 'g')
				{
					multiplier = 2;
				}
				for (k = -1; k <= 1; k++)
				{
					for (l = -1; l <= 1; l++)
					{
						if (isSpaceInGrid(board, i + k, j + l))
						{
							if (board->boardMatrix[i + k][j + l] == color)
							{
								total += 2 * multiplier;
							}
							else if (board->boardMatrix[i + k][j + l] == 'g')
							{
								total += 1 * multiplier;
							}
							else if (board->boardMatrix[i + k][j + l] != 's')
							{
								total -= 3 * multiplier;
							}
						}
					}
				}
			}
		}

	}
	return total;
}

int calculateAdjacentStep(char color, int column, int row)
{
	char lastPiece = board->boardMatrix[board->lastColumn][row];
	int i, j;

	int total = 0;

	PointValue* pointValue;

	if (lastPiece == color)
	{
		pointValue = &you;
	}
	else if (lastPiece == 'g')
	{
		pointValue = &green;
	}
	else // opposing color, can't be a space because there must be a piece there
	{
		pointValue = &other;
	}

	for (i = -1; i <= 1; i++)
	{
		for (j = -1; j <= 1; j++)
		{
			if (board->boardMatrix[board->lastColumn + i][row + j] == color)
			{
				total += pointValue->youPoint;
			}
			else if (board->boardMatrix[board->lastColumn + i][row + j] == 'g')
			{
				total += pointValue->greenPoint;
			}
			else if (board->boardMatrix[board->lastColumn + i][row + j] != 's')
			{
				total += pointValue->otherPoint;
			}
		}
	}
	return total;
}

#define ODD 1
#define EVEN 0
int findGoodSquares(int column, int row)
{
	char ulWin = 0;
	char dlWin = 0;
	char urWin = 0;
	char drWin = 0;
	char lWin = 0;
	char rWin = 0;
	int threatCount = 0;

	char activePiece = board->boardMatrix[column][row];

	/* Strategy for checking for wins:
	 * 		In each of the directions (\, /, -, |), get the six pieces around the piece that was just
	 * 		placed. Put these into a string in the order they are in on the grid, and then find the different
	 * 		win patterns inside that string.
	 */
	// diagonally up and left (\)
	char lineString[WIN_NUMBER * 2];
	int i;

	char currentChar;
	for (i = -(WIN_NUMBER - 1); i <= (WIN_NUMBER - 1); i++)
	{
		currentChar = board->boardMatrix[board->lastColumn + i][row - i];
		if (currentChar != 's' || ((board->lastColumn + i >= 0 && board->lastColumn + i < board->width) && row - i - 1 >= 0 && row - i < board->height && currentChar == 's'
				&& board->boardMatrix[board->lastColumn + i][row - i - 1] == 's'))
		{
			lineString[i + (WIN_NUMBER - 1)] = currentChar;
			if (currentChar == 's')
			{
				if (i < 0)
				{
					// the space is on the left
					ulWin = 1;
				}
				else if (i > 0)
				{
					// the space is on the right
					drWin = 1;
				}
			}
		}
		else
		{
			lineString[i + (WIN_NUMBER - 1)] = '\0';
		}

	}

	lineString[i + (WIN_NUMBER - 1)] = '\0';

	int redThreat;
	int blueThreat;
	if (activePiece == 'g' || activePiece == 's')
	{
		redThreat = checkLineForThreat(lineString, 'r', 0, row, 0);
		blueThreat = checkLineForThreat(lineString, 'b', 0, row, 0);
		threatCount += redThreat + blueThreat;

		if (activePiece == 'g')
		{
			if (redThreat < -1)
			{
				ulWin *= 'r';
				drWin *= 'r';
			}
			else if (blueThreat > 1)
			{
				ulWin *= 'b';
				drWin *= 'b';
			}
		}
	}
	else
	{
		threatCount += checkLineForThreat(lineString, activePiece, 1, row, 0);
	}

	// diagonally up and right (/)
	for (i = -(WIN_NUMBER - 1); i <= (WIN_NUMBER - 1); i++)
	{
		currentChar = board->boardMatrix[board->lastColumn + i][row + i];
		if (currentChar != 's' || ((board->lastColumn + i >= 0 && board->lastColumn + i < board->width) && row + i - 1 >= 0 && row + i < board->height && currentChar == 's'
				&& board->boardMatrix[board->lastColumn + i][row + i - 1] == 's'))
		{
			lineString[i + (WIN_NUMBER - 1)] = currentChar;
			if (currentChar == 's')
			{
				if (i < 0)
				{
					// the space is on the left
					dlWin = 1;
				}
				else if (i > 0)
				{
					// the space is on the right
					urWin = 1;
				}
			}
		}
		else
		{
			lineString[i + (WIN_NUMBER - 1)] = '\0';
		}
	}

	lineString[i + (WIN_NUMBER - 1)] = '\0';

	if (activePiece == 'g' || activePiece == 's')
	{
		redThreat = checkLineForThreat(lineString, 'r', 0, row, 0);
		blueThreat = checkLineForThreat(lineString, 'b', 0, row, 0);
		threatCount += redThreat + blueThreat;
		if (activePiece == 'g')
		{
			if (redThreat < -1)
			{
				dlWin *= 'r';
				urWin *= 'r';
			}
			else if (blueThreat > 1)
			{
				dlWin *= 'b';
				urWin *= 'b';
			}
		}
	}
	else
	{
		threatCount += checkLineForThreat(lineString, activePiece, 1, row, 0);
	}

	// left to right (-)
	for (i = -(WIN_NUMBER - 1); i <= (WIN_NUMBER - 1); i++)
	{
		currentChar = board->boardMatrix[board->lastColumn + i][row];
		if (currentChar != 's' || ((board->lastColumn + i >= 0 && board->lastColumn + i < board->width) && row - 1 >= 0 && currentChar == 's' && board->boardMatrix[board->lastColumn + i][row - 1]
				== 's'))
		{
			lineString[i + (WIN_NUMBER - 1)] = currentChar;
			if (currentChar == 's')
			{
				if (i < 0)
				{
					// the space is on the left
					lWin = 1;
				}
				else if (i > 0)
				{
					// the space is on the right
					rWin = 1;
				}
			}
		}
		else
		{
			lineString[i + (WIN_NUMBER - 1)] = '\0';
		}
	}

	lineString[i + (WIN_NUMBER - 1)] = '\0';

	if (activePiece == 'g' || activePiece == 's')
	{
		redThreat = checkLineForThreat(lineString, 'r', 0, row, 1);
		blueThreat = checkLineForThreat(lineString, 'b', 0, row, 1);
		threatCount = redThreat + blueThreat;
		if (redThreat != 0 && blueThreat != 0)
		{
			if (blueThreat > 1)
			{
				if (dlWin == 'r' || drWin == 'r')
				{
					threatCount -= 2;
				}
				else if (ulWin == 'r' || urWin == 'r')
				{
					threatCount += 2;
				}
			}
			if (redThreat < -1)
			{
				if (dlWin == 'b' || drWin == 'b')
				{
					threatCount += 2;
				}
				else if (ulWin == 'b' || urWin == 'b')
				{
					threatCount -= 2;
				}
			}
		}
	}
	else
	{
		threatCount += checkLineForThreat(lineString, activePiece, 1, row, 1);
	}

	return threatCount;
}

int checkLineForThreat(char* string, char activePiece, int checkForBlock, int row, int horizontal)
{
	int oddOrEven;
	int i;
	char enemyPiece = (activePiece == 'b') ? 'r' : 'b';
	char* lineSubString;

	int numberOfStrings = strlen(string) - WIN_NUMBER;
	for (i = 0; i <= numberOfStrings; i++)
	{
		lineSubString = string + i;

		if ((lineSubString[0] + lineSubString[1] + lineSubString[2] + lineSubString[3] == 'g' + 's' + activePiece + activePiece) || (lineSubString[0] + lineSubString[1] + lineSubString[2]
				+ lineSubString[3] == 'g' + 's' + 'g' + activePiece))
		{
			int j;
			if (!horizontal)
			{
				for (j = 0; j < WIN_NUMBER; j++)
				{
					if (lineSubString[j] == 's')
					{
						break;
					}
				}
				oddOrEven = (i + j - (WIN_NUMBER - 1) + row) % 2;
			}
			else
			{
				oddOrEven = row % 2;
			}
			if (activePiece == 'b')
			{
				if ((oddOrEven == ODD && realColor == 'b') || (oddOrEven == EVEN && realColor =='r'))
				{
					return 4;
				}
				return 2;
			}
			else
			{
				if ((oddOrEven == EVEN && realColor == 'b') || (oddOrEven == ODD && realColor == 'r'))
				{
					return -4;
				}
				return -2;
			}
		}
		if (checkForBlock)
		{
			if ((lineSubString[0] + lineSubString[1] + lineSubString[2] + lineSubString[3] == 'g' + 'g' + activePiece + enemyPiece) || (lineSubString[0] + lineSubString[1] + lineSubString[2]
					+ lineSubString[3] == 'g' + activePiece + enemyPiece + enemyPiece))
			{
				if (activePiece == 'b')
				{
					return 2;
				}
				else
				{
					return -2;
				}
			}
		}

	}
	return 0;
}

// feel free to add your own
#define NUMBER_OF_RPG_VERBS 25
char* rpgVerbs[] =
{ "used", "cast", "attacked with", "threw", "summoned", "fired", "attempted to distract the opponent using", "tried to make the opponent trip over", "made a wish and received", "sang", "tried",
		"typed", "activated", "combined 14 different ingredients to make", "synthesized", "picked up", "transformed into", "got in", "united with", "detonated", "gave an attack order to", "selected",
		"bribed the game designer with", "pulled out", "did a dance around" };

// from the "way too much time on my hands" department. Takes up almost no time compared to the actual search.
void printSillyRPGStartText()
{
	if (!(board->lastColumn == 0 && board->boardMatrix[0][0] == 's'))
	{
		// check to see if it's the second move. If so, print ambush text (since the opponent "attacked" first)
		int found = 0;
		int x;
		for (x = 0; x < board->width; x++)
		{
			if (board->boardMatrix[x][0] == 'r' || board->boardMatrix[x][0] == 'g')
			{
				if (!found && board->boardMatrix[x][1] == 's')
				{
					found = 1;
				}
				else
				{
					break;
				}
			}
			else if (board->boardMatrix[x][0] != 's')
			{
				break;
			}
		}
		if (x == board->width && found)
		{
			fprintf(stderr, "Ambushed by enemy AI!\n");
		}
		// get the opponent's last move, roll 1d4 to see if it "hits" :P
		fprintf(stderr, "Enemy AI %s (%d, %c)!\n", rpgVerbs[rand() % NUMBER_OF_RPG_VERBS], board->lastColumn + 1, board->boardMatrix[board->lastColumn][findLastRow(board)]);
		if (rand() % 4 == 0)
		{
			fprintf(stderr, "KOS-MOS dodged!\n");
		}
		else
		{
			fprintf(stderr, "KOS-MOS took %d damage!\n", (rand() % 100) + 50);
		}
	}
	else
	{
		// if it's the first move, print this
		fprintf(stderr, "Enemy AI appears!\n");
	}
}

void printSillyRPGEndText(Move moveToMake, int alpha)
{
	if (alpha > 1000)
	{
		// print this if we have a guaranteed win (probably the only useful text, along with the forced loss text :P)
		fprintf(stderr, "KOS-MOS %s (%d, %c)!\nCritical hit!\n", rpgVerbs[rand() % NUMBER_OF_RPG_VERBS], moveToMake.column + 1, moveToMake.colour);
		fprintf(stderr, "Enemy AI took %d damage!\n", alpha + (rand() % 2000) - 1000);
		if (alpha % 1000 == currentDepth)
		{
			// print this only if the move we made wins the game
			fprintf(stderr, "Defeated enemy AI!\n");
			fprintf(stderr, "KOS-MOS gained %d EXP!\n", (rand() % 1000) + 1000);
			fprintf(stderr, "Received %dG!\n", (rand() % 3000) + 3000);
		}
	}
	else if (alpha < -2000)
	{
		// print this only when we're about to lose
		fprintf(stderr, "KOS-MOS %s (%d, %c)!\nEnemy AI dodged gracefully!\n", rpgVerbs[rand() % NUMBER_OF_RPG_VERBS], moveToMake.column + 1, moveToMake.colour);
	}
	else if ((alpha + 50) >= 0)
	{
		// "damage" is somewhat based on the heuristic calculation, with some randomness to spice things up :P
		fprintf(stderr, "KOS-MOS %s (%d, %c)!\nEnemy AI took %d damage!\n", rpgVerbs[rand() % NUMBER_OF_RPG_VERBS], moveToMake.column + 1, moveToMake.colour, alpha + 100 + (rand() % 100));
	}
	else
	{
		// even though loosely it's based on the heuristic, having a negative score/missing your "attack" doesn't mean you're losing, necessarily
		fprintf(stderr, "KOS-MOS %s (%d, %c)!\nKOS-MOS missed!\n", rpgVerbs[rand() % NUMBER_OF_RPG_VERBS], moveToMake.column + 1, moveToMake.colour);
	}
}

