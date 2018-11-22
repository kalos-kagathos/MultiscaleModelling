#include <SFML\Window.hpp>
#include "MySFMLClasses.h"
#include <math.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <fstream>
#include <vector>
using namespace std;

#define arimo	"Czcionki/Arimo-Regular.ttf"
#define remach	"Czcionki/RemachineScript.ttf"

const int boardCoordinate_x = 310;
const int boardCoordinate_y = 35;
const int screenWidth = 1920;
const int screenHeight = 1080;

int height = 207;
int width = 321;
int numberOfGrains = 2;
int inclusionsNumber = 10;
int inclusionsSize = 5;
string saveFilename = "map";
string importFilename = "map";
int cellSize = 10;
vector<int> choosen;
char probablityOfRule4 = 50;
int boundarySize = 1;

const double A = 86710969050178.5;
const double B = 9.41268203527779;
const double C = A / B;
double t = 0.001;
const double e = 2.718281828459;
double ro0 = C + (1 - C)*pow(e, (-B*t));
double ro1;
const double roCritical = 4.21584 * pow(10, 12) / (height*width);

unsigned int numberOfRegeneratedGrains = 100000000;

const int menuCoordinateX = 10;
const int menuCoordinateY = 10;

class Grain
	:public RectangleShape{
public:
	bool alive;
	double ro;
	unsigned int grainID;
	Grain(){}
};

bool isOnGrainBorder(Grain(**board), int x, int y)
{
	unsigned int ID = board[x][y].grainID;
	if (board[x+1][y].grainID != ID)
		return true;
	if (board[x-1][y].grainID != ID)
		return true;
	if (board[x+1][y+1].grainID != ID)
		return true;
	if (board[x+1][y-1].grainID != ID)
		return true;
	if (board[x-1][y+1].grainID != ID)
		return true;
	if (board[x-1][y-1].grainID != ID)
		return true;
	if (board[x][y+1].grainID != ID)
		return true;
	if (board[x][y-1].grainID != ID)
		return true;
	return false;
}

unsigned int newID()
{
	return ++numberOfGrains;
}

unsigned int newIDRegenerated()
{
	return numberOfRegeneratedGrains++;
}

bool checkIfNewGrainCanBePlaced(Grain(**board), int x, int y)
{
	int r = 3;
	for (int i = x - 7; i < x + 7; i++)
		for (int j = (y - 7); j < y+7; j++)
			if (((i - x)*(i - x) + (j - y)*(j - y)) < (r*r))
				if (board[i][j].alive == 1)
					return false;
	return true;
}

Color newColor()
{
	return Color(rand()%252+2, rand() % 252+2, rand() % 251+2);
}

