
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <algorithm>
#include "glut.h"
#include <vector>
//////////////ours

const int W = 600;
const int H = 600;

const int TH = 256; // texture size. must be power of 2
const int TW = 256;

// matrix that stores a texture
unsigned char tx0[TH][TW][3] = { 0 };

double floorsAmount = 3.0;
double numWindows = 2.0;
double numColor = 1.0;

/// roof color ligher
double roofRlighter = 1.0;
double roofGlighter = 0.4;
double roofBlighter = 0.4;

/// roof color darker
double roofRDarker = 0.6;
double roofGDarker = 0.0;
double roofBDarker = 0.0;

const double PI = 3.14156;

const int GSZ = 100;

double ground[GSZ][GSZ] = { 0 };
double ground2[GSZ][GSZ] = { 0 }; /// river water

double tmp[GSZ][GSZ] = { 0 };
double offset = 0;

typedef struct Ground
{
	int row, col;
} Ground;

bool visited[GSZ][GSZ] = { false };

typedef struct
{
	double x, y, z;
} POINT3;

Ground cityLoc;
POINT3 eye = {1,24,21};
// ego-motion (loco-motion)
double speed = 0;
double angular_speed = 0;
double sight_angle = PI;
POINT3 direction = {sin(sight_angle),-0.3,cos(sight_angle)};
// aircraft motion
double ac_speed = 0;
double ac_angular_speed = 0;
double yaw = PI;
double pitch = 0.0;
POINT3 ac_direction = { sin(yaw),0,cos(yaw) };
POINT3 aircraft = { 0,15,6 };

bool terrainIsForming = true;
bool isCaptured = false;
bool isRain = false;
bool canBuildTown = false;
bool foundLocation = false;

void UpdateTerrain1();
void UpdateTerrain2();
void UpdateTerrain3();

void Smooth();
void rain();
void findTowenLoc();
bool searchTownLoc(int x, int z);

void DrawWall(int kind);
void DrawRoof();
void DrawCylinderWindows(int n);
void DrawCylinderNoWindows(int n);
void DrawInsideBuilding(int n);
void DrawHouse();
void DrawTown();
void DrawRoad();
void drawOneRoad(Ground g1, Ground g2, int texType, double roadHigh);


void DrawWall(int kind)
{
	// left part
	if (kind == 1) 	glColor3d(0.8, 0.7, 0.5);
	else if (kind == 2) glColor3d(0.7, 0.6, 0.4);
	glBegin(GL_POLYGON);
	glVertex3d(-5, 0, 0);
	glVertex3d(-5, 10, 0);
	glVertex3d(-2, 10, 0);
	glVertex3d(-2, 0, 0);
	glEnd();

	// mid part
	glBegin(GL_POLYGON);
	glVertex3d(-2, 0, 0);
	glVertex3d(-2, 3, 0);
	glVertex3d(2, 3, 0);
	glVertex3d(2, 0, 0);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3d(-2, 7, 0);
	glVertex3d(-2, 10, 0);
	glVertex3d(2, 10, 0);
	glVertex3d(2, 7, 0);
	glEnd();

	// right part
	glBegin(GL_POLYGON);
	glVertex3d(2, 0, 0);
	glVertex3d(2, 10, 0);
	glVertex3d(5, 10, 0);
	glVertex3d(5, 0, 0);
	glEnd();

}

void DrawRoof()
{
	glColor3d(roofRDarker, roofGDarker, roofBDarker);
	glBegin(GL_POLYGON);
	glVertex3d(-1, 0, 0);
	glVertex3d(0, 3, 0);
	glVertex3d(0, 0, 1);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3d(1, 0, 0);
	glVertex3d(0, 3, 0);
	glVertex3d(0, 0, -1);
	glEnd();


	glColor3d(roofRlighter, roofGlighter, roofBlighter);
	glBegin(GL_POLYGON);
	glVertex3d(-1, 0, 0);
	glVertex3d(0, 3, 0);
	glVertex3d(0, 0, -1);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3d(1, 0, 0);
	glVertex3d(0, 3, 0);
	glVertex3d(0, 0, 1);
	glEnd();


}
void DrawCylinderWindows(int n)
{
	double alpha, teta = 2 * PI / n;
	int numParts = 2 * numWindows + 1;
	for (alpha = 0;alpha < 2 * PI;alpha += teta)
	{
		////shadow colors
		double shadowFactor = (1 - cos(alpha)) / 2;
		double wallColorR = 0.96 - 0.2 * shadowFactor;
		double wallColorG = 0.76 - 0.2 * shadowFactor;
		double wallColorB = 0.56 - 0.2 * shadowFactor;

		double sinVector = sin(alpha + teta) - sin(alpha);
		double cosVector = cos(alpha + teta) - cos(alpha);
		double sinWidth = (sinVector / numParts);
		double cosWidth = (cosVector / numParts);
		for (int j = 0; j <= numParts; j += 2) {

			glBegin(GL_POLYGON); // upper points are darker
			glColor3d(wallColorR, wallColorG, wallColorB);
			glVertex3d(sin(alpha) + sinWidth * double(j), 1, cos(alpha) + cosWidth * (double)(j)); // point 1

			glColor3d(wallColorR, wallColorG, wallColorB);
			glVertex3d(sin(alpha) + sinWidth * double(j + 1), 1, cos(alpha) + cosWidth * (double)(j + 1)); // point 2
			// lower points are brighter
			glColor3d(wallColorR, wallColorG, wallColorB);
			glVertex3d(sin(alpha) + sinWidth * double(j + 1), 0, cos(alpha) + cosWidth * (double)(j + 1)); // point 3

			glColor3d(wallColorR, wallColorG, wallColorB);
			glVertex3d(sin(alpha) + sinWidth * double(j), 0, cos(alpha) + cosWidth * (double)(j)); //point 4
			glEnd();
		}
	}
}
void DrawCylinderNoWindows(int n)
{
	double alpha, teta = 2 * PI / n;

	for (alpha = 0;alpha < 2 * PI;alpha += teta)
	{
		////shadow colors
		double shadowFactor = (1 - cos(alpha)) / 2;
		double wallColorR = 0.96 - 0.2 * shadowFactor;
		double wallColorG = 0.76 - 0.2 * shadowFactor;
		double wallColorB = 0.56 - 0.2 * shadowFactor;

		glBegin(GL_POLYGON); // upper points are darker
		glColor3d(wallColorR, wallColorG, wallColorB);
		glVertex3d(sin(alpha), 1, cos(alpha)); // point 1

		glColor3d(wallColorR, wallColorG, wallColorB);
		glVertex3d(sin(alpha + teta), 1, cos(alpha + teta)); // point 2
		// lower points are brighter
		glColor3d(wallColorR, wallColorG, wallColorB);
		glVertex3d(sin(alpha + teta), 0, cos(alpha + teta)); // point 3

		glColor3d(wallColorR, wallColorG, wallColorB);
		glVertex3d(sin(alpha), 0, cos(alpha)); //point 4
		glEnd();
	}
}

