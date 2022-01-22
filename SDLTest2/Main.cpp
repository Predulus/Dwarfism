#include <iostream>
using namespace std;

//Using SDL, SDL_image, standard IO, and strings
#include <SDL.h>
#include "SDL_ttf.h"
#include <stdio.h>
#include <string>
#include <time.h>
#include "PerlinNoise.h"
#include "noise.h"
#include "noiseutils.h"
#include <cmath>
#include <vmmlib/vector.hpp>
using namespace vmml;


using namespace noise;

// Create a PerlinNoise object with the reference permutation vector
PerlinNoise* pn;
module::Perlin modPN;
module::Perlin terrainType;
utils::NoiseMap heightMap;
module::RidgedMulti mountainTerrain;
module::ScaleBias flatTerrain;
module::Billow baseFlatTerrain;
utils::NoiseMapBuilderPlane heightMapBuilder;
module::Select finalTerrain;

double mountainTerrainFrequency;
double baseFlatTerrainFrequency;
double baseFlatTerrainPersistence;
double terrainTypeFrequency;
double terrainTypePersistence;
double flatTerrainScale;
double flatTerrainBias;
double finalTerrainUprBound;
double finalTerrainLwrBound;
double finalTerrainEdgeFalloff;

double lowlandsFactor = 25;

//Screen dimension constants
const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

//Starts up SDL and creates window
bool Init();

void RenderMessage(const char*, int, double, int, int, int, int, SDL_Renderer*);
void GetBlockSize();
void Close();
void InitNoiseGrid();
void NewLandGrid();
void DrawInitialLandLevelsToTextures();

	//Frees media and shuts down SDL
//void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The surface contained by the window
SDL_Surface* gScreenSurface = NULL;

Uint32 pixelFormat;

int xpos = 0;
int mousePosX;
int mousePosY;
int mouseXIndex;
int mouseYIndex;
auto noiseGridMaxHeight = 1.0;
auto noiseGridMinHeight = 0.65;
auto ScreenWidth = 0;
auto ScreenHeight = 0;
auto numBlocksX = 240;
auto numBlocksY = 200;
const auto numBlocksZ = 100;
double originalBlockWidth;
double originalBlockHeight;
auto lowestGroundZ = numBlocksZ / 2;
auto zlevelMax = numBlocksZ - 1;
auto zlevel = 64; // (int)(numBlocksZ / 2);
auto homeZ = zlevel;
auto homeX = 0;
auto homeY = 0;
auto originXIndex = 0;
auto originYIndex = 0;
auto scale = 1.0;
auto homeScale = scale;
auto scaleDelta = 0.1;
auto scaleMax = 5.0;
double blockWidth = 0.0;
double blockHeight = 0.0;
SDL_Renderer* renderer;
auto speed = 2;
auto deltax = speed;
double** noiseGrid;
int*** landGrid;
auto noiseScale = 1;
TTF_Font* Sans;
const int BLACK = 0;
const int GREEN = 1;
const int BLUE = 2;
const int GREEN1 = 3;
const int GREEN2 = 4;
const int GREEN3 = 5;
const int GREEN4 = 6;
const int GREEN5 = 7;
const int GREEN6 = 8;
const int GREEN7 = 9;

SDL_Texture* levelTextures[numBlocksZ];


void BuildNoise() {
	mountainTerrain.SetFrequency(mountainTerrainFrequency);
	baseFlatTerrain.SetFrequency(baseFlatTerrainFrequency);
	baseFlatTerrain.SetPersistence(baseFlatTerrainPersistence);
	terrainType.SetFrequency(terrainTypeFrequency);
	terrainType.SetPersistence(terrainTypePersistence);

	flatTerrain.SetSourceModule(0, baseFlatTerrain);
	flatTerrain.SetScale(flatTerrainScale);
	flatTerrain.SetBias(flatTerrainBias);

	finalTerrain.SetSourceModule(0, flatTerrain);
	finalTerrain.SetSourceModule(1, mountainTerrain);
	finalTerrain.SetControlModule(terrainType);
	finalTerrain.SetBounds(finalTerrainLwrBound, finalTerrainUprBound);
	finalTerrain.SetEdgeFalloff(finalTerrainEdgeFalloff);

	heightMapBuilder.SetBounds(0, numBlocksX, 0, numBlocksY);
	heightMapBuilder.SetSourceModule(finalTerrain);
	heightMapBuilder.SetDestNoiseMap(heightMap);
	heightMapBuilder.SetDestSize(numBlocksX, numBlocksY);
	heightMapBuilder.Build();
}

