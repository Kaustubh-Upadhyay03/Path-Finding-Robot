#define GYRO_SENSOR_PORT S3
#define COLOR_SENSOR_PORT S2

const int CALIBRATION_TIME = 50;
const int DISPENSE_SPEED = 5;
const int NUM_ROWS = 3;
const int NUM_COLUMNS = 3;
const int PLAYER_COLOR = (int)colorRed;
const int PLAYER_VALUE = 1;
const int ROBOT_VALUE = 2;
const float TILE_SIZE = 14.5;
const int MED_SPEED = 50;
const int SLOW_SPEED = 10;
const int WHEEL_RADIUS = 2.75;
const int WHEEL_TO_CM = 180/(PI * WHEEL_RADIUS);
const int TURN = 90;
const int TURN_AROUND = 180;
const int NUM_SQUARES = 9;
const int S_TO_MS = 1000;
const int ROTATION_COUNT = 100;
const int WAIT_TIME = 500;

//In the array -> Empty is 0; Player = 1; Robot is 2;

int board[NUM_ROWS][NUM_COLUMNS] = {0,0,0,0,0,0,0,0,0};

void startGame()
{
	//calibrate sensors + reset timer
	SensorType(GYRO_SENSOR_PORT) = sensorEV3_Gyro;
	resetGyro(GYRO_SENSOR_PORT);
	SensorType(COLOR_SENSOR_PORT) = sensorEV3_Color;
	SensorMode(COLOR_SENSOR_PORT) = modeEV3Color_Color;

	wait1Msec(CALIBRATION_TIME);
	clearTimer(T1);
}

void go(int motorSpeed)
{
	motor[motorA] = motor[motorD] = motorSpeed;
}

void driveDistance(float distance, int power)
{
	nMotorEncoder[motorA] = 0;
	go(power);
	while (nMotorEncoder[motorA] < distance * WHEEL_TO_CM)
	{}
	go(0);
}

void driveTile()
{
	driveDistance(TILE_SIZE, MED_SPEED);
	wait1Msec(WAIT_TIME);
}

void turnClockwise(int angle)
{
	motor[motorA] = -SLOW_SPEED;
	motor[motorD] = SLOW_SPEED;

	while(abs(getGyroDegrees(GYRO_SENSOR_PORT)) < angle)
	{}

	go(0);
	resetGyro(GYRO_SENSOR_PORT);
	wait1Msec(CALIBRATION_TIME);
}

void turnCounterClockwise(int angle)
{
	motor[motorA] = SLOW_SPEED;
	motor[motorD] = -SLOW_SPEED;

	while(abs(getGyroDegrees(GYRO_SENSOR_PORT)) < angle)
	{}

	go(0);
	resetGyro(GYRO_SENSOR_PORT);
	wait1Msec(CALIBRATION_TIME);
}

//while T < 1500, check for color + display
void scanTile(int row, int column)
{
	/*if the element at the row/column 
	is empty scan it to see if the player marked it */
	if (!board[row][column])
	{   
	    clearTimer(T2);
		while(time1[T2] < S_TO_MS)
		{
            if (SensorValue(COLOR_SENSOR_PORT) == PLAYER_COLOR)
            {
                board[row][column] = PLAYER_VALUE;
            }
	    }
        //display tile scanned + check if stored in array
        displayString(0,"%i %i",row,column);
        displayString(1,"%i", board[row][column]);
    }
}

void scanBoard(int &currentRow, int &currentColumn)
{
	//drives from start to (0,0)
	driveTile();
	//scans (0,0)
	scanTile(currentRow,currentColumn);
	turnClockwise(TURN);

	/*for the first row, the first tile is already scanned 
	-> need to scan 2 more tiles (0,1) and (0,2)*/
	for (int currentColumn = 0; currentColumn < NUM_COLUMNS - 1;currentColumn++)
	{
		driveTile();
		scanTile(currentRow,currentColumn);
	}

	turnCounterClockwise(TURN);
	driveTile();
	currentRow++;
	//at this point the robot is at (1,2)
	scanTile(currentRow,currentColumn);
	turnCounterClockwise(TURN);

	for (int currentColumn = 2; currentColumn > 0; currentColumn--)
	{
		driveTile();
		scanTile(currentRow,currentColumn);
	}

	//at this point the robot is at (1,0)
	turnClockwise(TURN);
	driveTile();
	currentRow++;
	//at this point it is at (2,0);
	scanTile(currentRow,currentColumn);
	turnClockwise(TURN);

	//same as with row 0
	for (int currentColumn = 0; currentColumn < NUM_COLUMNS - 1; currentColumn++)
	{
		driveTile();
		scanTile(currentRow,currentColumn);
	}

	turnCounterClockwise(TURN);
	//at this point the robot is at (2,2)
	driveTile();
	//at this point the robot is at the end
}

void decideSquare(int &chosenRow,int &chosenColumn)
{
	do
	{
		chosenRow = abs(rand() % (NUM_ROWS));
		chosenColumn = abs(rand() % (NUM_COLUMNS));
	}
	while(board[chosenRow][chosenColumn] != 0);

	board[chosenRow][chosenColumn] = ROBOT_VALUE;
	//display square chosen + check if stored in array
	displayString(3, "Chosen Square is (%i,%i)", chosenRow,chosenColumn);
	displayString(4, "%i", board[chosenRow][chosenColumn]);
}