void DrawInsideBuilding(int n) {
	double alpha, teta = 2 * PI / n;

	for (alpha = 0;alpha < 2 * PI;alpha += teta)
	{

		glBegin(GL_POLYGON); // upper points are darker
		glColor3d(0, 0, 0.2);
		glVertex3d(sin(alpha), 1, cos(alpha)); // point 1

		glColor3d(0, 0, 0.2);
		glVertex3d(sin(alpha + teta), 1, cos(alpha + teta)); // point 2
		// lower points are brighter
		glColor3d(0, 0, 0.2);
		glVertex3d(sin(alpha + teta), 0, cos(alpha + teta)); // point 3

		glColor3d(0, 0, 0.2);
		glVertex3d(sin(alpha), 0, cos(alpha)); //point 4
		glEnd();
	}
}

void DrawHouse()
{
	int i;
	for (i = 0; i < 2 * floorsAmount; i += 2) {
		glPushMatrix();
		glTranslated(0, i, 0);
		DrawCylinderNoWindows(4);
		glPopMatrix();

		glPushMatrix();
		glTranslated(0, i + 1, 0);
		DrawCylinderWindows(4);
		glPopMatrix();
	}
	glPushMatrix();
	glTranslated(0, i, 0);
	DrawCylinderNoWindows(4);
	glPopMatrix();

	glPushMatrix();
	glScaled(0.95, 2 * floorsAmount + 0.5, 0.95);
	DrawInsideBuilding(4);
	glPopMatrix();

	glPushMatrix();
	glTranslated(0, 2 * floorsAmount + 1, 0);
	DrawRoof();
	glPopMatrix();

}