void ReSeedHeightMap() {
	unsigned int t = (unsigned int)time(NULL);
	terrainType.SetSeed(t);
	heightMapBuilder.Build();
}

bool InitLevelTextures(SDL_Renderer* renderer) {
	for (int i = 0; i < numBlocksZ; i++) {
		printf("Attempting to create texture for level %d\n", i);
		levelTextures[i] = SDL_CreateTexture(renderer, pixelFormat, SDL_TEXTUREACCESS_TARGET, ScreenWidth, ScreenHeight);
		if (levelTextures[i] == NULL) {
			printf("Failed to create texture for z-level: %d\n", i);
			printf("SDL Error : % s\n", SDL_GetError());
			return false; // texture creation failed
		}
	}
	return true;
}

void DestroyLevelTextures() {
	for (int i = 0; i < numBlocksZ; i++) {
		SDL_DestroyTexture(levelTextures[i]);
		levelTextures[i] = NULL;
	}
}

bool Init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		if (TTF_Init() < 0) {
			std::cout << "Couldn't initialize TTF lib: " << TTF_GetError() << std::endl;
		}
		Sans = TTF_OpenFont("D:/Users/user/source/repos/SDLTest2/GeosansLight.ttf", 18); //this opens a font style and sets a size
		if (!Sans) {
			printf("TTF_OpenFont: %s\n", TTF_GetError());
			// handle error
		}
		unsigned int t = (unsigned int)time(NULL);
		pn = new PerlinNoise(t);
		modPN.SetSeed(t);
		//Create window
		gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			mountainTerrainFrequency = (0.005);
			baseFlatTerrainFrequency = (0.0005);
			baseFlatTerrainPersistence = (0.0025);
			terrainTypeFrequency = (0.01);
			terrainTypePersistence = (0.0025);
			flatTerrainScale = (0.00125);
			flatTerrainBias = (-100.75);
			finalTerrainUprBound = (10.0);
			finalTerrainLwrBound = (0.0);
			finalTerrainEdgeFalloff = (1.125);
			BuildNoise();

			//Get window surface
			gScreenSurface = SDL_GetWindowSurface(gWindow);
			renderer = SDL_CreateRenderer(gWindow, -1, 0);


			//			SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
			//SDL_FillRect(gScreenSurface, NULL, SDL_MapRGB(gScreenSurface->format, 255, 0, 0));
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_DisplayMode DM;
			SDL_GetCurrentDisplayMode(0, &DM);
			ScreenWidth = DM.w;
			ScreenHeight = DM.h;
			//numBlocksY = (int)(numBlocksX * (double)ScreenHeight / (double)ScreenWidth);

			pixelFormat = SDL_GetWindowPixelFormat(gWindow);  //must be called after window creation
			printf("Pixel format was: %s\n", SDL_GetPixelFormatName(pixelFormat));
			InitLevelTextures(renderer); //must be called after renderer creation, SDL_GetWindowPixelFormat call, ScreenWidth and height obtained

			originalBlockWidth = ScreenWidth / numBlocksX;
			originalBlockHeight = ScreenHeight / numBlocksY;
			blockWidth = originalBlockWidth;
			blockHeight = originalBlockHeight;

			SDL_StartTextInput();
			printf("Initialized.\n");

			InitNoiseGrid();
			NewLandGrid();
			DrawInitialLandLevelsToTextures();

		}
	}
	return success;
}

double FindMin() {
	double min = 100000.0;
		for (int i = 0; i < numBlocksX; i++) {
			for (int j = 0; j < numBlocksY; j++) {
				if (noiseGrid[i][j] < min)
					min = noiseGrid[i][j];
			}
		}
	return min;
}