bool grainGrowth(Grain(**board), Grain(**newBoard)) {
	bool emptyGrains = 0;
	for (int i = 1; i < width - 1; i++)
		for (int j = 1; j < height - 1; j++)
			if (board[i][j].grainID == 0) {
				emptyGrains = 1;
				int IDs[4] = {0,0,0,0};
				bool present = 0;
				int index;
				if (board[i + 1][j].grainID > 1) {
					present = 1;
					index = 0;
					IDs[0]=board[i + 1][j].grainID;
				}
				if (board[i - 1][j].grainID > 1) {
					present = 1;
					index = 1;
					IDs[1] = board[i - 1][j].grainID;
				}
				if (board[i][j + 1].grainID > 1) {
					present = 1;
					index = 2;
					IDs[2] = board[i][j+1].grainID;
				}
				if (board[i][j - 1].grainID > 1) {
					present = 1;
					index = 3;
					IDs[3] = board[i][j-1].grainID;
				}

				if (present) {
					for (int z = 0; z < 4; z++)
						for (int y = 0; y < 4 && y != z; y++)
							if (IDs[y] == IDs[z] && IDs[y] > 1)
								index = y;
					switch (index) {
						case 0: {
							newBoard[i][j].grainID = board[i + 1][j].grainID;
							newBoard[i][j].alive = 1;
							newBoard[i][j].setFillColor(board[i + 1][j].getFillColor());
							break;
						}
						case 1: {
							newBoard[i][j].grainID = board[i - 1][j].grainID;
							newBoard[i][j].alive = 1;
							newBoard[i][j].setFillColor(board[i - 1][j].getFillColor());
							break;
						}
						case 2: {
							newBoard[i][j].grainID = board[i][j+1].grainID;
							newBoard[i][j].alive = 1;
							newBoard[i][j].setFillColor(board[i][j+1].getFillColor());
							break;
						}
						case 3: {
							newBoard[i][j].grainID = board[i][j - 1].grainID;
							newBoard[i][j].alive = 1;
							newBoard[i][j].setFillColor(board[i][j - 1].getFillColor());
							break;
						}
					}
				}
			}
			else {
				newBoard[i][j].grainID = board[i][j].grainID;
				newBoard[i][j].alive = 1;
				newBoard[i][j].setFillColor(board[i][j].getFillColor());
			}

	return emptyGrains;
}
bool controlledGrainGrowth(Grain(**board), Grain(**newBoard)) {
	bool emptyGrains = 0;
	for (int i = 1; i < width - 1; i++)
		for (int j = 1; j < height - 1; j++)
			if (board[i][j].grainID == 0) {
				emptyGrains = 1;
				vector<pair<Grain*, int>> neighbours;
				for (int a = -1; a < 2; a++)
					for (int b = -1; b < 2; b++)
						if (board[i + a][j + b].grainID > 1)
							neighbours.push_back(make_pair(&board[i + a][j + b],1));
				if (neighbours.size()) {
					for (int y = 0; y < neighbours.size(); y++)
						for (int z = y + 1; z < neighbours.size(); z++)
							if (neighbours[y].first->grainID == neighbours[z].first->grainID)
								neighbours[y].second++;
					int max = 1;
					int index = 0;
					for (int y = 0; y < neighbours.size(); y++)
						if (neighbours[y].second > max) {
							max = neighbours[y].second;
							index = y;
						}

					if (max < 3) {
						//rule4
						int prob = rand() % 100;
						if (prob <= probablityOfRule4) {
							newBoard[i][j].grainID = neighbours[index].first->grainID;
							newBoard[i][j].alive = 1;
							newBoard[i][j].setFillColor(neighbours[index].first->getFillColor());
							continue;
						}
						else {
							newBoard[i][j].grainID = board[i][j].grainID;
							newBoard[i][j].alive = 1;
							newBoard[i][j].setFillColor(board[i][j].getFillColor());
							continue;
						}
					}
					else if (max < 5) {
						//rule2 lub rule 3
						// + rule
						if (board[i + 1][j].grainID > 1 && board[i + 1][j].grainID == board[i - 1][j].grainID) {
							if (board[i + 1][j].grainID == board[i][j + 1].grainID) {
								newBoard[i][j].grainID = board[i + 1][j].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i + 1][j].getFillColor());
								continue;
							}
							else if (board[i + 1][j].grainID == board[i][j - 1].grainID) {
								newBoard[i][j].grainID = board[i + 1][j].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i + 1][j].getFillColor());
								continue;
							}
							else {
								newBoard[i][j].grainID = board[i][j].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i][j].getFillColor());
								continue;
							}
						}
						else if (board[i][j + 1].grainID > 1 && board[i][j + 1].grainID == board[i][j - 1].grainID) {
							if (board[i][j+1].grainID == board[i+1][j].grainID) {
								newBoard[i][j].grainID = board[i][j + 1].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i][j + 1].getFillColor());
								continue;
							}
							else if (board[i][j + 1].grainID == board[i-1][j].grainID) {
								newBoard[i][j].grainID = board[i][j + 1].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i][j + 1].getFillColor());
								continue;
							}
							else {
								newBoard[i][j].grainID = board[i][j].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i][j].getFillColor());
								continue;
							}
						}

						// X - rule
						if (board[i + 1][j+1].grainID > 1 && board[i + 1][j + 1].grainID == board[i - 1][j - 1].grainID) {
							if (board[i + 1][j+1].grainID == board[i-1][j + 1].grainID) {
								newBoard[i][j].grainID = board[i + 1][j+1].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i + 1][j+1].getFillColor());
								continue;
							}
							else if (board[i + 1][j+1].grainID == board[i+1][j - 1].grainID) {
								newBoard[i][j].grainID = board[i + 1][j+1].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i + 1][j+1].getFillColor());
								continue;
							}
							else {
								newBoard[i][j].grainID = board[i][j].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i][j].getFillColor());
								continue;
							}
						}
						else if (board[i-1][j + 1].grainID > 1 && board[i-1][j + 1].grainID == board[i+1][j - 1].grainID) {
							if (board[i-1][j + 1].grainID == board[i + 1][j+1].grainID) {
								newBoard[i][j].grainID = board[i-1][j + 1].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i-1][j + 1].getFillColor());
								continue;
							}
							else if (board[i-1][j + 1].grainID == board[i - 1][j-1].grainID) {
								newBoard[i][j].grainID = board[i-1][j + 1].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i-1][j + 1].getFillColor());
								continue;
							}
							else {
								newBoard[i][j].grainID = board[i][j].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i][j].getFillColor());
								continue;
							}
						}
						else {
							int prob = rand() % 100;
							if (prob <= probablityOfRule4) {
								newBoard[i][j].grainID = neighbours[index].first->grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(neighbours[index].first->getFillColor());
								continue;
							}
							else {
								newBoard[i][j].grainID = board[i][j].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i][j].getFillColor());
								continue;
							}
						}
					}
					else { //rule1
						newBoard[i][j].grainID = neighbours[index].first->grainID;
						newBoard[i][j].alive = 1;
						newBoard[i][j].setFillColor(neighbours[index].first->getFillColor());
					}
				}
			}
			else {
				newBoard[i][j].grainID = board[i][j].grainID;
				newBoard[i][j].alive = 1;
				newBoard[i][j].setFillColor(board[i][j].getFillColor());
			}

	return emptyGrains;
}
bool controlledGrainGrowth2(Grain(**board), Grain(**newBoard)) {
	bool emptyGrains = 0;
	for (int i = 1; i < width - 1; i++)
		for (int j = 1; j < height - 1; j++)
			if (board[i][j].grainID == 0) {
				emptyGrains = 1;
				vector<pair<Grain*, int>> neighbours;
				for (int a = -1; a < 2; a++)
					for (int b = -1; b < 2 && (b != 0 | a != 0); b++)
						if (board[i + a][j + b].grainID > 1)
							neighbours.push_back(make_pair(&board[i + a][j + b], 1));
				if (neighbours.size()) {
					for (int y = 0; y < neighbours.size(); y++)
						for (int z = y + 1; z < neighbours.size(); z++)
							if (neighbours[y].first->grainID == neighbours[z].first->grainID)
								neighbours[y].second++;
					int max = 1;
					int index = 0;
					for (int y = 0; y < neighbours.size(); y++)
						if (neighbours[y].second > max) {
							max = neighbours[y].second;
							index = y;
						}

					if (max < 3) {
						//rule4
						int prob = rand() % 100;
						if (prob <= probablityOfRule4) {
							newBoard[i][j].grainID = neighbours[index].first->grainID;
							newBoard[i][j].alive = 1;
							newBoard[i][j].setFillColor(neighbours[index].first->getFillColor());
							continue;
						}
						else {
							newBoard[i][j].grainID = board[i][j].grainID;
							newBoard[i][j].alive = 1;
							newBoard[i][j].setFillColor(board[i][j].getFillColor());
						}
					}
					else if (max < 5) {
						//rule2 lub rule 3
						// + rule
						if (board[i + 1][j].grainID > 1 && board[i + 1][j].grainID == board[i - 1][j].grainID) {
							if (board[i + 1][j].grainID == board[i][j + 1].grainID) {
								newBoard[i][j].grainID = board[i + 1][j].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i + 1][j].getFillColor());
								continue;
							}
							else if (board[i + 1][j].grainID == board[i][j - 1].grainID) {
								newBoard[i][j].grainID = board[i + 1][j].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i + 1][j].getFillColor());
								continue;
							}
							else {
								newBoard[i][j].grainID = board[i][j].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i][j].getFillColor());
								continue;
							}
						}
						else if (board[i][j + 1].grainID > 1 && board[i][j + 1].grainID == board[i][j - 1].grainID) {
							if (board[i][j + 1].grainID == board[i + 1][j].grainID) {
								newBoard[i][j].grainID = board[i][j + 1].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i][j + 1].getFillColor());
								continue;
							}
							else if (board[i][j + 1].grainID == board[i - 1][j].grainID) {
								newBoard[i][j].grainID = board[i][j + 1].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i][j + 1].getFillColor());
								continue;
							}
							else {
								newBoard[i][j].grainID = board[i][j].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i][j].getFillColor());
								continue;
							}
						}

						// X - rule
						if (board[i + 1][j + 1].grainID > 1 && board[i + 1][j + 1].grainID == board[i - 1][j - 1].grainID) {
							if (board[i + 1][j + 1].grainID == board[i - 1][j + 1].grainID) {
								newBoard[i][j].grainID = board[i + 1][j + 1].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i + 1][j + 1].getFillColor());
								continue;
							}
							else if (board[i + 1][j + 1].grainID == board[i + 1][j - 1].grainID) {
								newBoard[i][j].grainID = board[i + 1][j + 1].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i + 1][j + 1].getFillColor());
								continue;
							}
							else {
								newBoard[i][j].grainID = board[i][j].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i][j].getFillColor());
								continue;
							}
						}
						else if (board[i - 1][j + 1].grainID > 1 && board[i - 1][j + 1].grainID == board[i + 1][j - 1].grainID) {
							if (board[i - 1][j + 1].grainID == board[i + 1][j + 1].grainID) {
								newBoard[i][j].grainID = board[i - 1][j + 1].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i - 1][j + 1].getFillColor());
								continue;
							}
							else if (board[i - 1][j + 1].grainID == board[i - 1][j - 1].grainID) {
								newBoard[i][j].grainID = board[i - 1][j + 1].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i - 1][j + 1].getFillColor());
								continue;
							}
							else {
								newBoard[i][j].grainID = board[i][j].grainID;
								newBoard[i][j].alive = 1;
								newBoard[i][j].setFillColor(board[i][j].getFillColor());
								continue;
							}
						}
					}
					else { //rule1
						newBoard[i][j].grainID = neighbours[index].first->grainID;
						newBoard[i][j].alive = 1;
						newBoard[i][j].setFillColor(neighbours[index].first->getFillColor());
					}
				}
			}
			else {
				newBoard[i][j].grainID = board[i][j].grainID;
				newBoard[i][j].alive = 1;
				newBoard[i][j].setFillColor(board[i][j].getFillColor());
			}

	return emptyGrains;
}
void vonNeumannNeighbourhood(Grain(**board), Grain(**newBoard))
{
	for (int i = 1; i < width-1; i++)
		for (int j = 1; j < height-1; j++)
		{
			if (board[i][j].grainID > 1)
			{
				newBoard[i][j].alive = 1;
				newBoard[i][j].grainID = board[i][j].grainID;
				newBoard[i][j].setFillColor(board[i][j].getFillColor());
				if (board[i + 1][j].alive == 0)
				{
					newBoard[i + 1][j].alive = 1;
					newBoard[i + 1][j].grainID = board[i][j].grainID;
					newBoard[i + 1][j].setFillColor(board[i][j].getFillColor());
				}
				if (board[i - 1][j].alive == 0)
				{
					newBoard[i - 1][j].alive = 1;
					newBoard[i - 1][j].grainID = board[i][j].grainID;
					newBoard[i - 1][j].setFillColor(board[i][j].getFillColor());
				}
				if (board[i][j + 1].alive == 0)
				{
					newBoard[i][j + 1].alive = 1;
					newBoard[i][j + 1].grainID = board[i][j].grainID;
					newBoard[i][j + 1].setFillColor(board[i][j].getFillColor());
				}
				if (board[i][j - 1].alive == 0)
				{
					newBoard[i][j - 1].alive = 1;
					newBoard[i][j - 1].grainID = board[i][j].grainID;
					newBoard[i][j - 1].setFillColor(board[i][j].getFillColor());
				}
			}
		}

}
void rule1(Grain(**board)) {
	for (int i = 2; i < width - 2; i++)
		for (int j = 2; j < height - 2; j++)
			if (board[i][j].grainID > 1)
			{
				for (int a = -1; a < 2; a++)
					if (board[i + a][j + 1].grainID != board[i][j].grainID && board[i + a][j + 1].grainID > 1) {
						int counter = 0;
						for (int y = -1; y < 2; y++)
							for (int z = -1; z < 2; z++)
								if (board[i + a][j + 1].grainID == board[i + y][j + z].grainID)
									counter++;
						if (counter > 4) {
							board[i][j] = board[i + a][j + 1];
							break;
						}
					}

				if (board[i + 1][j].grainID != board[i][j].grainID && board[i + 1][j].grainID > 1) {
					int counter = 0;
					for (int y = -1; y < 2; y++)
						for (int z = -1; z < 2; z++)
							if (board[i + 1][j].grainID == board[i + y][j + z].grainID)
								counter++;
					if (counter > 4) {
						board[i][j] = board[i + 1][j];
						break;
					}
				}
			}
}
void rule2(Grain(**board)) {
	for (int i = 2; i < width - 2; i++)
		for (int j = 2; j < height - 2; j++)
			if (board[i][j].grainID > 1) {
///
			}
	
}
void mooreNeighbourhood(Grain(**board), Grain(**newBoard))
{
	for (int i = 1; i < width-1; i++)
		for (int j = 1; j < height-1; j++)
		{
			if (board[i][j].alive == 1)
			{
				newBoard[i][j].alive = 1;
				newBoard[i][j].grainID = board[i][j].grainID;
				newBoard[i][j].setFillColor(board[i][j].getFillColor());
				if (board[i + 1][j].alive == 0)
				{
					newBoard[i + 1][j].alive = 1;
					newBoard[i + 1][j].grainID = board[i][j].grainID;
					newBoard[i + 1][j].setFillColor(board[i][j].getFillColor());
				}
				if (board[i - 1][j].alive == 0)
				{
					newBoard[i - 1][j].alive = 1;
					newBoard[i - 1][j].grainID = board[i][j].grainID;
					newBoard[i - 1][j].setFillColor(board[i][j].getFillColor());
				}
				if (board[i][j + 1].alive == 0)
				{
					newBoard[i][j + 1].alive = 1;
					newBoard[i][j + 1].grainID = board[i][j].grainID;
					newBoard[i][j + 1].setFillColor(board[i][j].getFillColor());
				}
				if (board[i][j - 1].alive == 0)
				{
					newBoard[i][j - 1].alive = 1;
					newBoard[i][j - 1].grainID = board[i][j].grainID;
					newBoard[i][j - 1].setFillColor(board[i][j].getFillColor());
				}
				if (board[i + 1][j + 1].alive == 0)
				{
					newBoard[i + 1][j + 1].alive = 1;
					newBoard[i + 1][j + 1].grainID = board[i][j].grainID;
					newBoard[i + 1][j + 1].setFillColor(board[i][j].getFillColor());
				}
				if (board[i - 1][j + 1].alive == 0)
				{
					newBoard[i - 1][j + 1].alive = 1;
					newBoard[i - 1][j + 1].grainID = board[i][j].grainID;
					newBoard[i - 1][j + 1].setFillColor(board[i][j].getFillColor());
				}
				if (board[i - 1][j - 1].alive == 0)
				{
					newBoard[i - 1][j - 1].alive = 1;
					newBoard[i - 1][j - 1].grainID = board[i][j].grainID;
					newBoard[i - 1][j - 1].setFillColor(board[i][j].getFillColor());
				}
				if (board[i + 1][j - 1].alive == 0)
				{
					newBoard[i + 1][j - 1].alive = 1;
					newBoard[i + 1][j - 1].grainID = board[i][j].grainID;
					newBoard[i + 1][j - 1].setFillColor(board[i][j].getFillColor());
				}
			}
		}
}
void hexLeftNeighbourhood(Grain(**board), Grain(**newBoard))
{
	for (int i = 0; i < width; i++)
		for (int j = 0; j < height; j++)
		{
			if (board[i][j].alive == 1)
			{
				newBoard[i][j].alive = 1;
				newBoard[i][j].grainID = board[i][j].grainID;
				newBoard[i][j].setFillColor(board[i][j].getFillColor());
				if (board[i + 1][j].alive == 0)
				{
					newBoard[i + 1][j].alive = 1;
					newBoard[i + 1][j].grainID = board[i][j].grainID;
					newBoard[i + 1][j].setFillColor(board[i][j].getFillColor());
				}
				if (board[i - 1][j].alive == 0)
				{
					newBoard[i - 1][j].alive = 1;
					newBoard[i - 1][j].grainID = board[i][j].grainID;
					newBoard[i - 1][j].setFillColor(board[i][j].getFillColor());
				}
				if (board[i][j + 1].alive == 0)
				{
					newBoard[i][j + 1].alive = 1;
					newBoard[i][j + 1].grainID = board[i][j].grainID;
					newBoard[i][j + 1].setFillColor(board[i][j].getFillColor());
				}
				if (board[i][j - 1].alive == 0)
				{
					newBoard[i][j - 1].alive = 1;
					newBoard[i][j - 1].grainID = board[i][j].grainID;
					newBoard[i][j - 1].setFillColor(board[i][j].getFillColor());
				}
				if (board[i + 1][j + 1].alive == 0)
				{
					newBoard[i + 1][j + 1].alive = 1;
					newBoard[i + 1][j + 1].grainID = board[i][j].grainID;
					newBoard[i + 1][j + 1].setFillColor(board[i][j].getFillColor());
				}
				if (board[i - 1][j - 1].alive == 0)
				{
					newBoard[i - 1][j - 1].alive = 1;
					newBoard[i - 1][j - 1].grainID = board[i][j].grainID;
					newBoard[i - 1][j - 1].setFillColor(board[i][j].getFillColor());
				}
			}
		}
}
void hexRightNeighbourhood(Grain(**board), Grain(**newBoard))
{
	for (int i = 0; i < width; i++)
		for (int j = 0; j < height; j++)
		{
			if (board[i][j].alive == 1)
			{
				newBoard[i][j].alive = 1;
				newBoard[i][j].grainID = board[i][j].grainID;
				newBoard[i][j].setFillColor(board[i][j].getFillColor());
				if (board[i + 1][j].alive == 0)
				{
					newBoard[i + 1][j].alive = 1;
					newBoard[i + 1][j].grainID = board[i][j].grainID;
					newBoard[i + 1][j].setFillColor(board[i][j].getFillColor());
				}
				if (board[i - 1][j].alive == 0)
				{
					newBoard[i - 1][j].alive = 1;
					newBoard[i - 1][j].grainID = board[i][j].grainID;
					newBoard[i - 1][j].setFillColor(board[i][j].getFillColor());
				}
				if (board[i][j + 1].alive == 0)
				{
					newBoard[i][j + 1].alive = 1;
					newBoard[i][j + 1].grainID = board[i][j].grainID;
					newBoard[i][j + 1].setFillColor(board[i][j].getFillColor());
				}
				if (board[i][j - 1].alive == 0)
				{
					newBoard[i][j - 1].alive = 1;
					newBoard[i][j - 1].grainID = board[i][j].grainID;
					newBoard[i][j - 1].setFillColor(board[i][j].getFillColor());
				}
				if (board[i - 1][j + 1].alive == 0)
				{
					newBoard[i - 1][j + 1].alive = 1;
					newBoard[i - 1][j + 1].grainID = board[i][j].grainID;
					newBoard[i - 1][j + 1].setFillColor(board[i][j].getFillColor());
				}
				if (board[i + 1][j - 1].alive == 0)
				{
					newBoard[i + 1][j - 1].alive = 1;
					newBoard[i + 1][j - 1].grainID = board[i][j].grainID;
					newBoard[i + 1][j - 1].setFillColor(board[i][j].getFillColor());
				}
			}
		}
}
void pentaRandomNeighbourhood(Grain(**board), Grain(**newBoard))
{
	for (int i = 0; i < width; i++)
		for (int j = 0; j < height; j++)
		{
			if (board[i][j].alive == 1)
			{
				newBoard[i][j].alive = 1;
				newBoard[i][j].grainID = board[i][j].grainID;
				newBoard[i][j].setFillColor(board[i][j].getFillColor());
				char option = rand() % 4;
				switch (option) {
					case 0: {
						//gora
						if (board[i - 1][j].alive == 0)
						{
							newBoard[i - 1][j].alive = 1;
							newBoard[i - 1][j].grainID = board[i][j].grainID;
							newBoard[i - 1][j].setFillColor(board[i][j].getFillColor());
						}
						if (board[i - 1][j + 1].alive == 0)
						{
							newBoard[i - 1][j + 1].alive = 1;
							newBoard[i - 1][j + 1].grainID = board[i][j].grainID;
							newBoard[i - 1][j + 1].setFillColor(board[i][j].getFillColor());
						}
						if (board[i - 1][j - 1].alive == 0)
						{
							newBoard[i - 1][j - 1].alive = 1;
							newBoard[i - 1][j - 1].grainID = board[i][j].grainID;
							newBoard[i - 1][j - 1].setFillColor(board[i][j].getFillColor());
						}
						if (board[i][j + 1].alive == 0)
						{
							newBoard[i][j + 1].alive = 1;
							newBoard[i][j + 1].grainID = board[i][j].grainID;
							newBoard[i][j + 1].setFillColor(board[i][j].getFillColor());
						}
						if (board[i][j - 1].alive == 0)
						{
							newBoard[i][j - 1].alive = 1;
							newBoard[i][j - 1].grainID = board[i][j].grainID;
							newBoard[i][j - 1].setFillColor(board[i][j].getFillColor());
						}
						break;
					}
					case 1: {
						//lewa
						if (board[i - 1][j - 1].alive == 0)
						{
							newBoard[i - 1][j - 1].alive = 1;
							newBoard[i - 1][j - 1].grainID = board[i][j].grainID;
							newBoard[i - 1][j - 1].setFillColor(board[i][j].getFillColor());
						}
						if (board[i][j - 1].alive == 0)
						{
							newBoard[i][j - 1].alive = 1;
							newBoard[i][j - 1].grainID = board[i][j].grainID;
							newBoard[i][j - 1].setFillColor(board[i][j].getFillColor());
						}
						if (board[i + 1][j - 1].alive == 0)
						{
							newBoard[i + 1][j - 1].alive = 1;
							newBoard[i + 1][j - 1].grainID = board[i][j].grainID;
							newBoard[i + 1][j - 1].setFillColor(board[i][j].getFillColor());
						}
						if (board[i - 1][j].alive == 0)
						{
							newBoard[i - 1][j].alive = 1;
							newBoard[i - 1][j].grainID = board[i][j].grainID;
							newBoard[i - 1][j].setFillColor(board[i][j].getFillColor());
						}
						if (board[i + 1][j].alive == 0)
						{
							newBoard[i + 1][j].alive = 1;
							newBoard[i + 1][j].grainID = board[i][j].grainID;
							newBoard[i + 1][j].setFillColor(board[i][j].getFillColor());
						}
						break;
					}
					case 2: {
						//dol
						if (board[i + 1][j].alive == 0)
						{
							newBoard[i + 1][j].alive = 1;
							newBoard[i + 1][j].grainID = board[i][j].grainID;
							newBoard[i + 1][j].setFillColor(board[i][j].getFillColor());
						}
						if (board[i + 1][j - 1].alive == 0)
						{
							newBoard[i + 1][j - 1].alive = 1;
							newBoard[i + 1][j - 1].grainID = board[i][j].grainID;
							newBoard[i + 1][j - 1].setFillColor(board[i][j].getFillColor());
						}
						if (board[i + 1][j + 1].alive == 0)
						{
							newBoard[i + 1][j + 1].alive = 1;
							newBoard[i + 1][j + 1].grainID = board[i][j].grainID;
							newBoard[i + 1][j + 1].setFillColor(board[i][j].getFillColor());
						}
						if (board[i][j - 1].alive == 0)
						{
							newBoard[i][j - 1].alive = 1;
							newBoard[i][j - 1].grainID = board[i][j].grainID;
							newBoard[i][j - 1].setFillColor(board[i][j].getFillColor());
						}
						if (board[i][j + 1].alive == 0)
						{
							newBoard[i][j + 1].alive = 1;
							newBoard[i][j + 1].grainID = board[i][j].grainID;
							newBoard[i][j + 1].setFillColor(board[i][j].getFillColor());
						}
						break;
					}
					case 3: {
						//prawa
						if (board[i][j + 1].alive == 0)
						{
							newBoard[i][j + 1].alive = 1;
							newBoard[i][j + 1].grainID = board[i][j].grainID;
							newBoard[i][j + 1].setFillColor(board[i][j].getFillColor());
						}
						if (board[i + 1][j + 1].alive == 0)
						{
							newBoard[i + 1][j + 1].alive = 1;
							newBoard[i + 1][j + 1].grainID = board[i][j].grainID;
							newBoard[i + 1][j + 1].setFillColor(board[i][j].getFillColor());
						}
						if (board[i - 1][j + 1].alive == 0)
						{
							newBoard[i - 1][j + 1].alive = 1;
							newBoard[i - 1][j + 1].grainID = board[i][j].grainID;
							newBoard[i - 1][j + 1].setFillColor(board[i][j].getFillColor());
						}
						if (board[i + 1][j].alive == 0)
						{
							newBoard[i + 1][j].alive = 1;
							newBoard[i + 1][j].grainID = board[i][j].grainID;
							newBoard[i + 1][j].setFillColor(board[i][j].getFillColor());
						}
						if (board[i - 1][j].alive == 0)
						{
							newBoard[i - 1][j].alive = 1;
							newBoard[i - 1][j].grainID = board[i][j].grainID;
							newBoard[i - 1][j].setFillColor(board[i][j].getFillColor());
						}
						break;
					}
				}
			}
		}
}
void boardCreation(Grain(**board))
{
	int cellSize_Y = (screenHeight - boardCoordinate_y - 10) / height;
	int cellSize_X = (screenWidth - boardCoordinate_x - 10) / width;
	if (cellSize_X > cellSize_Y)
		cellSize = cellSize_Y;
	else
		cellSize = cellSize_Y;

	for (int i = 0; i < width; i++)
		for (int j = 0; j < height; j++)
		{
			board[i][j].alive = 0;
			board[i][j].ro = 0;
			board[i][j].grainID = 0;
			board[i][j].draw = 0;
			board[i][j].setSize(Vector2f(cellSize, cellSize)); //wielkosc pola to 100x100
			board[i][j].setPosition(boardCoordinate_x + (float)i * cellSize, boardCoordinate_y + (float)j * cellSize); // plansza zaczyna sie od 50 piksela, pola rozstawione co 15 pikseli
			board[i][j].setFillColor(Color(0, 0, 0));
		}
}
void drawbmp(string filename, Grain(**board), int WIDTH, int HEIGHT) {

	unsigned int headers[13];
	FILE * outfile;
	int extrabytes;
	int paddedsize;
	int x; int y; int n;
	int red, green, blue;

	extrabytes = 4 - ((WIDTH * 3) % 4);                 // How many bytes of padding to add to each
														// horizontal line - the size of which must
														// be a multiple of 4 bytes.
	if (extrabytes == 4)
		extrabytes = 0;

	paddedsize = ((WIDTH * 3) + extrabytes) * HEIGHT;

	// Headers...
	// Note that the "BM" identifier in bytes 0 and 1 is NOT included in these "headers".

	headers[0] = paddedsize + 54;      // bfSize (whole file size)
	headers[1] = 0;                    // bfReserved (both)
	headers[2] = 54;                   // bfOffbits
	headers[3] = 40;                   // biSize
	headers[4] = WIDTH;  // biWidth
	headers[5] = HEIGHT; // biHeight

						 // Would have biPlanes and biBitCount in position 6, but they're shorts.
						 // It's easier to write them out separately (see below) than pretend
						 // they're a single int, especially with endian issues...

	headers[7] = 0;                    // biCompression
	headers[8] = paddedsize;           // biSizeImage
	headers[9] = 0;                    // biXPelsPerMeter
	headers[10] = 0;                    // biYPelsPerMeter
	headers[11] = 0;                    // biClrUsed
	headers[12] = 0;                    // biClrImportant

	std::string str = filename + ".bmp";
	const char * c = str.c_str();
	outfile = fopen(c, "wb");

	//
	// Headers begin...
	// When printing ints and shorts, we write out 1 character at a time to avoid endian issues.
	//

	fprintf(outfile, "BM");

	for (n = 0; n <= 5; n++)
	{
		fprintf(outfile, "%c", headers[n] & 0x000000FF);
		fprintf(outfile, "%c", (headers[n] & 0x0000FF00) >> 8);
		fprintf(outfile, "%c", (headers[n] & 0x00FF0000) >> 16);
		fprintf(outfile, "%c", (headers[n] & (unsigned int)0xFF000000) >> 24);
	}

	// These next 4 characters are for the biPlanes and biBitCount fields.

	fprintf(outfile, "%c", 1);
	fprintf(outfile, "%c", 0);
	fprintf(outfile, "%c", 24);
	fprintf(outfile, "%c", 0);

	for (n = 7; n <= 12; n++)
	{
		fprintf(outfile, "%c", headers[n] & 0x000000FF);
		fprintf(outfile, "%c", (headers[n] & 0x0000FF00) >> 8);
		fprintf(outfile, "%c", (headers[n] & 0x00FF0000) >> 16);
		fprintf(outfile, "%c", (headers[n] & (unsigned int)0xFF000000) >> 24);
	}

	//
	// Headers done, now write the data...
	//

	for (y = HEIGHT - 1; y >= 0; y--)     // BMP image format is written from bottom to top...
	{
		for (x = 0; x <= WIDTH - 1; x++)
		{
			Color c = board[x][y].getFillColor();
			red = c.r;
			green = c.g;
			blue = c.b;

			if (red > 255) red = 255; if (red < 0) red = 0;
			if (green > 255) green = 255; if (green < 0) green = 0;
			if (blue > 255) blue = 255; if (blue < 0) blue = 0;

			// Also, it's written in (b,g,r) format...

			fprintf(outfile, "%c", blue);
			fprintf(outfile, "%c", green);
			fprintf(outfile, "%c", red);
		}
		if (extrabytes)      // See above - BMP lines must be of lengths divisible by 4.
		{
			for (n = 1; n <= extrabytes; n++)
			{
				fprintf(outfile, "%c", 0);
			}
		}
	}

	fclose(outfile);
	return;
}
void readBMP(string filename, Grain(**board)) {
	int i;
	std::string str = filename + ".bmp";
	const char * c = str.c_str();
	FILE* f = fopen(c, "rb");
	unsigned char info[54];
	fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header

	// extract image height and width from header
	int size = *(int*)&info[2] - 54;
	width = *(int*)&info[18];
	height = *(int*)&info[22];
	int extrabytes = size / height - 3 * width;               // How many bytes of padding to add to each
													// horizontal line - the size of which must
													// be a multiple of 4 bytes.

	unsigned char* data = new unsigned char[size]; // allocate 3 bytes per pixel
	fread(data, sizeof(unsigned char), size, f); // read the rest of the data at once
	fclose(f);

	int red, green, blue;
	boardCreation(board);

	int ii = 0;
	for (int y = height - 1; y >= 0; y--)     // BMP image format is written from bottom to top...
	{
		for (int x = 0; x <= width - 1; x++)
		{
			red = data[ii];
			green = data[ii + 1];
			blue = data[ii + 2];
			board[x][y].setFillColor((Color(red, green, blue)));
			ii += 3;
		}
		ii += extrabytes;
	}
	return;
}
void readTXT(string filename, Grain(**board)) {
	ifstream input(filename);
	cout << endl << filename << endl;
	if (input.is_open())
	{
		string data;
		getline(input, data);
		width = stoi(data);
		getline(input, data);
		height = stoi(data);
		getline(input, data);
		numberOfGrains = stoi(data);
		boardCreation(board);

		for (int i = 1; i < width - 1; i++)
			for (int j = 1; j < height - 1; j++) {
				getline(input, data);
				board[i][j].grainID = stoi(data);
				if (board[i][j].grainID > 0)
					board[i][j].alive = 1;
			}

		Color* colors = new Color[numberOfGrains+2];
		for (int i = 0; i < numberOfGrains+2; i++)
			colors[i] = newColor();
		colors[0] = Color(0,0,0); //borders
		colors[1] = Color(0, 0, 0); //empty grains
		colors[2] = Color(255, 255, 255); //inclusions
		
		for (int i = 1; i < width - 1; i++)
			for (int j = 1; j < height - 1; j++)
				board[i][j].setFillColor(colors[board[i][j].grainID+1]);

		input.close();
	}
	else cout << "Unable to open file";
}
void saveAsTxt(string filename, Grain(**board)) {
	ofstream output;
	output.open(filename);
	cout << endl << filename << endl;
	if (output.is_open())
	{
		output << width << "\n" << height << "\n" << numberOfGrains;
		for (int i = 1; i < width - 1; i++)
			for (int j = 1; j < height - 1; j++)
				output << "\n" << board[i][j].grainID;
		output.close();
	}
	else cout << "Unable to save file";
}
void addInclusions(Grain(**board), int number, int size, bool type) {
	if (type) {
		bool canBePlaced;
		while (number > 0) {
			canBePlaced = 1;
			int x = rand() % (width - 2*size - 2) + 1 + size;
			int y = rand() % (height - 2*size - 2) + 1 + size;
			for (int j = -size; j < size; j++)
				for (int k = -size; k < size; k++)
					if (j*j + k*k < size * size && board[x + j][y + k].grainID == 1) {
						canBePlaced = 0;
						break;
					}
			if (canBePlaced) {
				for (int j = -size; j < size; j++)
					for (int k = -size; k < size; k++) 
						if ((j*j + k*k) < (size * size)) {
							board[x + j][y + k].grainID = 1;
							board[x + j][y + k].alive = 1;
							board[x + j][y + k].setFillColor(Color(255, 255, 255));
						}
				number--;
			}
		}
	}
	else {
		bool canBePlaced;
		while (number > 0) {
			canBePlaced = 1;
			int x = rand() % (width - size - 2) + 1;
			int y = rand() % (height - size - 2) + 1;

			for (int j = 0; j < size; j++)
				for (int k = 0; k < size; k++)
					if (board[x + j][y + k].grainID == 1) {
						canBePlaced = 0;
						break;
					}
			if (canBePlaced) {
				for (int j = 0; j < size; j++)
					for (int k = 0; k < size; k++) {
						board[x + j][y + k].grainID = 1;
						board[x + j][y + k].alive = 1;
						board[x + j][y + k].setFillColor(Color(255, 255, 255));
					}
				number--;
			}
		}
	}
}
void addInclusionsAfterSimulation(Grain(**board), int number, int size, bool type) {
	if (type) {
		bool canBePlaced;
		while (number > 0) {
			canBePlaced = 1;
			int x = rand() % (width - 2 * size - 2) + 1 + size;
			int y = rand() % (height - 2 * size - 2) + 1 + size;
			if (isOnGrainBorder(board, x, y)) {
				for (int j = -size; j < size; j++)
					for (int k = -size; k < size; k++)
						if (j*j + k * k < size * size && board[x + j][y + k].grainID == 1) {
							canBePlaced = 0;
							break;
						}
				if (canBePlaced) {
					for (int j = -size; j < size; j++)
						for (int k = -size; k < size; k++)
							if ((j*j + k * k) < (size * size)) {
								board[x + j][y + k].grainID = 1;
								board[x + j][y + k].alive = 1;
								board[x + j][y + k].setFillColor(Color(255, 255, 255));
							}
					number--;
				}
			}
		}
	}
	else {
		bool canBePlaced;
		while (number > 0) {
			canBePlaced = 1;
			int x = rand() % (width - size - 2) + 1;
			int y = rand() % (height - size - 2) + 1;

			if (isOnGrainBorder(board, x, y)) {
				for (int j = 0; j < size; j++)
					for (int k = 0; k < size; k++)
						if (board[x + j][y + k].grainID == 1) {
							canBePlaced = 0;
							break;
						}
				if (canBePlaced) {
					for (int j = 0; j < size; j++)
						for (int k = 0; k < size; k++) {
							board[x + j][y + k].grainID = 1;
							board[x + j][y + k].alive = 1;
							board[x + j][y + k].setFillColor(Color(255, 255, 255));
						}
					number--;
				}
			}
		}
	}
}
bool isChoosen(int id) {
	bool is = 0;
	if (choosen.size())
		for (int i = 0; i < choosen.size(); i++) {
			if (choosen[i] == id) {
				is = 1;
				break;
			}
		}
	return is;
}