void DrawTown() {

	int x = cityLoc.row;
	int z = cityLoc.col;
	glPushMatrix();
	glTranslated(z - GSZ / 2, ground[x][z], x - GSZ / 2);
	glScaled(0.8, 0.4, 0.8);
	DrawHouse();
	glPopMatrix();

	if (ground[x - 5][z] != NULL && ground[x - 5][z] > 0.2 && ground[x - 5][z] > ground2[x - 5][z] - 1) {
		glPushMatrix();
		glTranslated(z - GSZ / 2, ground[x-5][z], x - 5 - GSZ / 2);
		glScaled(0.7, 0.3, 0.7);
		DrawHouse();
		glPopMatrix();
	}

	if (ground[x][z - 5] != NULL && ground[x][z - 5] > 0.2 && ground[x][z - 5] > ground2[x][z - 5] - 1) {
		glPushMatrix();
		glTranslated(z - 5 - GSZ / 2, ground[x][z - 5], x - GSZ / 2);
		glScaled(0.7, 0.3, 0.7);
		DrawHouse();
		glPopMatrix();
	}

	if (ground[x + 5][z] != NULL && ground[x + 5][z] > 0.2 && ground[x + 5][z] > ground2[x + 5][z] - 1) {
		glPushMatrix();
		glTranslated(z - GSZ / 2, ground[x + 5][z], x + 5 - GSZ / 2);
		glScaled(0.7, 0.3, 0.7);
		DrawHouse();
		glPopMatrix();
	}

	if (ground[x][z + 5] != NULL && ground[x][z + 5] > 0.2 && ground[x][z + 5] > ground2[x][z + 5] - 1) {
		glPushMatrix();
		glTranslated(z + 5 - GSZ / 2, ground[x][z + 5], x - GSZ / 2);
		glScaled(0.7, 0.3, 0.7);
		DrawHouse();
		glPopMatrix();
	}

	if (ground[x + 5][z + 5] != NULL && ground[x + 5][z + 5] > 0.2 && ground[x + 5][z + 5] > ground2[x + 5][z + 5] - 1) {
		glPushMatrix();
		glTranslated(z + 5 - GSZ / 2, ground[x + 5][z + 5], x + 5 - GSZ / 2);
		glScaled(0.7, 0.3, 0.7);
		DrawHouse();
		glPopMatrix();
	}

	if (ground[x - 5][z + 5] != NULL && ground[x - 5][z + 5] > 0.2 && ground[x - 5][z + 5] > ground2[x - 5][z + 5] - 1) {
		glPushMatrix();
		glTranslated(z + 5 - GSZ / 2, ground[x - 5][z + 5], x - 5 - GSZ / 2);
		glScaled(0.7, 0.3, 0.7);
		DrawHouse();
		glPopMatrix();
	}

	if (ground[x + 5][z - 5] != NULL && ground[x + 5][z - 5] > 0.2 && ground[x + 5][z - 5] > ground2[x + 5][z - 5] - 1) {
		glPushMatrix();
		glTranslated(z - 5 - GSZ / 2, ground[x + 5][z - 5], x + 5 - GSZ / 2);
		glScaled(0.7, 0.3, 0.7);
		DrawHouse();
		glPopMatrix();
	}

	if (ground[x - 5][z - 5] != NULL && ground[x - 5][z - 5] > 0.2 && ground[x - 5][z - 5] > ground2[x - 5][z - 5] - 1) {
		glPushMatrix();
		glTranslated(z - 5 - GSZ / 2, ground[x - 5][z - 5], x - 5 - GSZ / 2);
		glScaled(0.7, 0.3, 0.7);
		DrawHouse();
		glPopMatrix();
	}
}


void findTowenLoc()
{
	int x, z;

	srand(time(0));

	// Loop until a valid location is found
	do {
		x = rand() % GSZ;
		z = rand() % GSZ;
	} while (x < 10 || z < 10 || ground[x][z] < 0);

	// Search around the randomly selected location
	foundLocation = searchTownLoc(x, z);

	if (!foundLocation) {
		std::cerr << "No suitable location found to build the town." << std::endl; 
		canBuildTown = false;
	}
	// Place the house at the found location
	//if (foundLocation) {
	//	glPushMatrix();
	//	glTranslated(cityLoc.row - GSZ / 2, ground[cityLoc.row][cityLoc.col], cityLoc.col - GSZ / 2);
	//	glScaled(10, 10, 10);
	//	DrawHouse();
	//	glPopMatrix();
	//}
	//else {
	//	std::cerr << "No suitable location found to build the town." << std::endl;
	//	// Optionally set canBuildTown = false to avoid repeated attempts
	//	canBuildTown = false;
	//}
}


bool searchTownLoc(int x, int z) {
	visited[x][z] = true;
	std::vector<Ground> my_stack;
	Ground current = { x, z };
	my_stack.push_back(current);

	while (!my_stack.empty()) {
		//std::cerr << "in while" << std::endl;
		current = my_stack.back();
		my_stack.pop_back();
		x = current.row;
		z = current.col;

		if (x >= 10 && x < GSZ - 10 && z >= 10 && z < GSZ - 10) {
			std::cerr << "in if 1" << std::endl;
			if (ground[x][z] >= 0 && ground[x][z] > ground2[x][z]-1) {
				std::cerr << "in if 2" << std::endl;
				if (ground[x - 10][z] < ground2[x - 10][z] - 1 || ground[x + 10][z] < ground2[x + 10][z] - 1 || ground[x][z - 10] < ground2[x][z - 10] - 1 || ground[x][z + 10] < ground2[x][z + 10] - 1) { // at least one river around
					std::cerr << "in if 3" << std::endl;
					if (ground2[x - 10][z]  <= -1 || ground2[x + 10][z] <= -1 || ground2[x][z - 10] <= -1 || ground2[x][z + 10] <= -1) { // at least one sea around
						std::cerr << "in if 4" << std::endl;
						if (ground[x - 10][z] > 0 || ground[x + 10][z] > 0 || ground[x][z - 10] > 0 || ground[x][z + 10] > 0) { // at least one ground around
							std::cerr << "in if final" << std::endl;
							cityLoc.row = current.row;
							cityLoc.col = current.col;
							std::cout << "City location confirmed: (" << cityLoc.row << ", " << cityLoc.col << ")" << std::endl;
							return true; // Found a suitable location
						}
					}
				}
			}
		}

		// Add neighbors to the stack if they haven't been visited
		if (x < GSZ - 1 && !visited[x + 1][z])
		{
			visited[x + 1][z] = true;
			my_stack.push_back({ x + 1, z });
		}
		if (x > 0 && !visited[x - 1][z])
		{
			visited[x - 1][z] = true;
			my_stack.push_back({ x - 1, z });
		}
		if (z > 0 && !visited[x][z - 1])
		{
			visited[x][z - 1] = true;
			my_stack.push_back({ x, z - 1 });
		}
		if (z < GSZ - 1 && !visited[x][z + 1])
		{
			visited[x][z + 1] = true;
			my_stack.push_back({ x, z + 1 });
		}
	}
	return false;
}