double FindMax() {
	double max = -100000.0;
		for (int i = 0; i < numBlocksX; i++) {
			for (int j = 0; j < numBlocksY; j++) {
				if (noiseGrid[i][j] > max)
					max = noiseGrid[i][j];
			}
		}
	return max;
}

void CreateNoiseGridFromHeightMap() { // Fills noiseGrid with values from the libnoise HeightMap
	unsigned int kk = 0;
	// Visit every block of the image and assign a color generated with Perlin noise
	for (int i = 0; i < numBlocksX; i++) {     // y
		for (int j = 0; j < numBlocksY; j++) {  // x
			double x = (double)i / ((double)numBlocksX);
			double y = (double)j / ((double)numBlocksY);
			noiseGrid[i][j] = heightMap.GetValue(i, j);
		}
	}

	// Scale the grid to 0.0 --> 1
	double min = FindMin();
	double max = FindMax();
	double range = max - min;
	for (int i = 0; i < numBlocksX; i++) {
		for (int j = 0; j < numBlocksY; j++) {
			noiseGrid[i][j] = ((noiseGrid[i][j] - min) /range);
			if (noiseGrid[i][j] > noiseGridMaxHeight)
				noiseGrid[i][j] = noiseGridMaxHeight;
			if (noiseGrid[i][j] < noiseGridMinHeight)
				noiseGrid[i][j] = ((noiseGrid[i][j]- noiseGridMinHeight) / lowlandsFactor) + noiseGridMinHeight;
			if (noiseGrid[i][j] > 1.0)
				printf("Noise value greater than 1.0 detected.");
		}
	}
}

void InitNoiseGrid() {	// Allocates mem for noiseGrid and calls CreateNoiseGridFromHeightMap to fill it with values from the libnoise HeightMap 
	noiseGrid = new double* [numBlocksX];
	for (int i = 0; i < numBlocksX; ++i)
		noiseGrid[i] = new double[numBlocksY];
	CreateNoiseGridFromHeightMap();				
}

void DeleteNoiseGrid() {
	for (int i = 0; i < numBlocksX; ++i)
		delete[] noiseGrid[i];
	delete noiseGrid;
}

double AdjustedNoiseGrid(int i, int j) {  
	double lowestAsFractionofTotal = lowestGroundZ/ numBlocksZ;
	double adjusted = ((1.0 - lowestAsFractionofTotal) * noiseGrid[i][j]) + lowestAsFractionofTotal;
	return adjusted;
}
void NewLandGrid() {
	GetBlockSize();
	landGrid = new int** [numBlocksX];
	for (int i = 0; i < numBlocksX; i++) {
		landGrid[i] = new int* [numBlocksY];
		for (int j = 0; j < numBlocksY; j++) { 
			landGrid[i][j] = new int[numBlocksZ];
			int numSteps = (noiseGridMaxHeight - noiseGridMinHeight) * numBlocksZ;
			for (int k = 0; k < numBlocksZ; k++) {
				if (k < lowestGroundZ) {
					landGrid[i][j][k] = BLACK;
				}
				//double heightVal = noiseGridMinHeight + ((double)k * (noiseGridMaxHeight - noiseGridMinHeight) / (double)numBlocksZ);
				//double nextHeightVal = noiseGridMinHeight + ((double)k + 1 * (noiseGridMaxHeight - noiseGridMinHeight) / (double)numBlocksZ);
				double heightVal = (double)k/(double)numBlocksZ;
				double nextHeightVal = ((double)k + 1.0) / (double)numBlocksZ;
				if (noiseGrid[i][j] < heightVal) {
					double diff = heightVal - noiseGrid[i][j];
					if (diff < (1.0 / (double)numBlocksZ))
						landGrid[i][j][k] = GREEN1;
					else if (diff < (2.0 / (double)numBlocksZ))
						landGrid[i][j][k] = GREEN2;
					else if (diff < (3.0 / (double)numBlocksZ))
						landGrid[i][j][k] = GREEN3;
					else if (diff < (4.0 / (double)numBlocksZ))
						landGrid[i][j][k] = GREEN4;
					else if (diff < (5.0 / (double)numBlocksZ))
						landGrid[i][j][k] = GREEN5;
					else if (diff < (6.0 / (double)numBlocksZ))
						landGrid[i][j][k] = GREEN6;
					else if (diff < (7.0 / (double)numBlocksZ))
						landGrid[i][j][k] = GREEN7;
					//printf("%d\n", landGrid[i][j][k]);
					else landGrid[i][j][k] = BLUE;
				}
				else {
					if (noiseGrid[i][j] >= heightVal && noiseGrid[i][j] <= nextHeightVal) {
						landGrid[i][j][k] = GREEN;
						//printf("%d\n", landGrid[i][j][k]);
					}
					else {
						landGrid[i][j][k] = BLACK;
						//printf("%d\n", landGrid[i][j][k]);
					}
				}
			}
		}
	}
}