Button start("START/STOP", arimo, 30, menuCoordinateX + 55, menuCoordinateY + 12, 280, 40, menuCoordinateX + 10, menuCoordinateY + 10);
Button restart("RESTART", arimo, 20, menuCoordinateX + 95, menuCoordinateY + 63, 280, 30, menuCoordinateX + 10, menuCoordinateY + 60);
Button losuj("GENERATE NUCLEUSES", arimo, 20, menuCoordinateX + 30, menuCoordinateY + 135, 280, 30, menuCoordinateX + 10, menuCoordinateY + 132);
MyText inclusionsNumber_Text("Inclusions number:", arimo, 15, menuCoordinateX + 10, menuCoordinateY + 315);
MyText inclusionsSize_Text("Inclusions size:", arimo, 15, menuCoordinateX + 10, menuCoordinateY + 345);
MyText inclusionsType_Text("Inclusions type:", arimo, 15, menuCoordinateX + 10, menuCoordinateY + 380);
TextField inclusionsNumber_TextField("10", arimo, 15, menuCoordinateX + 155, menuCoordinateY + 315, 140, 25, menuCoordinateX + 150, menuCoordinateY + 312);
TextField inclusionsSize_TextField("5", arimo, 15, menuCoordinateX + 155, menuCoordinateY + 345, 140, 25, menuCoordinateX + 150, menuCoordinateY + 342);
Button inclusionsTypeCircular_Button("Circular", arimo, 15, menuCoordinateX + 157, menuCoordinateY + 380, 65, 25, menuCoordinateX + 150, menuCoordinateY + 377);
Button inclusionsTypeSquare_Button("Square", arimo, 15, menuCoordinateX + 234, menuCoordinateY + 380, 65, 25, menuCoordinateX + 225, menuCoordinateY + 377);
Button addInclusions_Button("ADD INCLUSIONS", arimo, 20, menuCoordinateX + 60, menuCoordinateY + 415, 280, 30, menuCoordinateX + 10, menuCoordinateY + 412);
TextField shapeControl_TextField("Shape control:", arimo, 15, menuCoordinateX + 10, menuCoordinateY + 460, 140, 25, menuCoordinateX + 10, menuCoordinateY + 460);
Button shapeControl_Button("ON", arimo, 15, menuCoordinateX + 123, menuCoordinateY + 460, 30, 25, menuCoordinateX + 120, menuCoordinateY + 457);
TextField shapeControlProbability_TextField("50", arimo, 15, menuCoordinateX + 175, menuCoordinateY + 460, 25, 25, menuCoordinateX + 170, menuCoordinateY + 457);
MyText percent_Text("%", arimo, 15, menuCoordinateX + 202, menuCoordinateY + 460);
Button selectGrains_Button("SELECT GRAINS", arimo, 20, menuCoordinateX + 65, menuCoordinateY + 500, 280, 30, menuCoordinateX + 10, menuCoordinateY + 497);
Button substructure_Button("Substructure", arimo, 15, menuCoordinateX + 35, menuCoordinateY + 540, 135, 25, menuCoordinateX + 10, menuCoordinateY + 537);
Button dualPhase_Button("Dual phase", arimo, 15, menuCoordinateX + 185, menuCoordinateY + 540, 135, 25, menuCoordinateX + 155, menuCoordinateY + 537);
MyText boundarySize_Text("Boundary size:", arimo, 15, menuCoordinateX + 10, menuCoordinateY + 585);
TextField boundarySize_TextField("1", arimo, 15, menuCoordinateX + 160, menuCoordinateY + 585, 135, 25, menuCoordinateX + 155, menuCoordinateY + 582);
Button generateBoundaries_Button("GENERATE BOUNDARIES", arimo, 20, menuCoordinateX + 25, menuCoordinateY + 620, 280, 30, menuCoordinateX + 10, menuCoordinateY + 617);
MyText saveTo_Text("Save as:", arimo, 15, menuCoordinateX + 10, menuCoordinateY + 900);
Button save_bmp("*.bmp file", arimo, 15, menuCoordinateX + 98, menuCoordinateY + 900, 100, 25, menuCoordinateX + 80, menuCoordinateY + 897);
Button save_txt("*.txt file", arimo, 15, menuCoordinateX + 215, menuCoordinateY + 900, 100, 25, menuCoordinateX + 190, menuCoordinateY + 897);
MyText savePath_Text("Path:", arimo, 15, menuCoordinateX + 10, menuCoordinateY + 935);
TextField saveFilename_TextField("map", arimo, 15, menuCoordinateX + 85, menuCoordinateY + 935, 210, 25, menuCoordinateX + 80, menuCoordinateY + 932);
MyText importFrom_Text("Import:", arimo, 15, menuCoordinateX + 10, menuCoordinateY + 980);
Button import_bmp("*.bmp file", arimo, 15, menuCoordinateX + 98, menuCoordinateY + 980, 100, 25, menuCoordinateX + 80, menuCoordinateY + 977);
Button import_txt("*.txt file", arimo, 15, menuCoordinateX + 215, menuCoordinateY + 980, 100, 25, menuCoordinateX + 190, menuCoordinateY + 977);
MyText importPath_Text("Path:", arimo, 15, menuCoordinateX + 10, menuCoordinateY + 1015);
TextField importFilename_TextField("map", arimo, 15, menuCoordinateX + 85, menuCoordinateY + 1015, 210, 25, menuCoordinateX + 80, menuCoordinateY + 1012);

