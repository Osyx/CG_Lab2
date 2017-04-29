#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"

using namespace std;
using glm::vec3;
using glm::mat3;

struct Intersection {
	vec3 position;
	float distance;
	int triangleIndex;
};

const int SCREEN_WIDTH = 100;
const int SCREEN_HEIGHT = 100;
SDL_Surface* screen;
int t;
vector<Triangle> triangles;
float focalLength = SCREEN_HEIGHT / 2;
vec3 cameraPos(0, 0, -2);
float m = std::numeric_limits<float>::max();

void update();
void draw();
vec3 intersection(Triangle triangle, vec3 s, vec3 d);
bool closestIntersection(vec3 start, vec3 dir, const vector<Triangle>& triangles, Intersection& closestIntersection);

int main(int argc, char* argv[]) {

	screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT);
	LoadTestModel(triangles);
	t = SDL_GetTicks();	// Set start value for timer.

	while (NoQuitMessageSDL()) {
		update();
		draw();
	}

	SDL_SaveBMP(screen, "screenshot.bmp");
	return 0;
}

void update() {
	// Compute frame time:
	int t2 = SDL_GetTicks();
	float dt = float(t2 - t);
	t = t2;
	cout << "Render time: " << dt << " ms." << endl;
	Uint8* keystate = SDL_GetKeyState(0);
	if (keystate[SDLK_UP]) {
		// Move camera forward
		cameraPos.z += 0.1f;
		cout << "Current focal length: " << cameraPos.z << endl;
	}
	if (keystate[SDLK_DOWN]) {
		// Move camera backward
		cameraPos.z -= 0.1f;
		cout << "Current focal length: " << cameraPos.z << endl;
	}
	if (keystate[SDLK_LEFT]) {
		// Move camera to the left
		cameraPos.x -= 0.1f;
		cout << "Current x: " << cameraPos.x << endl;
	}
	if (keystate[SDLK_RIGHT]) {
		// Move camera to the right
		cameraPos.x += 0.1f;
		cout << "Current x: " << cameraPos.x << endl;
	}
}

void draw() {
	if (SDL_MUSTLOCK(screen))
		SDL_LockSurface(screen);

	for (int y = 0; y < SCREEN_HEIGHT; ++y) {
		for (int x = 0; x < SCREEN_WIDTH; ++x) {
			vec3 d(x - SCREEN_WIDTH / 2, y - SCREEN_HEIGHT / 2, focalLength);
			vec3 color(0, 0, 0);
			Intersection closestInt;
			if (closestIntersection(cameraPos, d, triangles, closestInt)) {
				color = triangles[closestInt.triangleIndex].color;
			}
			PutPixelSDL(screen, x, y, color);
		}
	}

	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);

	SDL_UpdateRect(screen, 0, 0, 0, 0);
}

vec3 intersection(Triangle triangle, vec3 s, vec3 d) {
	vec3 e1 = triangle.v1 - triangle.v0;
	vec3 e2 = triangle.v2 - triangle.v0;
	vec3 b = s - triangle.v0;
	mat3 A(-d, e1, e2);
	return glm::inverse(A) * b;
}

bool closestIntersection(vec3 start, vec3 dir, const vector<Triangle>& triangles, Intersection& closestIntersection) {
	float distance = m;
	bool intersected = false;
	vec3 temp;
	for (size_t i = 0; i < triangles.size(); ++i) {
		temp = intersection(triangles[i], start, dir);
		if (0 <= temp.x && temp.x <= distance && 0 <= temp.y && 0 <= temp.z && temp.y + temp.z <= 1) {
			if (!intersected)
				intersected = true;
			/*distance = sqrt((temp.x - start.x) + (temp.y - start.y) + (temp.z - start.z));*/
			distance = temp.x;
			closestIntersection.position = start + temp.x * dir;
			closestIntersection.distance = glm::distance(start + temp.x * dir, start);
			closestIntersection.triangleIndex = i;
		}
	}
	return intersected;
}