void UpdateLandGrid() {
	GetBlockSize();
	for (int i = 0; i < numBlocksX; i++) {
		for (int j = 0; j < numBlocksY; j++) {
			int numSteps = (noiseGridMaxHeight - noiseGridMinHeight) * numBlocksZ;
			for (int k = 0; k < numBlocksZ; k++) {
				if (k < lowestGroundZ) {
					landGrid[i][j][k] = BLACK;
				}
				//double heightVal = noiseGridMinHeight + ((double)k * (noiseGridMaxHeight - noiseGridMinHeight) / (double)numBlocksZ);
				//double nextHeightVal = noiseGridMinHeight + ((double)k + 1 * (noiseGridMaxHeight - noiseGridMinHeight) / (double)numBlocksZ);
				double heightVal = (double)k / (double)numBlocksZ;
				double nextHeightVal = ((double)k + 1.0) / (double)numBlocksZ;
				if (noiseGrid[i][j] < heightVal) {
					double diff = heightVal - noiseGrid[i][j];
					if (diff < (1.0 / (double)numBlocksZ))
						landGrid[i][j][k] = GREEN1;
					else if (diff < (2.0 / (double)numBlocksZ))
						landGrid[i][j][k] = GREEN2;
					else if (diff < (3.0 / (double)numBlocksZ))
						landGrid[i][j][k] = GREEN3;
					else if (diff < (4.0 / (double)numBlocksZ))
						landGrid[i][j][k] = GREEN4;
					else if (diff < (5.0 / (double)numBlocksZ))
						landGrid[i][j][k] = GREEN5;
					else if (diff < (6.0 / (double)numBlocksZ))
						landGrid[i][j][k] = GREEN6;
					else if (diff < (7.0 / (double)numBlocksZ))
						landGrid[i][j][k] = GREEN7;
					//printf("%d\n", landGrid[i][j][k]);
					else landGrid[i][j][k] = BLUE;
				}
				else {
					if (noiseGrid[i][j] >= heightVal && noiseGrid[i][j] <= nextHeightVal) {
						landGrid[i][j][k] = GREEN;
						//printf("%d\n", landGrid[i][j][k]);
					}
					else {
						landGrid[i][j][k] = BLACK;
						//printf("%d\n", landGrid[i][j][k]);
					}
				}
			}
		}
	}
}

//void fillLandCube() {
//
//}

void DeleteLandGrid() {
	for (int i = 0; i < numBlocksX; i++) {
		for (int j = 0; j < numBlocksY; j++)
		{
			delete landGrid[i][j];
		}
		delete landGrid[i];
	}
	delete landGrid;
}

void MakeRiver() {
	;
}

void GetBlockSize() {
	blockWidth = originalBlockWidth * scale;
	blockHeight = originalBlockHeight * scale;
}

vec2f GetWorldPosFromBlockIndices(int i, int j) {
	int x = i * blockWidth;
	int y = j * blockHeight;
	return vec2f(x, y);
}