void goToChosenSquare(int chosenRow, int chosenColumn)
{
	//start from end square facing away from the board
	turnClockwise(TURN_AROUND);
	driveTile();
	//int currentRow = 2;
	//int currentColumn = 2;
	//at (2,2) and go down the rows
	for (int currentRow = 2; currentRow > chosenRow; currentRow--)
	{
		driveTile();
	}
	//at the right row but not the right column
	turnClockwise(TURN);

	for (int currentColumn = 2; currentColumn > chosenColumn; currentColumn--)
	{
		driveTile();
	}
}

void placePiece()
{
	motor[motorC] = DISPENSE_SPEED;

	nMotorEncoder[motorC] = 0;

	float rotation = ROTATION_COUNT;

	while(nMotorEncoder[motorC] < rotation)
	{}

	motor[motorC] = -DISPENSE_SPEED;

	while(nMotorEncoder[motorC] > 0)
	{}

	motor[motorC] = 0;
}

void goToStart(int currentRow, int currentColumn)
{
	//go to column 0 and facing left initially
	for (int currentColumn; currentColumn > 0; currentColumn--)
	{
		driveTile();
	}
	turnCounterClockwise(TURN);
	//go down to row 0
	for (int currentRow; currentRow > 0; currentRow--)
	{
		driveTile();
	}
	//at (0,0) so go down to start and turn around
	driveTile();
	turnClockwise(TURN_AROUND);
}


//0 means no win yet, 1 is player win, 2 is robot win
int checkRows()
{
	int result = 0;
	for (int row = 0; row < NUM_ROWS; row++)
	{
		if (board[row][0] == PLAYER_VALUE && board[row][0] == board[row][1] && board[row][0] == board[row][2])
		{
			result = PLAYER_VALUE;
		}
		else if (board[row][0] == ROBOT_VALUE && board[row][0] == board[row][1] && board[row][0] == board[row][2])
		{
			result = ROBOT_VALUE;
		}
	}
	return result;
}

int checkColumns()
{
	int result = 0;
	for (int column = 0; column < NUM_COLUMNS; column++)
	{
		if (board[0][column] == PLAYER_VALUE && board[0][column] == board[1][column] && board[0][column] == board[2][column])
		{
			result = PLAYER_VALUE;
		}
		else if (board[0][column] == ROBOT_VALUE && board[0][column] == board[1][column] && board[0][column] == board[2][column])
		{
			result = ROBOT_VALUE;
		}
	}
	return result;
}

int checkDiagonals()
{
	int result = 0;
	if ((board[0][0] == PLAYER_VALUE && board[0][0] == board[1][1] && board[0][0] == board[2][2]) ||(board[0][2] == PLAYER_VALUE && board[0][2] == board[1][1] && board[0][2] == board[2][0]))
	{
		result = PLAYER_VALUE;
	}
	else if ((board[0][0] == ROBOT_VALUE && board[0][0] == board[1][1] && board[0][0] == board[2][2]) || (board[0][2] == ROBOT_VALUE && board[0][2] == board[1][1] && board[0][2] == board[2][0]))
	{
		result = ROBOT_VALUE;
	}
	return result;
}

int checkWin()
{
	int result = 0;
	result = checkRows();
	if (!result)
    {
		result = checkColumns();
    }
	if (!result)
    {
		result = checkDiagonals();
    }
	return result;
}

bool checkTie(int playerTurnCount, int robotTurnCount)
{
	bool isTie = false;
	//if all the squares are filled
	int totalTurns = playerTurnCount + robotTurnCount;
	if (totalTurns == NUM_SQUARES)
	{
		//if no one won
		if(!checkWin())
		isTie = true;
	}
	return isTie;
}

int checkResult(int playerTurnCount, int robotTurnCount)
{
	int result = 0;
	result = checkWin();
	//if no one won check for tie
	if (!checkWin())
    {
		//if not a tie that means game continues
	    if(!checkTie(playerTurnCount, robotTurnCount))
        {
            //-1 means no result, 0 means tie, 1 means player win, 2 means robot win
		    result = -1;
        }
    }
	return result;
}

void shutdown(int playerTurnCount, int robotTurnCount)
{
	int result = checkResult(playerTurnCount, robotTurnCount);
	int totalTurns = playerTurnCount + robotTurnCount;
	float totalTime = time1[T1]/S_TO_MS;
	float averageTime = totalTime/totalTurns;
	string message = "";

	if (result == 0)
	{
		message = "Tie";
	}
	else if (result == PLAYER_VALUE)
	{
		message = "Player Wins";
	}
	else
	{
		message = "Robot Wins";
	}

	displayString(0,"%s", message);
	displayString(1, "Total time was %f",totalTime);
	displayString(2, "Average time per turn was %f", averageTime);

	while(!getButtonPress(buttonAny))
	{}
	while(getButtonPress(buttonAny))
	{}
}

task main()
{
	int result = -1;
	//bool isTie = false;
	int playerTurnCount = 0;
	int robotTurnCount = 0;
	int currentRow = 0;
	int currentColumn = 0;
	int chosenRow = 0;
	int chosenColumn = 0;

	startGame();
	//if the game is not over
	while(result == -1)
	{
		currentRow = 0;
		currentColumn = 0;
		//player signals turn end using button press
		while(!getButtonPress(buttonAny))
		{}

		while(getButtonPress(buttonAny))
		{}

		playerTurnCount++;
		scanBoard(currentRow, currentColumn);
		// check for win after the player's turn is over
		result = checkResult(playerTurnCount, robotTurnCount);
		decideSquare(chosenRow, chosenColumn);
		goToChosenSquare(chosenRow,chosenColumn);
		placePiece();
		goToStart(currentRow, currentColumn);
		robotTurnCount++;
		//check for win after robot's turn is over
		result = checkResult(playerTurnCount, robotTurnCount);
	}
	shutdown(playerTurnCount,robotTurnCount);
}

