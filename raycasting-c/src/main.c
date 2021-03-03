#include <stdio.h>
#include <limits.h>
#include <SDL2/SDL.h>
#include "constants.h"
#include "textures.h"

const int map[MAP_NUM_ROWS][MAP_NUM_COLS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ,1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

struct Player {
	float x;
	float y;
	float width;
	float height;
	int turnDirection; // -1 for left, +1 for right
	int walkDirection; // -1 for back, +1 for front
	float rotationAngle;
	float walkSpeed;
	float turnSpeed;
} player;

struct Ray {
	float rayAngle;
	float wallHitX;
	float wallHitY;
	float distance;
	int wallHitContent;
	int wasHitVertical;
	int isRayFacingUp;
	int isRayFacingDown;
	int isRayFacingLeft;
	int isRayFacingRight;
} rays[NUM_RAYS];

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int isGameRunning = FALSE;
float ticksLastFrame = 0;

Uint32* colorBuffer = NULL;
SDL_Texture* colorBufferTexture;
Uint32* wallTexture = NULL;
Uint32* textures[NUM_TEXTURES];

int initializeWindow()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		fprintf(stderr, "Error initializing SDL.\n");
		return (FALSE);
	}
	window = SDL_CreateWindow(
		NULL,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		SDL_WINDOW_BORDERLESS
	);
	if (!window)
	{
		fprintf(stderr, "Error creating SDL window.\n");
		return (FALSE);
	}
	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer)
	{
		fprintf(stderr, "Error creating SDL renderer.\n");
		return (FALSE);
	}
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	return (TRUE);
}

int mapHasWallAt(float x, float y)
{
	if (x < 0 || x > WINDOW_WIDTH || y < 0 || y > WINDOW_HEIGHT)
		return TRUE;
	int mapGridIndexX = floor(x / TILE_SIZE);
	int mapGridIndexY = floor(y / TILE_SIZE);
	// printf("%d\n",map[mapGridIndexY][mapGridIndexX]);
	return map[mapGridIndexY][mapGridIndexX] != 0;
}

void movePlayer(float deltaTime){
	player.rotationAngle += player.turnDirection * player.turnSpeed * deltaTime;
	float moveStep = player.walkDirection * player.walkSpeed * deltaTime;

	float newPlayerX = player.x + cos(player.rotationAngle) * moveStep;
	float newPlayerY = player.y + sin(player.rotationAngle) * moveStep;
	if(!mapHasWallAt(newPlayerX, newPlayerY))
	{
		player.x = newPlayerX;
		player.y = newPlayerY;
	}
}

void renderPlayer() {
	SDL_SetRenderDrawColor(renderer,255, 255, 255, 255);
	SDL_Rect playerRect = {
		MINIMAP_SCALE_FACTOR * player.x,
		MINIMAP_SCALE_FACTOR * player.y,
		MINIMAP_SCALE_FACTOR * player.width,
		MINIMAP_SCALE_FACTOR * player.height
	};
	SDL_RenderFillRect(renderer, &playerRect);

	SDL_RenderDrawLine(
		renderer,
		MINIMAP_SCALE_FACTOR * player.x,
		MINIMAP_SCALE_FACTOR * player.y,
 		(MINIMAP_SCALE_FACTOR * player.x) + cos(player.rotationAngle) * 40,
		(MINIMAP_SCALE_FACTOR * player.y) + sin(player.rotationAngle) * 40
	);
}

float normalizeAngle(float angle)
{
	angle = remainder(angle , TWO_PI);
	if (angle < 0)
		angle = TWO_PI + angle;
	return angle;
}