void DrawLandLevelToTexture(int _zlevel) {
	GetBlockSize();
	SDL_Rect rect;
	rect.w = blockWidth+1;
	rect.h = blockHeight+1;
	//printf("Drawland\n");
	//printf("ypos: %i\n", rect.y);
	printf("Drawing land to texture for level %d.\n", _zlevel);

	if (SDL_SetRenderTarget(renderer, levelTextures[_zlevel]) != 0) {
		printf("Renderer could not be targeted to texture for level %d.\nSDL Error: %s\n", _zlevel, SDL_GetError());
		Close();
		exit(EXIT_FAILURE);
	}
	SDL_RenderClear(renderer);
	for (int i = 0; i < numBlocksX; i++) {
		//printf("\n");
		for (int j = 0; j < numBlocksY; j++) {
			int landType = landGrid[i][j][_zlevel];
			//printf("%d ", landType);
			switch (landType) {
			case BLUE:
				SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);   // 0 is 
				break;
			case GREEN:
				SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);   // 1 is
				break;
			case GREEN1:
				SDL_SetRenderDrawColor(renderer, 0, 239, 11, 255);   // 1 is
				break;
			case GREEN2:
				SDL_SetRenderDrawColor(renderer, 0, 225, 20, 255);   // 1 is
				break;
			case GREEN3:
				SDL_SetRenderDrawColor(renderer, 0, 213, 28, 255);   // 1 is
				break;
			case GREEN4:
				SDL_SetRenderDrawColor(renderer, 0, 203, 34, 255);   // 1 is
				break;
			case GREEN5:
				SDL_SetRenderDrawColor(renderer, 0, 193, 40, 255);   // 1 is
				break;
			case GREEN6:
				SDL_SetRenderDrawColor(renderer, 0, 183, 46, 255);   // 1 is
				break;
			case GREEN7:
				SDL_SetRenderDrawColor(renderer, 0, 173, 52, 255);   // 1 is
				break;
			case BLACK:
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);     // 2 is
				break;
			}
			rect.x = (i - originXIndex) * blockWidth;
			rect.y = (j - originYIndex) * blockHeight;
			//printf("%d %d %d %d ||| ", rect.x, rect.y, rect.w, rect.h);
			SDL_RenderFillRect(renderer, &rect);
		}
		//printf("%i\n", (i - originXIndex) * blockWidth);
	}
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}


void DrawInitialLandLevelsToTextures() {
	for (int i = 0; i < numBlocksZ; i++) {
		DrawLandLevelToTexture(i);
	}
}