void rain() {

	double delta = 0.015;
	int x, z;

	srand(time(0));

	x = rand() % GSZ;
	z = rand() % GSZ;

	double a, b, c, d, next;
	for (int i = 0; i < 5; i++) {
		x = rand() % GSZ-1;
		z = rand() % GSZ-1;
		while ((ground[x][z] + 0.5 > ground[x - 1][z] || ground[x][z] + 0.5 > ground[x][z - 1] || ground[x][z] + 0.5 > ground[x + 1][z] || ground[x][z] + 0.5 > ground[x][z + 1]) && (x >= 1 && z >= 1 && x < GSZ - 1 && z < GSZ - 1 && ground[x][z] > 0.0 && ground[x][z] < 4.0))
		{		
			a = ground[x - 1][z];
			b = ground[x][z - 1];
			c = ground[x + 1][z];
			d = ground[x][z + 1];
			
			if (std::max({ a, b, c, d, ground[x][z] }) == ground[x][z]) {
				ground[x][z] += delta*0.1;
			}
			else {
				ground[x][z] -= delta;
			}

			next = std::min({ a, b, c, d });

			if (next == a) {
				x -= 1;
			}
			else if (next == b) {
				z -= 1;
			}
			else if (next == c) {
				x += 1;
			}
			else z += 1;
		}
	}
}
// defines each pixel in texture
void SetTexture(int numTxture)
{
	int i, j;
	int rnd;
	switch(numTxture)
	{
	case 0: // bricks
		for(i=0;i<TH;i++)
			for (j = 0;j < TW;j++)
			{// gray
				rnd = rand() % 30;
				if (i % (TH/2) <= 15 ||
					(i<TH/2 &&j%(TW/2)<=15) ||// vertical gray line on bottom half
					(i>=TH/2 &&	( j>15 && j%(TW/4)<=15) &&(j<TW/2-15) ||
						i >= TH / 2 && (j % (TW / 4) <= 15) && (j > 3*TW / 4-15))) // vertical gray lines on top half
				{
					tx0[i][j][0] = 190+ rnd;
					tx0[i][j][1] = 190 + rnd;
					tx0[i][j][2] = 190 + rnd;
				}
				else  // bricks
				{
					tx0[i][j][0] = 180 + rnd;
					tx0[i][j][1] = 70 + rnd;
					tx0[i][j][2] = 0 + rnd;
				}
			}
		break;
	case 1: // road
		for (i = 0;i < TH;i++)
			for (j = 0;j < TW;j++)
			{// white lines
				rnd = rand() % 30;
				if (i>TH-15 || i<15 ||
					i<TH/2 && i>=TH/2-15 && j<TW/2)
				{
					tx0[i][j][0] = 255 - rnd;
					tx0[i][j][1] = 255 - rnd;
					tx0[i][j][2] = 255 - rnd;
				}
				else  // road
				{
					tx0[i][j][0] = 140 + rnd;
					tx0[i][j][1] = 140 + rnd;
					tx0[i][j][2] = 140 + rnd;
				}
			}
		break;
	}
}

void init()
{
	int i, j;
	glClearColor(0.5,0.7,0.9,0);// color of window background
	glEnable(GL_DEPTH_TEST);

	srand(time(0));

	for (i = 0;i < 3000;i++)
		UpdateTerrain2();
	for (i = 0;i < 500;i++)
		UpdateTerrain3();
	Smooth();

	for (i = 0;i < 15;i++)
		UpdateTerrain3();

	// aligning the mid line of terrain
	for (j = 0;j < GSZ;j++)
	{
		ground[GSZ / 2 - 1][j] = ground[GSZ / 2 + 1][j] = ground[GSZ / 2][j];
		ground2[GSZ / 2 - 1][j] = ground2[GSZ / 2 + 1][j] = ground2[GSZ / 2][j];

	}

	glEnable(GL_NORMALIZE);

	// setting texture
	SetTexture(0); // bricks
	glBindTexture(GL_TEXTURE_2D, 0); // texture number is 0
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TW, TH, 0, GL_RGB, GL_UNSIGNED_BYTE, tx0);

	SetTexture(1); // road
	glBindTexture(GL_TEXTURE_2D, 1); // texture number is 1
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TW, TH, 0, GL_RGB, GL_UNSIGNED_BYTE, tx0);


}

void UpdateTerrain1()
{
	double delta = 2;
	int x, z;

	x = rand() % GSZ;
	z = rand() % GSZ;

	if (rand() % 2 == 0)
		delta = -delta;

	ground[z][x] += delta;
	ground2[z][x] += delta;

}

void UpdateTerrain2()
{
	int x1, z1, x2, z2;
	double delta = 0.05;
	double a, b;
	int i, j;

	if (rand() % 2 == 0)
		delta = -delta;

	x1 = rand() % GSZ;
	z1 = rand() % GSZ;

	x2 = rand() % GSZ;
	z2 = rand() % GSZ;

	if (x1 != x2)
	{
		a = (z2 - z1) / ((double)(x2 - x1));
		b = z1 - a * x1;

		for(i=0;i<GSZ;i++)
			for (j = 0;j < GSZ;j++)
			{
				if (i < a * j + b) { ground[i][j] += delta; ground2[i][j] += delta;
				}
				else {
					ground[i][j] -= delta; ground2[i][j] -= delta;
				}
			}
	}

}