float distanceBetweenPoints(float x1, float y1, float x2, float y2)
{
	return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

void castRay(float rayAngle, int stripId)
{
	rayAngle = normalizeAngle(rayAngle);

	int isRayFacingDown = rayAngle > 0 && rayAngle < PI;
	int isRayFacingUp = !isRayFacingDown;

	int isRayFacingRight = rayAngle < 0.5 * PI || rayAngle > 1.5 * PI;
	int isRayFacingLeft = !isRayFacingRight;

	float xintercept, yintercept;
	float xstep, ystep;

	//Horizontal ray-grid intersection code.
	int foundHorzWallHit = FALSE;
	float horzWallHitX = 0;
	float horzWallHitY = 0;
	int horzWallContent = 0;

	yintercept = floor(player.y / TILE_SIZE) * TILE_SIZE;
	yintercept += isRayFacingDown ? TILE_SIZE : 0;

	xintercept = player.x + (yintercept - player.y) / tan(rayAngle);

	ystep = TILE_SIZE;
	ystep *= isRayFacingUp ? -1 : 1;

	// TILE_SIZE is ystep. xstep adjusted by ystep.
	xstep = TILE_SIZE / tan(rayAngle);

	//When a race that was facing left last time turns to the right.
	xstep *= (isRayFacingLeft && xstep > 0) ? -1 : 1;
	//When a race that was facing right last time turns to the left.
	xstep *= (isRayFacingRight && xstep < 0) ? -1 : 1;

	float nextHorzTouchX = xintercept;
	float nextHorzTouchY = yintercept;

	while (nextHorzTouchX >= 0 && nextHorzTouchX <= WINDOW_WIDTH && nextHorzTouchY >= 0 && nextHorzTouchY <= WINDOW_HEIGHT)
	{
		float xToCheck = nextHorzTouchX;
		// In the case up, it is changed cell by incrementing pixel.
		float yToCheck = nextHorzTouchY + (isRayFacingUp ? -1 : 0);

		if (mapHasWallAt(xToCheck, yToCheck)) {
			horzWallHitX = nextHorzTouchX;
			horzWallHitY = nextHorzTouchY;
			horzWallContent = map[(int)floor(yToCheck / TILE_SIZE)][(int)floor(xToCheck / TILE_SIZE)];
			foundHorzWallHit = TRUE;
			break;
		} else {
			nextHorzTouchX += xstep;
			nextHorzTouchY += ystep;
		}
	}

	//Vertical ray-grid intersection code.
	int foundVertWallHit = FALSE;
	float vertWallHitX = 0;
	float vertWallHitY = 0;
	int vertWallContent = 0;

	xintercept = floor(player.x / TILE_SIZE) * TILE_SIZE;
	xintercept += isRayFacingRight ? TILE_SIZE : 0;

	yintercept = player.y + (xintercept - player.x) * tan(rayAngle);

	xstep = TILE_SIZE;
	xstep *= isRayFacingLeft ? -1 : 1;

	ystep = TILE_SIZE * tan(rayAngle);
	ystep *= (isRayFacingUp && ystep > 0) ? -1 : 1;
	ystep *= (isRayFacingDown && ystep < 0) ? -1 : 1;

	float nextVertTouchX = xintercept;
	float nextVertTouchY = yintercept;

	while (nextVertTouchX >= 0 && nextVertTouchX <= WINDOW_WIDTH && nextVertTouchY >= 0 && nextVertTouchY <= WINDOW_HEIGHT)
	{
		float xToCheck = nextVertTouchX + (isRayFacingLeft ? -1 : 0);
		float yToCheck = nextVertTouchY;

		if (mapHasWallAt(xToCheck, yToCheck)) {
			vertWallHitX = nextVertTouchX;
			vertWallHitY = nextVertTouchY;
			vertWallContent = map[(int)floor(yToCheck / TILE_SIZE)][(int)floor(xToCheck / TILE_SIZE)];
			foundVertWallHit = TRUE;
			break;
		} else {
			nextVertTouchX += xstep;
			nextVertTouchY += ystep;
		}
	}

	float horzHitDistance = foundHorzWallHit
									? distanceBetweenPoints(player.x, player.y, horzWallHitX, horzWallHitY)
									: FLT_MAX;
	float vertHitDistance = foundVertWallHit
									? distanceBetweenPoints(player.x, player.y, vertWallHitX, vertWallHitY)
									: FLT_MAX;
	if(horzHitDistance > vertHitDistance)
	{
		rays[stripId].distance = vertHitDistance;
		rays[stripId].wallHitX = vertWallHitX;
		rays[stripId].wallHitY = vertWallHitY;
		rays[stripId].wallHitContent = vertWallContent;
		rays[stripId].wasHitVertical = TRUE;
	}
	else
	{
		rays[stripId].distance = horzHitDistance;
		rays[stripId].wallHitX = horzWallHitX;
		rays[stripId].wallHitY = horzWallHitY;
		rays[stripId].wallHitContent = horzWallContent;
		rays[stripId].wasHitVertical = FALSE;
	}
	rays[stripId].rayAngle = rayAngle;
	rays[stripId].isRayFacingDown = isRayFacingDown;
	rays[stripId].isRayFacingUp = isRayFacingUp;
	rays[stripId].isRayFacingLeft = isRayFacingLeft;
	rays[stripId].isRayFacingRight = isRayFacingRight;
}

void castAllRays()
{
	float rayAngle = player.rotationAngle - (FOV_ANGLE / 2);
	for (int stripId = 0; stripId < NUM_RAYS; stripId++) {
		castRay(rayAngle, stripId);

		rayAngle += FOV_ANGLE / NUM_RAYS;
	}
}

void renderMap() {
	 for (int i = 0; i < MAP_NUM_ROWS; i++) {
            for (int j = 0; j < MAP_NUM_COLS; j++) {
                int tileX = j * TILE_SIZE;
                int tileY = i * TILE_SIZE;
                int tileColor = map[i][j] != 0 ? 255 : 0;
				SDL_SetRenderDrawColor(renderer, tileColor, tileColor, tileColor, 255);
				SDL_Rect mapTileRect = {
					MINIMAP_SCALE_FACTOR * tileX,
					MINIMAP_SCALE_FACTOR * tileY,
					MINIMAP_SCALE_FACTOR * TILE_SIZE,
					MINIMAP_SCALE_FACTOR * TILE_SIZE
					};
				SDL_RenderFillRect(renderer, &mapTileRect);
            }
        }
}

void renderRays() {
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	for (int i = 0; i < NUM_RAYS; i++)
	{
		SDL_RenderDrawLine(
			renderer,
			MINIMAP_SCALE_FACTOR * player.x,
			MINIMAP_SCALE_FACTOR * player.y,
			MINIMAP_SCALE_FACTOR * rays[i].wallHitX,
			MINIMAP_SCALE_FACTOR * rays[i].wallHitY
		);
	}
}

void processInput()
{
	SDL_Event event;
	SDL_PollEvent(&event);
	switch (event.type)
	{
		case SDL_QUIT:
			isGameRunning = FALSE;
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
				isGameRunning = FALSE;
			if (event.key.keysym.sym == SDLK_UP)
				player.walkDirection = 1;
			if (event.key.keysym.sym == SDLK_DOWN)
				player.walkDirection = -1;
			if (event.key.keysym.sym == SDLK_RIGHT)
				player.turnDirection = 1;
			if (event.key.keysym.sym == SDLK_LEFT)
				player.turnDirection = -1;
			break;
		case SDL_KEYUP:
			if (event.key.keysym.sym == SDLK_UP)
				player.walkDirection = 0;
			if (event.key.keysym.sym == SDLK_DOWN)
				player.walkDirection = 0;
			if (event.key.keysym.sym == SDLK_RIGHT)
				player.turnDirection = 0;
			if (event.key.keysym.sym == SDLK_LEFT)
				player.turnDirection = 0;
			break;
	}
}

void update()
{
	//Compute how long we have until the reach the target frame time in milliseconds
	int timeToWait = FRAME_TIME_LENGTH - (SDL_GetTicks() - ticksLastFrame);

	// Only delay execution if we are running too fast
	if (timeToWait > 0 && timeToWait <= FRAME_TIME_LENGTH){
		// printf("%d : %f\n", timeToWait, ticksLastFrame);
		SDL_Delay(timeToWait);
	}

	float deltaTime = (SDL_GetTicks() - ticksLastFrame) / 1000.0f;

	ticksLastFrame = SDL_GetTicks();

	movePlayer(deltaTime);

	castAllRays();
}

void generate3Dprojection()
{
	// printf("%f, %f\n", (WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2), (float)WINDOW_HEIGHT);
	for (int i = 0; i < NUM_RAYS; i++)
	{
		//↓見ている視野の角度を一定に保つ。(壁の丸みをなくす)
		float perpDistance = rays[i].distance * cos(rays[i].rayAngle - player.rotationAngle);
		//ProjPlaneを表示してみる。おそらく画面に対する壁の高さの割合を決めている
		float distanceProjPlane = (WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2);
		//↓これが曖昧(おそらく1タイルに対しての、1レーすの距離を画面サイズに拡大してる)
		float projectedWallHeight = (TILE_SIZE / perpDistance) * distanceProjPlane / 2;

		int wallStripHeight = (int)projectedWallHeight;

		int wallTopPixel = (WINDOW_HEIGHT / 2) - (wallStripHeight / 2);
		wallTopPixel = wallTopPixel < 0 ? 0 : wallTopPixel;

		int wallBottomPixel = (WINDOW_HEIGHT / 2) + (wallStripHeight / 2);
		wallBottomPixel = wallBottomPixel > WINDOW_HEIGHT ? WINDOW_HEIGHT : wallBottomPixel;

		// set the color of the ceiling
		for (int y = 0; y < wallTopPixel; y++)
			colorBuffer[(WINDOW_WIDTH * y) + i] = 0xFF333333;

		// calculate textureOffsetX
		int textureOffsetX;
		if(rays[i].wasHitVertical)
		{
			textureOffsetX = (int)rays[i].wallHitY % TEXTURE_HEIGHT;
		} else {
			textureOffsetX = (int)rays[i].wallHitX % TEXTURE_WIDTH;
		}

		int texNum = rays[i].wallHitContent - 1;
	
		// render the wall from wallTopPixel to wallBottomPixel
		for (int y = wallTopPixel; y < wallBottomPixel; y++)
		{
			int distanceFromTop = y + (wallStripHeight / 2) - (WINDOW_HEIGHT / 2);
			//高さを伸縮できる。
			int textureOffsetY = distanceFromTop * ((float)TEXTURE_HEIGHT / wallStripHeight);

			Uint32 texelColor = textures[texNum][(TEXTURE_WIDTH * textureOffsetY) + textureOffsetX];
			colorBuffer[(WINDOW_WIDTH * y) + i] = texelColor;
		}
		// set the color of the floor
		for (int y = wallBottomPixel; y < WINDOW_HEIGHT; y++)
			colorBuffer[(WINDOW_WIDTH * y) + i] = 0xFF777777;

	};
}

void clearColorBuffer(Uint32 color)
{
	for (int x = 0; x < WINDOW_WIDTH; x++)
		for (int y = 0; y < WINDOW_HEIGHT; y++)
			colorBuffer[(WINDOW_WIDTH * y) + x] = color;
}

void renderColorBuffer()
{
	SDL_UpdateTexture(
		colorBufferTexture,
		NULL,
		colorBuffer,
		(int)((Uint32)WINDOW_WIDTH * sizeof(Uint32))
		);
	SDL_RenderCopy(renderer, colorBufferTexture, NULL, NULL);
}

void render()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	generate3Dprojection();

	//clear the color buffer
	renderColorBuffer();
	clearColorBuffer(0xFF000000);

	//display the minimap
	renderMap();
	renderPlayer();
	renderRays();

	SDL_RenderPresent(renderer);
}