void DrawLand() {
	GetBlockSize();
	SDL_Rect rect;
	rect.w = blockWidth + 1;
	rect.h = blockHeight + 1;

	if (SDL_SetRenderTarget(renderer, NULL) != 0) {
		printf("Renderer could not be targeted back to screen.\nSDL Error: %s\n", SDL_GetError());
		Close();
		exit(EXIT_FAILURE);
	}
	SDL_RenderClear(renderer);
	
	if (SDL_RenderCopy(renderer, levelTextures[zlevel], NULL, NULL) != 0) {
		printf("Failed copying texture for level %d to screen renderer.\nSDL Error: %s\n", zlevel, SDL_GetError());
		Close();
		exit(EXIT_FAILURE);
	}

	RenderMessage("Z level:", zlevel, 9999999.9, 100, 100, 150, 36, renderer);
	RenderMessage("lowlandsFactor:", 1, lowlandsFactor, 100, 150, 250, 36, renderer);
	RenderMessage("Scale:", 1, scale, 100, 200, 150, 36, renderer);
	RenderMessage("Origin:", 1, scale, 100, 200, 150, 36, renderer);
	RenderMessage("Scale:", 1, scale, 100, 200, 150, 36, renderer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_RenderPresent(renderer);
}



void RenderMessage(const char *msg, int i, double d, int x, int y, int w, int h, SDL_Renderer* localRenderer) {
	SDL_Color White = { 255, 255, 255 };
	SDL_Color Red = { 255, 0, 0 };
	string s1(msg);
	string s2(" ");
	string s3;
	if (d > 999999)
		s3 = to_string(i);
	else
		s3 = to_string(d);
	
	string msgStr = s1 + s2 + s3;
	const char* zStr = msgStr.c_str();
	SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, zStr, Red); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
	SDL_Texture* Message = SDL_CreateTextureFromSurface(localRenderer, surfaceMessage); //now you can convert it into a texture
	SDL_Rect Message_rect; //create a rect
	Message_rect.x = x;  //controls the rect's x coordinate 
	Message_rect.y = y; // controls the rect's y coordinte
	Message_rect.w = w; // controls the width of the rect
	Message_rect.h = h; // controls the height of the rect
	SDL_RenderCopy(localRenderer, Message, NULL, &Message_rect); //you put the renderer's name first, the Message, 
															//the crop size(you can ignore this if you don't want 
															//to dabble with cropping), and the rect which is the 
															//size and coordinate of your texture

			//Don't forget to free your surface and texture
	SDL_FreeSurface(surfaceMessage);
	SDL_DestroyTexture(Message);
}
void Close()
{
	SDL_StopTextInput();
	DeleteNoiseGrid();
	DeleteLandGrid();

	DestroyLevelTextures();

	//Destroy window
	SDL_SetWindowFullscreen(gWindow, 0);

	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Start up SDL and create window
	if (!Init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Main loop flag
		bool quit = false;

		//Event handler
		SDL_Event e;
		bool shift = false;
		//While application is running
		while (!quit)
		{
			//Handle events on queue
			while (SDL_PollEvent(&e) != 0) {
				//User requests quit
				if (e.type == SDL_QUIT)
				{
					quit = true;
				}
				else if (e.type == SDL_KEYDOWN)
				{
					/* Check the SDLKey values and move change the coords */
					unsigned int t = (unsigned int)time(NULL);
					switch (e.key.keysym.sym) {
					//case 'r':
					//	delete pn;
					//	pn = new PerlinNoise(t);
					//	deleteNoiseGrid();
					//	deleteLandGrid();
					//	newNoiseGrid();
					//	newLandGrid();
					//	break;
					case 'q':
						if (shift)
							mountainTerrainFrequency -= 0.01;
						else
							mountainTerrainFrequency += 0.01;
						break;

					case 'w':
						if (shift)
							baseFlatTerrainFrequency -= 0.0001;
						else
							baseFlatTerrainFrequency += 0.0001;
						break;

					case 'e':
						if (shift)
							baseFlatTerrainPersistence -= 0.001;
						else
							baseFlatTerrainPersistence += 0.001;
						break;

					case '[':
						ReSeedHeightMap();
						CreateNoiseGridFromHeightMap();
						break;

					case 'l':
						if (shift) {
							lowlandsFactor -= 0.5;
							if (lowlandsFactor < 1.0)
								lowlandsFactor = 1.0;
						}
						else
							lowlandsFactor += 0.5;
						break;

					case 'r':
						if (shift)
							terrainTypeFrequency -= 0.0025;
						else
							terrainTypeFrequency += 0.0025;
						break;

					case 't':
						if (shift)
							terrainTypePersistence -= 0.01;
						else
							terrainTypePersistence += 0.01;
						break;


					case 'y':
						if (shift)
							flatTerrainScale -= 0.0001;
						else
							flatTerrainScale += 0.0001;
						break;

					case 'u':
						if (shift)
							flatTerrainBias -= -100.0;
						else
							flatTerrainBias += -100.0;
						break;

					case 'i':
						if (shift)
							finalTerrainLwrBound -= 10.0;
						else
							finalTerrainLwrBound += 10.0;
						break;

					case 'o':
						if (shift)
							finalTerrainUprBound -= 10.0;
						else
							finalTerrainUprBound += 10.0;
						break;

					case 'p':
						if (shift)
							finalTerrainEdgeFalloff -= 0.2;
						else
							finalTerrainEdgeFalloff += 0.2;
						break;

					case ',':
						zlevel += 1.0;
						if (zlevel > zlevelMax)
							zlevel = zlevelMax;
						break;	
					case '.':
						zlevel -= 1.0;
						if (zlevel < 0)
							zlevel = 0;
						break;
					case SDLK_F1:
						originXIndex = homeX;
						originYIndex = homeY;
						scale = homeScale;
						zlevel = homeZ;
						break;
					case SDLK_F2:
						homeX = originXIndex;
						homeY = originYIndex;
						homeScale = scale;
						homeZ = zlevel;
						break;
					case SDLK_LSHIFT:
						printf("LSHIFT\n");
						shift = true;
						break;
					case SDLK_RSHIFT:
						printf("RSHIFT\n");
						shift = true;
						break;
					case SDLK_LEFT:
						if (shift) {
							printf("SHIFT + LEFT\n");
							originXIndex -= 10;
							if (originXIndex < 0)
								originXIndex = 0;
						}
						else {
							originXIndex -= 1;
							if (originXIndex < 0)
								originXIndex = 0;
							printf("LEFT\n");
						}
						break;
					case SDLK_RIGHT:
						if (shift) {
							originXIndex += 10;
							if (originXIndex + (ScreenWidth / (int)blockWidth) > numBlocksX)
								originXIndex -= 10;
						}
						else {
							originXIndex += 1;
							if (originXIndex + (ScreenWidth / (int)blockWidth) > numBlocksX)
								originXIndex -= 1;
						}
						break;
					case SDLK_UP:
						if (shift) {
							originYIndex -= 10;
							if (originYIndex < 0)
								originYIndex = 0;
						}
						else {
							originYIndex -= 1;
							if (originYIndex < 0)
								originYIndex = 0;
						}
						break;
					case SDLK_DOWN:
						if (shift) {
							originYIndex += 10;
							if (originYIndex + (ScreenHeight / (int)blockHeight) > numBlocksY)
								originYIndex -= 10;
						}
						else {
 							originYIndex += 1;
							if (originYIndex + (ScreenHeight/(int)blockHeight) > numBlocksY)
								originYIndex -= 1;
						}
						break;
					case SDLK_ESCAPE:
						quit = true;
					default:
						break;
					}
				}
				else if (e.type == SDL_KEYUP)
				{
					/* Check the SDLKey values and move change the coords */
					switch (e.key.keysym.sym) {
					case SDLK_RSHIFT:
						shift = false;
						break;

					case SDLK_LSHIFT:
						shift = false;
						break;
					}
				}
				else if (e.type == SDL_MOUSEWHEEL)
				{
					SDL_GetMouseState(&mousePosX, &mousePosY);
					//float oldBlockWidth = blockWidth;  //indices before scale change
					//float oldBlockHeight = blockHeight;
					//float offsetX = originXIndex * oldBlockWidth;
					//float offsetY = originYIndex * oldBlockHeight;


					if (e.wheel.y > 0) // scroll up
					{
						float os = scale; // cache old scale
						scale += scaleDelta;
						if (scale > scaleMax)
							scale = scaleMax;
						else {
							float n = mousePosX / blockWidth;
							float nn = (blockWidth * n) / (blockWidth + (originalBlockWidth * scaleDelta));
							originXIndex = (int)(originXIndex + n - nn);

							float m = mousePosY / blockHeight;
							float mm = (blockHeight * m) / (blockHeight + (originalBlockHeight * scaleDelta));
							originYIndex = (int)(originYIndex + m - mm);
						}
					}
					else if (e.wheel.y < 0) // scroll down
					{
						float os = scale; // cache old scale
						scale -= scaleDelta;
						if (scale < 1.0)
							scale = 1.0;
						else {
							float n = mousePosX / blockWidth;
							float nn = (blockWidth * n) / (blockWidth + (originalBlockWidth * -scaleDelta));
							originXIndex = (int)(originXIndex + n - nn);
							if (originXIndex < 0) // safety check
								originXIndex = 0;
							float m = mousePosY / blockHeight;
							float mm = (blockHeight * m) / (blockHeight + (originalBlockHeight * -scaleDelta));
							originYIndex = (int)(originYIndex + m - mm);
							if (originYIndex < 0) // safety check
								originYIndex = 0;
						}
					}
					GetBlockSize();
					//originXIndex = (scale * ((float)mousePosX + offsetX) - mousePosX)/blockWidth;
					//originYIndex = (scale * ((float)mousePosY + offsetY) - mousePosY)/blockHeight;
				}
			}
//			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	//		SDL_RenderClear(renderer);
		//	SDL_RenderPresent(renderer);
			//animate();
			//draw();
			//drawNoise();
			UpdateLandGrid();
			DrawLand();
		}
	}
	//Free resources and close SDL
	Close();

	return 0;
}