// random walk
void UpdateTerrain3()
{
	double delta = 0.02;
	int x, z,count;
	int numPoints = 800;

	x = rand() % GSZ;
	z = rand() % GSZ;

	if (rand() % 2 == 0)
		delta = -delta;

	for (count = 1;count <= numPoints;count++)
	{
		ground[z][x] += delta;
		ground2[z][x] += delta;
		switch (rand() % 4)
		{
		case 0: // right
			x++;
			break;
		case 1: // up
			z++;
			break;
		case 2: // down
			z--;
			break;
		case 3: // left
			x--;
			break;
		}
		x += GSZ;
		x = x % GSZ;
		z += GSZ;
		z = z % GSZ;
	}


}

void Smooth()
{
	int i, j;

	for(i=1;i<GSZ-1;i++)
		for (j = 1;j < GSZ - 1;j++)
		{
			tmp[i][j] = (ground[i + 1][j - 1] + 2*ground[i + 1][j] + ground[i + 1][j + 1] +
				2*ground[i ][j - 1] + 4*ground[i ][j] + 2*ground[i ][j + 1] + 
				ground[i - 1][j - 1] + 2*ground[i - 1][j] + ground[i - 1][j + 1] ) /16.0;
		}

	for (i = 1;i < GSZ - 1;i++)
		for (j = 1;j < GSZ - 1;j++)
			ground[i][j] = tmp[i][j];

	////  for under ground
	for (i = 1;i < GSZ - 1;i++)
		for (j = 1;j < GSZ - 1;j++)
		{
			tmp[i][j] = (ground2[i + 1][j - 1] + 2 * ground2[i + 1][j] + ground2[i + 1][j + 1] +
				2 * ground2[i][j - 1] + 4 * ground2[i][j] + 2 * ground2[i][j + 1] +
				ground2[i - 1][j - 1] + 2 * ground2[i - 1][j] + ground2[i - 1][j + 1]) / 16.0;
		}

	for (i = 1;i < GSZ - 1;i++)
		for (j = 1;j < GSZ - 1;j++)
			ground2[i][j] = tmp[i][j];

}



void SetColor(double h)
{
	h = fabs(h)/ 10.0; 

	if(h<0.03) // sand
		glColor3d(0.9, 0.8, 0.7);
	else if(h<0.5)// grass
		glColor3d(0.2+h/3,0.5-h/2,0);
	else
		glColor3d(1.2*h, 1.2*h, 1.3*h);
}

void SetNormal(int i, int j)
{
	double nx, ny, nz;

	nx = ground[i][j - 1] - ground[i][j];
	ny = 1;
	nz = ground[i - 1][j] - ground[i][j];
	glNormal3d(nx, ny, nz);
}

void drawOneRoad(Ground g1, Ground g2, int texType, double roadHigh) {
	bool canBouild = true;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texType); // use texture number 1 - road
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // GL_MODULATE to count on color

	if (g1.col == g2.col) {
		for (int i = g1.row; i <= g2.row; i++) {
			if (ground[i][g1.col] < ground2[i][g1.col] - 1 || ground2[i][g1.col] < 0) canBouild = false;
		}
		if (canBouild) {
			for (int i = g1.row; i <= g2.row; i++) {
				glBegin(GL_POLYGON);
				glTexCoord2d(0, 0);	glVertex3d(g1.col - GSZ / 2, ground[i][g1.col] + roadHigh, i - GSZ / 2); // 1
				glTexCoord2d(0, 2);	glVertex3d(g1.col - GSZ / 2 + 1, ground[i][g1.col + 1] + roadHigh, i - GSZ / 2); //2
				glTexCoord2d(2, 2);	glVertex3d(g1.col - GSZ / 2 + 1, ground[i + 1][g1.col + 1] + roadHigh, i - GSZ / 2 + 1);  //3
				glTexCoord2d(2, 0);	glVertex3d(g1.col - GSZ / 2, ground[i + 1][g1.col] + roadHigh, i - GSZ / 2 + 1);  // 4
				glEnd();
			}
		}
	}

	canBouild = true;
	if (g1.row == g2.row) {
		for (int i = g1.col; i <= g2.col; i++) {
			if (ground[g1.row][i] < ground2[g1.row][i] - 1 || ground2[g1.row][i] < 0) canBouild = false;
		}
		if (canBouild) {
			for (int i = g1.col; i <= g2.col; i++) {
				glBegin(GL_POLYGON);
				/*glPushMatrix();
				glRotated(PI/2,1,1,1);*/
				glTexCoord2d(0, 0);	glVertex3d(i - GSZ / 2, ground[g1.row][i] + roadHigh, g1.row - GSZ / 2); // 1
				glTexCoord2d(0, 2);	glVertex3d(i - GSZ / 2, ground[g1.row + 1][i] + roadHigh, g1.row - GSZ / 2 + 1);  // 4
				glTexCoord2d(2, 2);	glVertex3d(i - GSZ / 2 + 1, ground[g1.row + 1][i + 1] + roadHigh, g1.row - GSZ / 2 + 1);  //3
				glTexCoord2d(2, 0);	glVertex3d(i - GSZ / 2 + 1, ground[g1.row][i + 1] + roadHigh, g1.row - GSZ / 2); //2

				//glPopMatrix();
				glEnd();
			}
		}
	}
	glDisable(GL_TEXTURE_2D);
}