void destroyWindow()
{
	free(colorBuffer);
	SDL_DestroyTexture(colorBufferTexture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void setup() {
	player.x = WINDOW_WIDTH / 2;
	player.y = WINDOW_HEIGHT / 2;
	player.width = 5;
	player.height = 5;
	player.turnDirection = 0;
	player.walkDirection = 0;
	player.rotationAngle = PI / 2;
	player.walkSpeed = 100;
	//ラジアン変換
	player.turnSpeed = 45 * (PI / 180);

	colorBuffer = (Uint32*)malloc(sizeof(Uint32) * WINDOW_WIDTH * WINDOW_HEIGHT);
	// create an SDL_Texture to display the colorbuffer
	colorBufferTexture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		WINDOW_WIDTH,
		WINDOW_HEIGHT
	);

	//load some textures from the textures.h
	textures[0] = (Uint32*)REDBRICK_TEXTURE;
	textures[1] = (Uint32*)PURPLESTONE_TEXTURE;
	textures[2] = (Uint32*)MOSSYSTONE_TEXTURE;
	textures[3] = (Uint32*)GRAYSTONE_TEXTURE;
	textures[4] = (Uint32*)COLORSTONE_TEXTURE;
	textures[5] = (Uint32*)BLUESTONE_TEXTURE;
	textures[6] = (Uint32*)WOOD_TEXTURE;
	textures[7] = (Uint32*)EAGLE_TEXTURE;
}

int main(void)
{
	isGameRunning = initializeWindow();
	setup();

	while (isGameRunning)
	{
		processInput();
		update();
		render();
	}
	destroyWindow();
	return (0);
}