int main()
{
	srand(time(NULL));
	MyRectangleShape menuFrame(300, 1060, menuCoordinateX, menuCoordinateY);

	start.rectangle.setFillColor(Color(200,0,0));
	//MyText title1("Neighbourhood:", arimo, 25, menuCoordinateX + 10, menuCoordinateY + 70);
	//TextField title2("von Neumann'a", arimo, 20, menuCoordinateX + 40, menuCoordinateY + 100, 100, 30, menuCoordinateX + 10, menuCoordinateY + 100);
	//TextField title3("Moore'a", arimo, 20, menuCoordinateX + 40, menuCoordinateY + 130, 100, 30, menuCoordinateX + 10, menuCoordinateY + 130);
	//TextField title4("Hex left", arimo, 20, menuCoordinateX + 40, menuCoordinateY + 160, 100, 30, menuCoordinateX + 10, menuCoordinateY + 160);
	//TextField title5("Hex right", arimo, 20, menuCoordinateX + 40, menuCoordinateY + 190, 100, 30, menuCoordinateX + 10, menuCoordinateY + 190);
	//TextField title6("Hex random", arimo, 20, menuCoordinateX + 40, menuCoordinateY + 220, 100, 30, menuCoordinateX + 10, menuCoordinateY + 220);
	//TextField title7("Penta random", arimo, 20, menuCoordinateX + 40, menuCoordinateY + 250, 100, 30, menuCoordinateX + 10, menuCoordinateY + 250);
	//MyText title10("Boundary conditions:", arimo, 25, menuCoordinateX + 10, menuCoordinateY + 280);
	//TextField title8("Periodic", arimo, 20, menuCoordinateX + 40, menuCoordinateY + 310, 100, 30, menuCoordinateX + 10, menuCoordinateY + 310);
	//TextField title9("Non-periodic", arimo, 20, menuCoordinateX + 40, menuCoordinateY + 340, 100, 30, menuCoordinateX + 10, menuCoordinateY + 340);
	//MyText title11("seed:", arimo, 25, menuCoordinateX + 10, menuCoordinateY + 370);
	TextField title12("Number of initial grains:", arimo, 15, menuCoordinateX + 10, menuCoordinateY + 100, 100, 25, menuCoordinateX + 10, menuCoordinateY + 100);
	TextField numberOfRandomGrains("", arimo, 15, menuCoordinateX + 180, menuCoordinateY + 100, 115, 25, menuCoordinateX + 175, menuCoordinateY + 97);
	//TextField title13("Rownomierne", arimo, 20, menuCoordinateX + 40, menuCoordinateY + 430, 100, 30, menuCoordinateX + 10, menuCoordinateY + 430);
	//TextField title14("Losowe z promieniem R", arimo, 20, menuCoordinateX + 40, menuCoordinateY + 460, 100, 30, menuCoordinateX + 10, menuCoordinateY + 460);
	//TextField title15("Przez klikniecie", arimo, 20, menuCoordinateX + 40, menuCoordinateY + 490, 100, 30, menuCoordinateX + 10, menuCoordinateY + 490);
	MyText title18("Space dimension:", arimo, 25, menuCoordinateX + 10, menuCoordinateY + 180);
	MyText menuText_X("X:", arimo, 25, menuCoordinateX + 40, menuCoordinateY + 220);
	MyText menuText_Y("Y:", arimo, 25, menuCoordinateX + 40, menuCoordinateY + 260);
	TextField Y_grainsNumber_TextField("207", arimo, 20, menuCoordinateX + 85, menuCoordinateY + 220, 210, 30, menuCoordinateX + 80, menuCoordinateY + 217);
	TextField X_grainsNumber_TextField("321", arimo, 20, menuCoordinateX + 85, menuCoordinateY + 260, 210, 30, menuCoordinateX + 80, menuCoordinateY + 257);
	inclusionsTypeCircular_Button.rectangle.setFillColor(Color(200, 0, 0));
	
	char kindOfNeighbourhood = 0, grainDraw = 0, ENTERING_TEXT = 0, ENTERING_TEXT2 = 0;
	bool INIT = 1, READY = 0, START = 0, STOP = 0, gameStop = 0, borderCondition = 1, monteCarlo = 0, circularInclusions = 1, SIMULATION_FINISHED = 0, CA_SUBSTRUCTURE = 0, BORDERS = 0;
	bool RESTART = 0, SHAPE_CONTROL = 0;
	int initialNumberOfGrains = 200;
	numberOfRandomGrains.text.setString(std::to_string(initialNumberOfGrains));
	Y_grainsNumber_TextField.text.setString(std::to_string(height));
	X_grainsNumber_TextField.text.setString(std::to_string(width));

	
	Grain **board = NULL, **newBoard = NULL;
	board = new Grain*[width];
	for (int i = 0; i < width; ++i)
		board[i] = new Grain[height];
	newBoard = new Grain*[width];
	for (int i = 0; i < width; ++i)
		newBoard[i] = new Grain[height];
	boardCreation(board);
	boardCreation(newBoard);
	
	Vector2f mousePosition;
	int x = 0, y = 0;

	sf::RenderWindow window(sf::VideoMode(1920, 1080, 32), "Multiscale Modelling", sf::Style::Default);	//glowne okno aplikacji
	window.setPosition(Vector2i(0, 0));

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (START)
			{
				if (restart.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
				{
					restart.rectangle.setOutlineColor(Color(200, 200, 200));
					if (Mouse::isButtonPressed(Mouse::Left))
					{
						START = 0;
						RESTART = 1;
						break;
					}
				}
				else restart.rectangle.setOutlineColor(Color(40, 40, 40));

				if (selectGrains_Button.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
				{
					selectGrains_Button.rectangle.setOutlineColor(Color(200, 200, 200));
					if (Mouse::isButtonPressed(Mouse::Left))
					{
						START = 0;
						SIMULATION_FINISHED = 1;
						break;
					}
				}
				else selectGrains_Button.rectangle.setOutlineColor(Color(40, 40, 40));

				if (start.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
				{
					start.rectangle.setOutlineColor(Color(200, 200, 200));
					if (Mouse::isButtonPressed(Mouse::Left))
					{
						START = 0;
						STOP = 1;
						break;
					}
				}
				else {
					start.rectangle.setOutlineColor(Color(40, 40, 40));
					//vonNeumannNeighbourhood(board, newBoard);
					//mooreNeighbourhood(board,newBoard);
					if (SHAPE_CONTROL) {
						if (!controlledGrainGrowth(board, newBoard)) {
							START = 0;
							SIMULATION_FINISHED = 1;
						}
					} else if (!grainGrowth(board, newBoard)) {
						START = 0;
						SIMULATION_FINISHED = 1;
					}

					for (int i = 1; i < width - 1; i++)
						for (int j = 1; j < height - 1; j++)
							board[i][j] = newBoard[i][j];
					//START = 0;
					//STOP = 1;
					break;
				}

				/*switch (kindOfNeighbourhood) {
				case 0: {
					vonNeumannNeighbourhood(board, newBoard);
					break;
				}
				case 1: {
					mooreNeighbourhood(board, newBoard);
					break;
				}
				case 2: {
					hexLeftNeighbourhood(board, newBoard);
					break;
				}
				case 3: {
					hexRightNeighbourhood(board, newBoard);
					break;
				}
				case 4: {
					if (rand() % 2)
						hexLeftNeighbourhood(board, newBoard);
					else
						hexRightNeighbourhood(board, newBoard);
					break;
				}
				case 5: {
					pentaRandomNeighbourhood(board, newBoard);
					break;
				}
				*/
			}
			if (INIT)
			{
				switch (ENTERING_TEXT) {
				case 0: {
					if (generateBoundaries_Button.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						generateBoundaries_Button.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							INIT = 0;
							BORDERS = 1;
							break;
						}
					}
					else generateBoundaries_Button.rectangle.setOutlineColor(Color(40, 40, 40));

					if (shapeControl_Button.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						if(!SHAPE_CONTROL)
							shapeControl_Button.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							SHAPE_CONTROL = !SHAPE_CONTROL;
							if (SHAPE_CONTROL)
								shapeControl_Button.rectangle.setFillColor(Color(200,0,0));
							else
								shapeControl_Button.rectangle.setFillColor(Color(100, 100, 100));
						}
					}
					else if(!SHAPE_CONTROL)
						shapeControl_Button.rectangle.setOutlineColor(Color(40, 40, 40));

					if (shapeControlProbability_TextField.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						shapeControlProbability_TextField.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							shapeControlProbability_TextField.text.setString("");
							probablityOfRule4 = 10;
							ENTERING_TEXT = 7;
						}
					}
					else shapeControlProbability_TextField.rectangle.setOutlineColor(Color(40, 40, 40));

					if (numberOfRandomGrains.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						numberOfRandomGrains.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							numberOfRandomGrains.text.setString("");
							initialNumberOfGrains = 100;
							ENTERING_TEXT = 1;
						}
					}
					else numberOfRandomGrains.rectangle.setOutlineColor(Color(40, 40, 40));

					if (losuj.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						losuj.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							INIT = 0;
							READY = 1;

							switch (grainDraw) {
							case 0: {
								for (int i = 0; i < initialNumberOfGrains; i++)
								{
									while (1) {
										x = rand() % (width - 2) + 1;
										y = rand() % (height - 2) + 1;
										if (board[x][y].alive == 0)
										{
											board[x][y].alive = 1;
											board[x][y].grainID = newID();
											board[x][y].setFillColor(newColor());
											break;
										}
									}
								}
								for (int i = 1; i < width - 1; i++)
									for (int j = 1; j < height - 1; j++)
										newBoard[i][j] = board[i][j];
								break;
							}
							case 1: {
								for (int x = 10; x < width - 1; x += 10)
									for (int y = 5; y < height - 1; y += 10)
									{
										if (board[x][y].alive == 0)
										{
											board[x][y].alive = 1;
											board[x][y].grainID = newID();
											board[x][y].setFillColor(newColor());
										}
									}
								grainDraw = 3;
								break;
							}
							case 2: {
								if (Vector2f(Mouse::getPosition(window)).y > boardCoordinate_y - 1 && Vector2f(Mouse::getPosition(window)).y < 1025 && Vector2f(Mouse::getPosition(window)).x > boardCoordinate_x - 1 && Vector2f(Mouse::getPosition(window)).x < 1806)
								{
									x = (Vector2f(Mouse::getPosition(window)).x - boardCoordinate_x) / 15;
									y = (Vector2f(Mouse::getPosition(window)).y - boardCoordinate_y) / 15;
									if (Mouse::isButtonPressed(Mouse::Left))
										if (board[x][y].alive == 0 && checkIfNewGrainCanBePlaced(board, x, y))
										{
											board[x][y].alive = 1;
											board[x][y].grainID = newID();
											board[x][y].setFillColor(newColor());
										}
										else
										{
											board[x][y].setFillColor(Color(0, 0, 0));
											board[x][y].alive = 0;
										}
								}
								break;
							}
							case 3: {
								if (Vector2f(Mouse::getPosition(window)).y > boardCoordinate_y - 1 && Vector2f(Mouse::getPosition(window)).y < 1025 && Vector2f(Mouse::getPosition(window)).x > boardCoordinate_x - 1 && Vector2f(Mouse::getPosition(window)).x < 1806)
								{
									x = (Vector2f(Mouse::getPosition(window)).x - boardCoordinate_x) / 15;
									y = (Vector2f(Mouse::getPosition(window)).y - boardCoordinate_y) / 15;
									if (Mouse::isButtonPressed(Mouse::Left))
										if (board[x][y].alive == 0)
										{
											board[x][y].alive = 1;
											board[x][y].grainID = newID();
											board[x][y].setFillColor(newColor());
										}
										else
										{
											board[x][y].setFillColor(Color(0, 0, 0));
											board[x][y].alive = 0;
										}
								}
								break;
							}
							default: {

							}
							}
						}
					}
					else losuj.rectangle.setOutlineColor(Color(40, 40, 40));

					if (import_bmp.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						import_bmp.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							readBMP(importFilename, board);
							for (int i = 1; i < width - 1; i++)
								for (int j = 1; j < height - 1; j++)
									newBoard[i][j] = board[i][j];
							Y_grainsNumber_TextField.text.setString(std::to_string(height));
							X_grainsNumber_TextField.text.setString(std::to_string(width));
							INIT = 0;
							READY = 1;
						}
					}
					else import_bmp.rectangle.setOutlineColor(Color(40, 40, 40));

					if (import_txt.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						import_txt.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							readTXT(importFilename+".txt", board);
							for (int i = 1; i < width - 1; i++)
								for (int j = 1; j < height - 1; j++)
									newBoard[i][j] = board[i][j];
							Y_grainsNumber_TextField.text.setString(std::to_string(height));
							X_grainsNumber_TextField.text.setString(std::to_string(width));
							INIT = 0;
							READY = 1;
						}
					}
					else import_txt.rectangle.setOutlineColor(Color(40, 40, 40));

					if (importFilename_TextField.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						importFilename_TextField.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							ENTERING_TEXT = 6;
						}
					}
					else Y_grainsNumber_TextField.rectangle.setOutlineColor(Color(40, 40, 40));

					if (Y_grainsNumber_TextField.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						Y_grainsNumber_TextField.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							Y_grainsNumber_TextField.text.setString("");
							height = 0;
							ENTERING_TEXT = 3;
						}
					}
					else Y_grainsNumber_TextField.rectangle.setOutlineColor(Color(40, 40, 40));

					if (X_grainsNumber_TextField.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						X_grainsNumber_TextField.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							X_grainsNumber_TextField.text.setString("");
							width = 0;
							ENTERING_TEXT = 2;
						}
					}
					else X_grainsNumber_TextField.rectangle.setOutlineColor(Color(40, 40, 40));

					if (inclusionsNumber_TextField.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						inclusionsNumber_TextField.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							inclusionsNumber_TextField.text.setString("");
							inclusionsNumber = 0;
							ENTERING_TEXT = 4;
						}
					}
					else inclusionsNumber_TextField.rectangle.setOutlineColor(Color(40, 40, 40));

					if (inclusionsSize_TextField.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						inclusionsSize_TextField.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							inclusionsSize_TextField.text.setString("");
							inclusionsSize = 0;
							ENTERING_TEXT = 5;
						}
					}
					else inclusionsSize_TextField.rectangle.setOutlineColor(Color(40, 40, 40));

					if (!circularInclusions && inclusionsTypeCircular_Button.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						inclusionsTypeCircular_Button.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							inclusionsTypeCircular_Button.rectangle.setFillColor(Color(200, 0, 0));
							inclusionsTypeSquare_Button.rectangle.setFillColor(Color(100, 100, 100));
							circularInclusions = 1;
						}
					}
					else if (!circularInclusions)
						inclusionsTypeCircular_Button.rectangle.setOutlineColor(Color(40, 40, 40));

					if (circularInclusions && inclusionsTypeSquare_Button.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						inclusionsTypeSquare_Button.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							inclusionsTypeCircular_Button.rectangle.setFillColor(Color(100, 100, 100));
							inclusionsTypeSquare_Button.rectangle.setFillColor(Color(200, 0, 0));
							circularInclusions = 0;
						}
					}
					else if (circularInclusions)
						inclusionsTypeSquare_Button.rectangle.setOutlineColor(Color(40, 40, 40));

					if (addInclusions_Button.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						addInclusions_Button.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							addInclusions(board, inclusionsNumber, inclusionsSize, circularInclusions);
							for (int i = 1; i < width - 1; i++)
								for (int j = 1; j < height - 1; j++)
									newBoard[i][j] = board[i][j];

						}
					}
					else addInclusions_Button.rectangle.setOutlineColor(Color(40, 40, 40));

					break;
				}
				case 1: {
					if (event.text.unicode >= 48 && event.text.unicode <= 57) {
						numberOfRandomGrains.text.setString(numberOfRandomGrains.text.getString() + (char)event.text.unicode);
					}
					else if (event.text.unicode == 13) {
						ENTERING_TEXT = 0;
						std::string n = numberOfRandomGrains.text.getString();
						initialNumberOfGrains = stoi(n);
					}
					break;
				}
				case 2: {
					if (event.text.unicode >= 48 && event.text.unicode <= 57) {
						X_grainsNumber_TextField.text.setString(X_grainsNumber_TextField.text.getString() + (char)event.text.unicode);
					}
					else if (event.text.unicode == 13) {
						ENTERING_TEXT = 0;
						std::string n = X_grainsNumber_TextField.text.getString();
						width = std::stoi(n);
					}
					break;
				}
				case 3: {
					if (event.text.unicode >= 48 && event.text.unicode <= 57) {
						Y_grainsNumber_TextField.text.setString(Y_grainsNumber_TextField.text.getString() + (char)event.text.unicode);
					}
					else if (event.text.unicode == 13) {
						ENTERING_TEXT = 0;
						std::string n = Y_grainsNumber_TextField.text.getString();
						height = std::stoi(n);
					}
					break;
				}
				case 4: {
					if (event.text.unicode >= 48 && event.text.unicode <= 57) {
						inclusionsNumber_TextField.text.setString(inclusionsNumber_TextField.text.getString() + (char)event.text.unicode);
					}
					else if (event.text.unicode == 13) {
						ENTERING_TEXT = 0;
						std::string n = inclusionsNumber_TextField.text.getString();
						inclusionsNumber = std::stoi(n);
					}
					break;
				}
				case 5: {
					if (event.text.unicode >= 48 && event.text.unicode <= 57) {
						inclusionsSize_TextField.text.setString(inclusionsSize_TextField.text.getString() + (char)event.text.unicode);
					}
					else if (event.text.unicode == 13) {
						ENTERING_TEXT = 0;
						std::string n = inclusionsSize_TextField.text.getString();
						inclusionsSize = std::stoi(n);
					}
					break;
				}
				case 6: {
					if (event.text.unicode == 59) {
						string s = importFilename_TextField.text.getString();
						string st = s.substr(0, s.size() - 1);
						importFilename_TextField.text.setString(st);
					}
					else if (event.text.unicode >= 92 && event.text.unicode <= 122) {
						importFilename_TextField.text.setString(importFilename_TextField.text.getString() + (char)event.text.unicode);
						cout << endl << (char)event.text.unicode << endl;
					}
					else if (event.text.unicode == 13) {
						ENTERING_TEXT = 0;
						importFilename = importFilename_TextField.text.getString();
					}
					break;
				}
				case 7: {
					if (event.text.unicode >= 48 && event.text.unicode <= 57) {
						shapeControlProbability_TextField.text.setString(shapeControlProbability_TextField.text.getString() + (char)event.text.unicode);
					}
					else if (event.text.unicode == 13) {
						ENTERING_TEXT = 0;
						std::string n = shapeControlProbability_TextField.text.getString();
						probablityOfRule4 = stoi(n);
					}
					break;
				}
				default: {
					//ERROR
				}
				}
			}
			if (READY)
			{
				if (restart.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
				{
					restart.rectangle.setOutlineColor(Color(200, 200, 200));
					if (Mouse::isButtonPressed(Mouse::Left))
					{
						READY = 0;
						RESTART = 1;
						break;
					}
				}
				else restart.rectangle.setOutlineColor(Color(40, 40, 40));

				if (start.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
				{
					start.rectangle.setOutlineColor(Color(200, 200, 200));
					if (Mouse::isButtonPressed(Mouse::Left))
					{
						READY = 0;
						START = 1;
					}
				}
				else start.rectangle.setOutlineColor(Color(100, 100, 100));
			}
			if (STOP) {
				switch (ENTERING_TEXT2) {
				case 0: {
					if (restart.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						restart.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							STOP = 0;
							RESTART = 1;
							break;
						}
					}
					else restart.rectangle.setOutlineColor(Color(40, 40, 40));

					if (start.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						start.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							START = 1;
							STOP = 0;
						}
					}
					else start.rectangle.setOutlineColor(Color(40, 40, 40));

					if (save_bmp.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						save_bmp.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							drawbmp(saveFilename, board, width, height);
							STOP = 0;
							READY = 1;
						}
					}
					else save_bmp.rectangle.setOutlineColor(Color(40, 40, 40));

					if (saveFilename_TextField.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						saveFilename_TextField.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							ENTERING_TEXT2 = 3;
						}
					}
					else saveFilename_TextField.rectangle.setOutlineColor(Color(40, 40, 40));

					if (save_txt.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						save_txt.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							saveAsTxt((saveFilename + ".txt"), board);
							STOP = 0;
							READY = 1;
						}
					}
					else save_txt.rectangle.setOutlineColor(Color(40, 40, 40));

					if (inclusionsNumber_TextField.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						inclusionsNumber_TextField.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							inclusionsNumber_TextField.text.setString("");
							inclusionsNumber = 0;
							ENTERING_TEXT2 = 1;
						}
					}
					else inclusionsNumber_TextField.rectangle.setOutlineColor(Color(40, 40, 40));

					if (inclusionsSize_TextField.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						inclusionsSize_TextField.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							inclusionsSize_TextField.text.setString("");
							inclusionsSize = 0;
							ENTERING_TEXT2 = 2;
						}
					}
					else inclusionsSize_TextField.rectangle.setOutlineColor(Color(40, 40, 40));

					if (!circularInclusions && inclusionsTypeCircular_Button.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						inclusionsTypeCircular_Button.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							inclusionsTypeCircular_Button.rectangle.setOutlineColor(Color(100, 0, 0));
							inclusionsTypeSquare_Button.rectangle.setOutlineColor(Color(40, 40, 40));
							circularInclusions = 1;
						}
					}
					else if (!circularInclusions)
						inclusionsTypeCircular_Button.rectangle.setOutlineColor(Color(40, 40, 40));

					if (circularInclusions && inclusionsTypeSquare_Button.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						inclusionsTypeSquare_Button.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							inclusionsTypeCircular_Button.rectangle.setOutlineColor(Color(40, 40, 40));
							inclusionsTypeSquare_Button.rectangle.setOutlineColor(Color(100, 0, 0));
							circularInclusions = 0;
						}
					}
					else if (circularInclusions)
						inclusionsTypeSquare_Button.rectangle.setOutlineColor(Color(40, 40, 40));

					if (addInclusions_Button.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
					{
						addInclusions_Button.rectangle.setOutlineColor(Color(200, 200, 200));
						if (Mouse::isButtonPressed(Mouse::Left))
						{
							addInclusionsAfterSimulation(board, inclusionsNumber, inclusionsSize, circularInclusions);
							for (int i = 1; i < width - 1; i++)
								for (int j = 1; j < height - 1; j++)
									newBoard[i][j] = board[i][j];

						}
					}
					else addInclusions_Button.rectangle.setOutlineColor(Color(40, 40, 40));
				}
				case 1: {
					if (event.text.unicode >= 48 && event.text.unicode <= 57) {
						inclusionsNumber_TextField.text.setString(inclusionsNumber_TextField.text.getString() + (char)event.text.unicode);
					}
					else if (event.text.unicode == 13) {
						ENTERING_TEXT2 = 0;
						std::string n = inclusionsNumber_TextField.text.getString();
						inclusionsNumber = std::stoi(n);
					}
					break;
				}
				case 2: {
					if (event.text.unicode >= 48 && event.text.unicode <= 57) {
						inclusionsSize_TextField.text.setString(inclusionsSize_TextField.text.getString() + (char)event.text.unicode);
					}
					else if (event.text.unicode == 13) {
						ENTERING_TEXT2 = 0;
						std::string n = inclusionsSize_TextField.text.getString();
						inclusionsSize = std::stoi(n);
					}
					break;
				}
				case 3: {
					if (event.text.unicode == 59) {
						string s = saveFilename_TextField.text.getString();
						string st = s.substr(0, s.size() - 1);
						saveFilename_TextField.text.setString(st);
					}
					else if (event.text.unicode >= 92 && event.text.unicode <= 122) {
						saveFilename_TextField.text.setString(saveFilename_TextField.text.getString() + (char)event.text.unicode);
					}
					else if (event.text.unicode == 13) {
						ENTERING_TEXT2 = 0;
						saveFilename = saveFilename_TextField.text.getString();
					}
					break;
				}
				default: {
					//ERROR
				}
				}
			}
			if (SIMULATION_FINISHED) {
				if (restart.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
				{
					restart.rectangle.setOutlineColor(Color(200, 200, 200));
					if (Mouse::isButtonPressed(Mouse::Left))
					{
						SIMULATION_FINISHED = 0;
						RESTART = 1;
						break;
					}
				}
				else restart.rectangle.setOutlineColor(Color(40, 40, 40));
				
				if (substructure_Button.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
				{
					substructure_Button.rectangle.setOutlineColor(Color(200, 200, 200));
					if (Mouse::isButtonPressed(Mouse::Left))
					{
						numberOfGrains = 1; 
						for (int i = 1; i < width - 1; i++)
							for (int j = 1; j < height - 1; j++)
								if (!isChoosen(board[i][j].grainID)) {
									newBoard[i][j].alive = 0;
									newBoard[i][j].grainID = 0;
									newBoard[i][j].setFillColor(Color(0, 0, 0));
								}
								else {
									newBoard[i][j].alive = 1;
									newBoard[i][j].grainID = 1;
									newBoard[i][j].setFillColor(board[i][j].getFillColor());
								}
						for (int i = 1; i < width - 1; i++)
							for (int j = 1; j < height - 1; j++)
								board[i][j] = newBoard[i][j];

						INIT = 1;
						SIMULATION_FINISHED = 0;
						break;
					}
				}
				else substructure_Button.rectangle.setOutlineColor(Color(40, 40, 40));
				
				if (dualPhase_Button.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
				{
					dualPhase_Button.rectangle.setOutlineColor(Color(200, 200, 200));
					if (Mouse::isButtonPressed(Mouse::Left))
					{
						numberOfGrains = 1;
						for (int i = 1; i < width - 1; i++)
							for (int j = 1; j < height - 1; j++)
								if (!isChoosen(board[i][j].grainID)) {
									newBoard[i][j].alive = 0;
									newBoard[i][j].grainID = 0;
									newBoard[i][j].setFillColor(Color(0, 0, 0));
								}
								else {
									newBoard[i][j].alive = 1;
									newBoard[i][j].grainID = 1;
									newBoard[i][j].setFillColor(Color(255,255,255));
								}
						for (int i = 1; i < width - 1; i++)
							for (int j = 1; j < height - 1; j++)
								board[i][j] = newBoard[i][j];

						INIT = 1;
						SIMULATION_FINISHED = 0;
						break;
					}
				}
				else dualPhase_Button.rectangle.setOutlineColor(Color(40, 40, 40));

				if (generateBoundaries_Button.rectangle.getGlobalBounds().contains(Vector2f(Mouse::getPosition(window))))
				{
					generateBoundaries_Button.rectangle.setOutlineColor(Color(200, 200, 200));
					if (Mouse::isButtonPressed(Mouse::Left))
					{
						SIMULATION_FINISHED = 0;
						BORDERS = 1;
						break;
					}
				}
				else generateBoundaries_Button.rectangle.setOutlineColor(Color(40, 40, 40));

				if (Vector2f(Mouse::getPosition(window)).y > boardCoordinate_y - 1 && Vector2f(Mouse::getPosition(window)).y < (cellSize*height + boardCoordinate_y) && Vector2f(Mouse::getPosition(window)).x > boardCoordinate_x - 1 && Vector2f(Mouse::getPosition(window)).x < (cellSize*width+boardCoordinate_x))
				{
					x = (Vector2f(Mouse::getPosition(window)).x - boardCoordinate_x) / cellSize;
					y = (Vector2f(Mouse::getPosition(window)).y - boardCoordinate_y) / cellSize;
					if (Mouse::isButtonPressed(Mouse::Left))
						if (board[x][y].grainID > 1) // && checkIfNewGrainCanBePlaced(board, x, y))
						{
							bool alreadyChoosen = 0;
							if (choosen.size() > 0) {
								for (int z = 0; z < choosen.size(); z++)
									if (choosen[z] == board[x][y].grainID) {
										for (int i = 1; i < width - 1; i++)
											for (int j = 1; j < height - 1; j++)
												if (board[i][j].grainID == choosen[z])
													board[i][j].setOutlineThickness(0);
										choosen.erase(choosen.begin() + z);
										alreadyChoosen = 1;
										break;
									}
							}

							if (!alreadyChoosen) {
								choosen.push_back(board[x][y].grainID);
								for (int i = 1; i < width - 1; i++)
									for (int j = 1; j < height - 1; j++)
										if (board[i][j].grainID == board[x][y].grainID)
											board[i][j].setOutlineThickness(1);
							}
						}
				}
			}
			if (CA_SUBSTRUCTURE) {
				for (int i = 1; i < width - 1; i++)
					for (int j = 1; j < height - 1; j++)
						if (!isChoosen(board[i][j].grainID)) {
							newBoard[i][j].alive = 0;
							newBoard[i][j].grainID = 0;
							newBoard[i][j].setFillColor(Color(0, 0, 0));
						}
						else {
							newBoard[i][j].alive = 1;
							newBoard[i][j].grainID = board[i][j].grainID;
							newBoard[i][j].setFillColor(board[i][j].getFillColor());
						}
				for (int i = 1; i < width - 1; i++)
					for (int j = 1; j < height - 1; j++)
						board[i][j] = newBoard[i][j];

				CA_SUBSTRUCTURE = 0;
				INIT = 1;
			}
			if (BORDERS) {
				if (boundarySize == 1) {
					for (int i = 1; i < width - 1; i++)
						for (int j = 1; j < height - 1; j++)
							for (int a = -1; a < 2; a++)
								for (int b = 0; b < 2; b++)
									if (board[i][j].grainID > 0 && board[i][j].grainID != board[i + a][j + b].grainID) {
										newBoard[i][j].grainID = -1;
										newBoard[i][j].setFillColor(Color(0, 0, 0));
										break;
									}
									else newBoard[i][j] = board[i][j];
				}
				else {

				}

				for (int i = 1; i < width - 1; i++)
					for (int j = 1; j < height - 1; j++)
						board[i][j] = newBoard[i][j];
				BORDERS = 0;
				INIT = 1;
			}
			if (RESTART) {
				boardCreation(board);
				boardCreation(newBoard);
				RESTART = 0;
				INIT = 1;
			}

			if (gameStop)
			{
				t += 0.001;
				ro1 = C + (1 - C)*pow(e, (-B*t));
				double deltaRo = abs(ro1 - ro0);
				ro0 = ro1;
				double roKom = deltaRo / (height*width);
				unsigned int borderCellCounter = 0;
				std::cout << roKom << std::endl;
				for (auto i = 2; i < width - 2; i++)
					for (auto j = 2; j < height - 2; j++)
					{
						if (isOnGrainBorder(board, i, j))
						{
							board[i][j].ro += 0.8*roKom;

							deltaRo -= 0.8*roKom;
							borderCellCounter++;
							if (board[i][j].ro > roCritical)
							{
								start.rectangle.setFillColor(Color(rand() % 256, rand() % 256, rand() % 256));
								board[i][j].ro = 0;
								board[i][j].grainID = newIDRegenerated();
								board[i][j].setFillColor(newColor());
								board[i + 1][j].ro = 0;
								board[i + 1][j].grainID = board[i][j].grainID;
								board[i + 1][j].setFillColor(board[i][j].getFillColor());
								board[i - 1][j].ro = 0;
								board[i - 1][j].grainID = board[i][j].grainID;
								board[i - 1][j].setFillColor(board[i][j].getFillColor());
								board[i + 1][j + 1].ro = 0;
								board[i + 1][j + 1].grainID = board[i][j].grainID;
								board[i + 1][j + 1].setFillColor(board[i][j].getFillColor());
								board[i + 1][j - 1].ro = 0;
								board[i + 1][j - 1].grainID = board[i][j].grainID;
								board[i + 1][j - 1].setFillColor(board[i][j].getFillColor());
								board[i - 1][j + 1].ro = 0;
								board[i - 1][j + 1].grainID = board[i][j].grainID;
								board[i - 1][j + 1].setFillColor(board[i][j].getFillColor());
								board[i - 1][j - 1].ro = 0;
								board[i - 1][j - 1].grainID = board[i][j].grainID;
								board[i - 1][j - 1].setFillColor(board[i][j].getFillColor());
								board[i][j + 1].ro = 0;
								board[i][j + 1].grainID = board[i][j].grainID;
								board[i][j + 1].setFillColor(board[i][j].getFillColor());
								board[i][j - 1].ro = 0;
								board[i][j - 1].grainID = board[i][j].grainID;
								board[i][j - 1].setFillColor(board[i][j].getFillColor());
							}
						}
						else
						{
							board[i][j].ro += 0.2*roKom;
							deltaRo -= 0.2*roKom;
							if (board[i][j].ro > roCritical)
							{
								board[i][j].ro = 0;
								board[i][j].grainID = newIDRegenerated();
								board[i][j].setFillColor(newColor());
								board[i + 1][j].ro = 0;
								board[i + 1][j].grainID = board[i][j].grainID;
								board[i + 1][j].setFillColor(board[i][j].getFillColor());
								board[i - 1][j].ro = 0;
								board[i - 1][j].grainID = board[i][j].grainID;
								board[i - 1][j].setFillColor(board[i][j].getFillColor());
								board[i + 1][j + 1].ro = 0;
								board[i + 1][j + 1].grainID = board[i][j].grainID;
								board[i + 1][j + 1].setFillColor(board[i][j].getFillColor());
								board[i + 1][j - 1].ro = 0;
								board[i + 1][j - 1].grainID = board[i][j].grainID;
								board[i + 1][j - 1].setFillColor(board[i][j].getFillColor());
								board[i - 1][j + 1].ro = 0;
								board[i - 1][j + 1].grainID = board[i][j].grainID;
								board[i - 1][j + 1].setFillColor(board[i][j].getFillColor());
								board[i - 1][j - 1].ro = 0;
								board[i - 1][j - 1].grainID = board[i][j].grainID;
								board[i - 1][j - 1].setFillColor(board[i][j].getFillColor());
								board[i][j + 1].ro = 0;
								board[i][j + 1].grainID = board[i][j].grainID;
								board[i][j + 1].setFillColor(board[i][j].getFillColor());
								board[i][j - 1].ro = 0;
								board[i][j - 1].grainID = board[i][j].grainID;
								board[i][j - 1].setFillColor(board[i][j].getFillColor());
							}
						}
					}

				roKom = deltaRo / 2;
				sleep(milliseconds(10));
				for (auto a = 0; a < 2;)
				{
					int i = rand() % (width - 4) + 2;
					int j = rand() % (height - 4) + 2;
					if (isOnGrainBorder(board, i, j))
					{
						/*gameStart = 1;
						gameStop = 0;*/
						board[i][j].ro += roKom;
						a++;
						if (board[i][j].ro > roCritical)
						{
							newBoard[i][j].ro = 0;
							newBoard[i][j].alive = 1;
							newBoard[i][j].grainID = newIDRegenerated();
							newBoard[i][j].setFillColor(newColor());
							newBoard[i + 1][j].ro = 0;
							newBoard[i + 1][j].alive = 1;
							newBoard[i + 1][j].grainID = newBoard[i][j].grainID;
							newBoard[i + 1][j].setFillColor(newBoard[i][j].getFillColor());
							newBoard[i - 1][j].ro = 0;
							newBoard[i - 1][j].alive = 1;
							newBoard[i - 1][j].grainID = newBoard[i][j].grainID;
							newBoard[i - 1][j].setFillColor(newBoard[i][j].getFillColor());
							newBoard[i + 1][j + 1].ro = 0;
							newBoard[i + 1][j + 1].alive = 1;
							newBoard[i + 1][j + 1].grainID = newBoard[i][j].grainID;
							newBoard[i + 1][j + 1].setFillColor(newBoard[i][j].getFillColor());
							newBoard[i + 1][j - 1].ro = 0;
							newBoard[i + 1][j - 1].alive = 1;
							newBoard[i + 1][j - 1].grainID = newBoard[i][j].grainID;
							newBoard[i + 1][j - 1].setFillColor(newBoard[i][j].getFillColor());
							newBoard[i - 1][j + 1].ro = 0;
							newBoard[i - 1][j + 1].alive = 1;
							newBoard[i - 1][j + 1].grainID = newBoard[i][j].grainID;
							newBoard[i - 1][j + 1].setFillColor(newBoard[i][j].getFillColor());
							newBoard[i - 1][j - 1].ro = 0;
							newBoard[i - 1][j - 1].alive = 1;
							newBoard[i - 1][j - 1].grainID = newBoard[i][j].grainID;
							newBoard[i - 1][j - 1].setFillColor(newBoard[i][j].getFillColor());
							newBoard[i][j + 1].ro = 0;
							newBoard[i][j + 1].alive = 1;
							newBoard[i][j + 1].grainID = newBoard[i][j].grainID;
							newBoard[i][j + 1].setFillColor(newBoard[i][j].getFillColor());
							newBoard[i][j - 1].ro = 0;
							newBoard[i][j - 1].alive = 1;
							newBoard[i][j - 1].grainID = newBoard[i][j].grainID;
							newBoard[i][j - 1].setFillColor(newBoard[i][j].getFillColor());
						}
					}
				}


				for (int i = 1; i < width - 1; i++)
					for (int j = 1; j < height - 1; j++)
						board[i][j] = newBoard[i][j];
			}

			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
				window.close();
		}

		if (monteCarlo)
		{
			vector< pair <int, int> > cells;
			for (int i = 1; i < width - 1; i++)
				for (int j = 1; j < height - 1; j++)
					


			int i, j;
			for (auto c = 2; c < 500; c++)
				for (auto d = 2; d < 500; d++)
				{

					i = rand() % (width - 2) + 1;
					j = rand() % (height - 2) + 1;
					if (board[i][j].draw == 0)
					{
						board[i][j].draw = 1;
					}
					else
					{
						continue;
					}
					int counter = 0;
					int a, b;
					if (board[i + 1][j].grainID != board[i][j].grainID)
						counter++;
					if (board[i - 1][j].grainID != board[i][j].grainID)
						counter++;
					if (board[i + 1][j + 1].grainID != board[i][j].grainID)
						counter++;
					if (board[i + 1][j - 1].grainID != board[i][j].grainID)
						counter++;
					if (board[i - 1][j + 1].grainID != board[i][j].grainID)
						counter++;
					if (board[i - 1][j - 1].grainID != board[i][j].grainID)
						counter++;
					if (board[i][j + 1].grainID != board[i][j].grainID)
						counter++;
					if (board[i][j - 1].grainID != board[i][j].grainID)
						counter++;

					if (counter == 0)
						continue;

					int id = board[i][j].grainID;
					int z = 0;
					while (id == board[i][j].grainID && z < 50)
					{
						z++;
						a = rand() % 3 - 1;
						b = rand() % 3 - 1;
						if (board[i + a][j + b].grainID == 0)
						{
							z = 50;
							break;
						}
						id = board[i + a][j + b].grainID;


					}
					if (z == 50)
						continue;

					int counter2 = 0;
					if (board[i + 1][j].grainID != id)
						counter2++;
					if (board[i - 1][j].grainID != id)
						counter2++;
					if (board[i + 1][j + 1].grainID != id)
						counter2++;
					if (board[i + 1][j - 1].grainID != id)
						counter2++;
					if (board[i - 1][j + 1].grainID != id)
						counter2++;
					if (board[i - 1][j - 1].grainID != id)
						counter2++;
					if (board[i][j + 1].grainID != id)
						counter2++;
					if (board[i][j - 1].grainID != id)
						counter2++;

					if (counter2 <= counter)
					{
						//std::cout << counter2 << std::endl;
						board[i][j].grainID = board[i + a][j + b].grainID;
						board[i][j].setFillColor(board[i + a][j + b].getFillColor());

						//newBoard[i][j] = board[i + a][j + b];
						//board[i][j] = newBoard[i][j];
					}

				}
		}

		for (int i = 1; i < width - 1; i++)
			for (int j = 1; j < height - 1; j++)
				window.draw(board[i][j]);

		window.draw(menuFrame);
		window.draw(start.rectangle);
		window.draw(start.text);
		window.draw(restart.rectangle);
		window.draw(restart.text);
		//window.draw(title1);
		//window.draw(title2.rectangle);
		//window.draw(title2.text);
		//window.draw(title3.rectangle);
		/*
		window.draw(title3.text);
		//window.draw(title4.rectangle);
		window.draw(title4.text);
		//window.draw(title5.rectangle);
		window.draw(title5.text);
		//window.draw(title6.rectangle);
		window.draw(title6.text);
		//window.draw(title7.rectangle);
		window.draw(title7.text);
		*/
		//window.draw(title8.text);
		//window.draw(title9.text);
		//window.draw(title10);
		//window.draw(title11);
		window.draw(title12.text);
		/*window.draw(title13.text);
		window.draw(title14.text);
		window.draw(title15.text); */
		window.draw(title18);
		window.draw(Y_grainsNumber_TextField.rectangle);
		window.draw(Y_grainsNumber_TextField.text);
		window.draw(X_grainsNumber_TextField.rectangle);
		window.draw(X_grainsNumber_TextField.text);
		window.draw(numberOfRandomGrains.rectangle);
		window.draw(numberOfRandomGrains.text);
		window.draw(losuj.rectangle);
		window.draw(losuj.text);
		window.draw(menuText_X);
		window.draw(menuText_Y);
		window.draw(inclusionsNumber_Text);
		window.draw(inclusionsSize_Text);
		window.draw(inclusionsType_Text);
		window.draw(inclusionsNumber_TextField.rectangle);
		window.draw(inclusionsNumber_TextField.text);
		window.draw(inclusionsSize_TextField.rectangle);
		window.draw(inclusionsSize_TextField.text);
		window.draw(inclusionsTypeCircular_Button.rectangle);
		window.draw(inclusionsTypeCircular_Button.text);
		window.draw(inclusionsTypeSquare_Button.rectangle);
		window.draw(inclusionsTypeSquare_Button.text);
		window.draw(addInclusions_Button.rectangle);
		window.draw(addInclusions_Button.text);
		window.draw(shapeControl_TextField.text);
		window.draw(shapeControl_Button.rectangle);
		window.draw(shapeControl_Button.text);
		window.draw(shapeControlProbability_TextField.rectangle);
		window.draw(shapeControlProbability_TextField.text);
		window.draw(percent_Text);
		window.draw(selectGrains_Button.rectangle);
		window.draw(selectGrains_Button.text);
		window.draw(substructure_Button.rectangle);
		window.draw(substructure_Button.text);
		window.draw(dualPhase_Button.rectangle);
		window.draw(dualPhase_Button.text);
		window.draw(boundarySize_Text);
		window.draw(boundarySize_TextField.rectangle);
		window.draw(boundarySize_TextField.text);
		window.draw(generateBoundaries_Button.rectangle);
		window.draw(generateBoundaries_Button.text);
		window.draw(saveTo_Text);
		window.draw(save_bmp.rectangle);
		window.draw(save_bmp.text);
		window.draw(save_txt.rectangle);
		window.draw(save_txt.text);
		window.draw(savePath_Text);
		window.draw(saveFilename_TextField.rectangle);
		window.draw(saveFilename_TextField.text);
		window.draw(importFrom_Text);
		window.draw(import_bmp.rectangle);
		window.draw(import_bmp.text);
		window.draw(import_txt.rectangle);
		window.draw(import_txt.text);
		window.draw(importPath_Text);
		window.draw(importFilename_TextField.rectangle);
		window.draw(importFilename_TextField.text);

		window.display();
	}


	return false;
}