void DrawRoad()
{
	int j;
	int firstRow = cityLoc.row - 5;
	int firstCol = cityLoc.col - 5;

	//glEnable(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 1); // use texture number 1 - road
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // GL_MODULATE to count on color

	//for (j = 1;j < GSZ;j++)
	//{
	//	glBegin(GL_POLYGON);
	//	glTexCoord2d(0, 0);	glVertex3d(j - GSZ / 2 - 1, ground[GSZ / 2][j - 1] + 0.1, 1); // 1
	//	glTexCoord2d(0, 2);	 glVertex3d(j - GSZ / 2 - 1, ground[GSZ / 2][j - 1] + 0.1, -1); //2
	//	glTexCoord2d(2, 2);	  glVertex3d(j - GSZ / 2, ground[GSZ / 2][j] + 0.1, -1);  //3
	//	glTexCoord2d(2, 0);	  glVertex3d(j - GSZ / 2, ground[GSZ / 2][j] + 0.1, 1);  // 4
	//	glEnd();
	//}
	for (int r = firstRow; r <= cityLoc.row + 5; r++) {
		for (int c = firstCol; c <= cityLoc.col + 5; c = c + 5) {
			if (c + 5 <= cityLoc.col + 5)	drawOneRoad({ r, c }, { r, c + 5 }, 0, 0.1);
		}
	}

	for (int r = firstRow; r <= cityLoc.row + 5; r = r + 5) {
		for (int c = firstCol; c <= cityLoc.col + 5; c = c + 5) {
			if(c + 5 <= cityLoc.col + 5)	drawOneRoad({ r, c }, { r, c + 5}, 1, 0.12);
			if(r + 5 <= cityLoc.row + 5)	drawOneRoad({ r, c }, { r + 5 , c }, 1, 0.12);

		}
	}
}

void DrawFloor() // terrain
{
	int i,j;

	for(i=2;i<GSZ;i++)
		for (j = 2;j < GSZ;j++)
		{
			glBegin(GL_POLYGON);
			SetColor(ground[i][j]);
			glVertex3d(j-GSZ/2, ground[i][j], i - GSZ / 2);
			SetColor(ground[i-1][j]);
			glVertex3d(j - GSZ / 2, ground[i-1][j], i-1 - GSZ / 2);
			SetColor(ground[i-1][j-1]);
			glVertex3d(j-1 - GSZ / 2, ground[i - 1][j-1], i - 1 - GSZ / 2);
			SetColor(ground[i][j-1]);
			glVertex3d(j - 1 - GSZ / 2, ground[i][j - 1], i - GSZ / 2);
			glEnd();
		}

	for (i = 2;i < GSZ;i++)
		for (j = 2;j < GSZ;j++)
		{
			glBegin(GL_POLYGON);
			glColor4d(0, 0, 0.5, 0.7);
			glVertex3d(j - GSZ / 2, ground2[i][j] - 1, i - GSZ / 2);
			glVertex3d(j - GSZ / 2, ground2[i - 1][j] - 1, i - 1 - GSZ / 2);
			glVertex3d(j - 1 - GSZ / 2, ground2[i - 1][j - 1] - 1, i - 1 - GSZ / 2);
			glVertex3d(j - 1 - GSZ / 2, ground2[i][j - 1] - 1, i - GSZ / 2);
			glEnd();
		}

	if (isRain) rain();

	if (canBuildTown) {
		findTowenLoc();
		canBuildTown = false;
	}

	if (foundLocation) {
		DrawTown();
		DrawRoad();
	}

	/*if (canBuildTown) {
		findTowenLoc();
		canBuildTown = false;
	}*/


	// water surface (transparent)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4d(0, 0.3, 0.6,0.8);
	glBegin(GL_POLYGON);
	glVertex3d(-GSZ / 2, 0, -GSZ / 2);
	glVertex3d(-GSZ / 2, 0, GSZ / 2);
	glVertex3d(GSZ / 2, 0, GSZ / 2);
	glVertex3d(GSZ / 2, 0, -GSZ / 2);
	glEnd();

	glDisable(GL_BLEND);
}

void DrawCylinder(int n)
{
	double alpha, teta = 2 * PI / n;

	for (alpha = 0;alpha < 2 * PI;alpha += teta)
	{
			glBegin(GL_POLYGON); // upper points are darker
			glColor3d(1 - alpha / (3 * PI), 1 - fabs(sin(alpha)) / 2, fabs(cos(alpha)) / 2);
			glVertex3d(sin(alpha), 1, cos(alpha)); // point 1

			glColor3d(1 - alpha / (3 * PI), 1 - fabs(sin(alpha + teta) / 2), fabs(cos(alpha + teta)) / 2);
			glVertex3d(sin(alpha + teta), 1, cos(alpha + teta)); // point 2
			// lower points are brighter
			glColor3d(1 - alpha / (3 * PI), 1 - fabs(sin(alpha + teta) / 2), fabs(cos(alpha + teta)) / 2);
			glVertex3d(sin(alpha + teta), 0, cos(alpha + teta)); // point 3

			glColor3d(1 - alpha / (3 * PI), 1 - fabs(sin(alpha)) / 2, fabs(cos(alpha)) / 2);
			glVertex3d(sin(alpha), 0, cos(alpha)); //point 4
			glEnd();

	}
}

//void DrawCylinder1(int n, double topr, double bottomr)
//{
//	double alpha, teta = 2 * PI / n;
//
//	for (alpha = 0;alpha < 2 * PI;alpha += teta)
//	{
//		glBegin(GL_POLYGON); // upper points are darker
//		glColor3d(1 - alpha / (2 * PI), fabs(sin(alpha)) , fabs(cos(alpha)) );
//		glVertex3d(topr*sin(alpha), 1, topr * cos(alpha)); // point 1
//
//		glColor3d(1 - alpha / (2 * PI), fabs(sin(alpha + teta) ), fabs(cos(alpha + teta)) );
//		glVertex3d(topr * sin(alpha + teta), 1, topr * cos(alpha + teta)); // point 2
//		// lower points are brighter
//		glColor3d(1 - alpha / (2 * PI),  fabs(sin(alpha + teta)), fabs(cos(alpha + teta)));
//		glVertex3d(bottomr*sin(alpha + teta), 0, bottomr * cos(alpha + teta)); // point 3
//
//		glColor3d(1 - alpha / (2 * PI),  fabs(sin(alpha)), fabs(cos(alpha)) );
//		glVertex3d(bottomr * sin(alpha), 0, bottomr * cos(alpha)); //point 4
//		glEnd();
//	}
//}

// counts on lighting
//void DrawCylinder2(int n, double topr, double bottomr)
//{
//	double alpha, teta = 2 * PI / n;
//
//	for (alpha = 0;alpha < 2 * PI;alpha += teta)
//	{
//		glBegin(GL_POLYGON); // upper points are darker
//		glNormal3d(sin(alpha), bottomr - topr, cos(alpha));
//		glVertex3d(topr * sin(alpha), 1, topr * cos(alpha)); // point 1
//
//		glNormal3d(sin(alpha + teta), bottomr - topr, cos(alpha + teta));
//		glVertex3d(topr * sin(alpha + teta), 1, topr * cos(alpha + teta)); // point 2
//		// lower points are brighter
//		glVertex3d(bottomr * sin(alpha + teta), 0, bottomr * cos(alpha + teta)); // point 3
//
//		glNormal3d(sin(alpha), bottomr - topr, cos(alpha));
//		glVertex3d(bottomr * sin(alpha), 0, bottomr * cos(alpha)); //point 4
//		glEnd();
//	}
//}
//void DrawSphere(int n, int slices)
//{
//	double beta, gamma = PI / slices;
//	double topr, bottomr,height;
//
//	for (beta = -PI / 2; beta <= PI / 2;beta += gamma)
//	{
//		bottomr = cos(beta);
//		topr = cos(beta + gamma);
//		height = sin(beta + gamma) - sin(beta);
//		glPushMatrix();
//		glTranslated(0, sin(beta), 0);
//		glScaled(1,height , 1);
//		DrawCylinder1(n, topr, bottomr);
//		glPopMatrix();
//	}
//}
// 
// with lighting
//void DrawSphere1(int n, int slices)
//{
//	double beta, gamma = PI / slices;
//	double topr, bottomr, height;
//
//	for (beta = -PI / 2; beta < PI / 2-gamma;beta += gamma)
//	{
//		bottomr = cos(beta);
//		topr = cos(beta + gamma);
//		height = sin(beta + gamma) - sin(beta);
//		glPushMatrix();
//		glTranslated(0, sin(beta), 0);
//		glScaled(1, height, 1);
//		DrawCylinder2(n, topr, bottomr); // lighting
//		glPopMatrix();
//	}
//}

//void DrawApple(int n, int slices)
//{
//	double beta, gamma = PI / slices;
//	double topr, bottomr, height;
//
//	for (beta = 0; beta <= 2*PI;beta += gamma)
//	{
//		bottomr = cos(beta);
//		topr = cos(beta + gamma);
//		height = sin(beta + gamma) - sin(beta);
//		glPushMatrix();
//		glTranslated(0, sin(beta), 0);
//		glScaled(1, height, 1);
//		DrawCylinder1(n, 1+topr, 1+bottomr);
//		glPopMatrix();
//	}
//}


//void DrawTorus(int n, int slices)
//{
//	double beta, gamma = PI / slices;
//	double topr, bottomr, height;
//
//	for (beta = 0; beta <= 2 * PI;beta += gamma)
//	{
//		bottomr = cos(beta);
//		topr = cos(beta + gamma);
//		height = sin(beta + gamma) - sin(beta);
//		glPushMatrix();
//		glTranslated(0, sin(beta), 0);
//		glScaled(1, height, 1);
//		DrawCylinder1(n, 3 + topr, 3 + bottomr);
//		glPopMatrix();
//	}
//}

//void DrawSlider()
//{
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	glColor4d(1, 1, 0.0, 0.5);
//
//	// background
////	glColor3d(1, 1, 0);
//	glBegin(GL_POLYGON);
//	glVertex2d(-1, -1);
//	glVertex2d(-1, 1);
//	glVertex2d(1, 1);
//	glVertex2d(1, -1);
//	glEnd();
//	glDisable(GL_BLEND);
//
//	glColor3d(0, 0, 0);
//	glBegin(GL_LINES);
//	glVertex2d(0, -0.95);
//	glVertex2d(0, 0.95);
//	glEnd();
//	
//	// slider button
//	glColor3d(0.4,0.4,0.4);
//	glBegin(GL_POLYGON);
//	glVertex2d(-0.2, pitch-0.2);
//	glVertex2d(-0.2, pitch+0.2);
//	glVertex2d(0.2, pitch+0.2);
//	glVertex2d(0.2, pitch -0.2);
//	glEnd();
//
//	glColor3d(0, 1, 0);
//	glBegin(GL_LINES);
//	glVertex2d(-0.18, pitch);
//	glVertex2d(0.18, pitch);
//	glEnd();
//
//
//}


void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clean frame buffer and Z-buffer
	glViewport(0, 0, W, H);

	glMatrixMode(GL_PROJECTION); // matrix of projections / vision
	glLoadIdentity();
	glFrustum(-1, 1, -1, 1, 0.7, 300); // camera definitions
	gluLookAt(eye.x, eye.y, eye.z, // eye definitions
		eye.x+direction.x, eye.y+direction.y, eye.z+direction.z,  // CENTER / Point of Interest
		0, 1, 0); // UP vector

	glMatrixMode(GL_MODELVIEW); // transformations on model objects
	glLoadIdentity(); // start matrix of transformations from identity

	DrawFloor();

	//else {
	//	std::cerr << "No suitable location found to build the town." << std::endl;
	//	// Optionally set canBuildTown = false to avoid repeated attempts
	//	canBuildTown = false;
	//}
/*
//	glColor3d(0, 1, 0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 1); // use texture number 1
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // GL_MODULATE to count on color

	glBegin(GL_POLYGON);
	glTexCoord2d(0, 0);	glVertex3d(-10, 0, 0);
	glTexCoord2d(0, 2);	  glVertex3d(-10, 20, 0);
	glTexCoord2d(4, 2);	  glVertex3d(10, 20, 0);
	glTexCoord2d(4, 0);	  glVertex3d(10, 0, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
*/
	glutSwapBuffers(); // show all
}

void idle() 
{
//	if(terrainIsForming)
//		UpdateTerrain3();


	offset += 0.3;
	// ego-motion
	sight_angle += angular_speed; // if angular speed >0 we turn left, oterwise we turn right

	direction.x = sin(sight_angle);
	direction.z = cos(sight_angle);

	eye.x += speed * direction.x;
	eye.z += speed * direction.z;


	glutPostRedisplay();
}


void SpecialKeys(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		angular_speed+=0.001;
		break;
	case GLUT_KEY_RIGHT:
		angular_speed -= 0.001;
		break;

	case GLUT_KEY_UP:
		speed+=0.01;
		break;
	case GLUT_KEY_DOWN:
		speed-=0.01;
		break;

	case GLUT_KEY_PAGE_UP:
		eye.y += 0.1;
		break;
	case GLUT_KEY_PAGE_DOWN:
		eye.y -= 0.1;
		break;
	}
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		ac_speed += 0.02;
		break;
	case 's':
		ac_speed -= 0.02;
		break;
	case 'a':
		ac_angular_speed += 0.002;
		break;
	case 'd':
		ac_angular_speed -= 0.002;
		break;
	}
}


void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		int bottom = 10+100*(pitch+1)/2.0 -20, top=10 + 100 * (pitch + 1) / 2.0 + 20;
		if(x>40 && x<80 && H-y>bottom && H-y<top)
			isCaptured = true;
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		isCaptured = false;
	}
//		terrainIsForming = !terrainIsForming;
//	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
//		Smooth();
}


void MouseDrag(int x, int y)
{
	if (isCaptured && H-y>15 && H-y<105)
	{
		pitch = -1 + 2 * (H - y - 10) / 100.0; // updating pitch
	}
}

void menu(int choice)
{
	switch (choice)
	{
	case 1: // regular view
		isRain = true;
		break;
	case 2: // top view
		isRain = false;
		break;
	case 3: // top view
		isRain = false;
		if (!canBuildTown) {
			canBuildTown = true;  // Set to true only if it hasn't already been triggered
		}
		break;
	}

}

void main(int argc, char* argv[]) 
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);// defines buffer for virtual display
	glutInitWindowSize(W, H);
	glutInitWindowPosition(400, 100);
	glutCreateWindow("3D example");

	glutDisplayFunc(display); // here will be all drawings
	glutIdleFunc(idle); // all the changes must be here and display must be invoked from here

	glutSpecialFunc(SpecialKeys);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);

	// adding menu
	glutCreateMenu(menu);
	glutAddMenuEntry("start rain", 1);
	glutAddMenuEntry("stop rain", 2);
	glutAddMenuEntry("build town", 3);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
	glutMotionFunc(MouseDrag);



	init();

	glutMainLoop();
}