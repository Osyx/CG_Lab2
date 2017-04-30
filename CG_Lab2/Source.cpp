#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"
#include <algorithm>
#include <cmath>

//using namespace std;

using glm::vec3;
using glm::mat3;

struct Intersection {
	vec3 position;
	float distance;
	int triangleIndex;
};

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
float focalLength = SCREEN_HEIGHT / 2;
float m = std::numeric_limits<float>::max();
float PI = acos(-1);
float yaw = 0;
int t;
mat3 R = mat3(vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
SDL_Surface* screen;
vec3 cameraPos(0, 0, -2);
vec3 right(R[0][0], R[0][1], R[0][2]);
vec3 down(R[1][0], R[1][1], R[1][2]);
vec3 forward(R[2][0], R[2][1], R[2][2]);
vec3 indirectLightColor = 0.5f * vec3(1, 1, 1);
vec3 lightPos(0, -0.5, -0.7);
vec3 lightColor = 14.f * vec3(1, 1, 1);
std::vector<Triangle> triangles;

bool closestIntersection(vec3 start, vec3 dir, const std::vector<Triangle>& triangles, Intersection& closestIntersection);
vec3 directLight(const Intersection& i);
void draw();
vec3 indirectLight(const Intersection& i);
vec3 intersection(Triangle triangle, vec3 s, vec3 d);
void rotateCamera();
void update();


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

bool closestIntersection(vec3 start, vec3 dir, const std::vector<Triangle>& triangles, Intersection& closestIntersection) {
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

vec3 directLight(const Intersection& i) {
	vec3 n_hat = triangles[i.triangleIndex].normal;
	vec3 r_hat = glm::normalize(lightPos - i.position);
	float r = glm::length(lightPos - i.position);

	Intersection shadow;
	vec3 shadowDir = glm::normalize(i.position - lightPos);
	if (closestIntersection(lightPos, shadowDir, triangles, shadow)) {
		// Without this if statement, there seems to be some sort of rounding error, that causes everything to be shadows.
		if (shadow.distance < r - 0.00001f) {
			return vec3(0, 0, 0);
		}
	}

	vec3 returnVec = lightColor * std::max(glm::dot(r_hat, n_hat), 0.0f) / (4.0f * PI * pow(r, 2.0f));
	return returnVec;
}


void draw() {
	if (SDL_MUSTLOCK(screen))
		SDL_LockSurface(screen);

	for (int y = 0; y < SCREEN_HEIGHT; ++y) {
		for (int x = 0; x < SCREEN_WIDTH; ++x) {
			Intersection closestInt;
			vec3 color(0, 0, 0);
			vec3 d(x - SCREEN_WIDTH / 2, y - SCREEN_HEIGHT / 2, focalLength);
			d = R * d;
			if (closestIntersection(cameraPos, d, triangles, closestInt)) {
				color = triangles[closestInt.triangleIndex].color * indirectLight(closestInt);
			}
			PutPixelSDL(screen, x, y, color);
		}
	}

	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);

	SDL_UpdateRect(screen, 0, 0, 0, 0);
}

vec3 indirectLight(const Intersection& i) {
	return directLight(i) + indirectLightColor;
}

vec3 intersection(Triangle triangle, vec3 s, vec3 d) {
	vec3 e1 = triangle.v1 - triangle.v0;
	vec3 e2 = triangle.v2 - triangle.v0;
	vec3 b = s - triangle.v0;
	mat3 A(-d, e1, e2);
	return glm::inverse(A) * b;
}

void rotateCamera() {
	vec3 a(cos(yaw), 0, -sin(yaw));
	vec3 b(0, 1, 0);
	vec3 c(sin(yaw), 0, cos(yaw));
	R = mat3(a, b, c);
}

void update() {
	// Compute frame time:
	int t2 = SDL_GetTicks();
	float dt = float(t2 - t);
	t = t2;
	std::cout << "Render time: " << dt << " ms." << std::endl;
	Uint8* keystate = SDL_GetKeyState(0);
	if (keystate[SDLK_UP]) {
		cameraPos += vec3(0, 0, 0.2f);
	}
	if (keystate[SDLK_DOWN]) {
		cameraPos -= vec3(0, 0, 0.2f);
	}
	if (keystate[SDLK_LEFT]) {
		yaw -= 0.03f;
		rotateCamera();
	}
	if (keystate[SDLK_RIGHT]) {
		yaw += 0.03f;
		rotateCamera();
	}
	if (keystate[SDLK_w]) {
		lightPos += forward;
	}
	if (keystate[SDLK_a]) {
		lightPos -= right;
	}
	if (keystate[SDLK_s]) {
		lightPos -= forward;
	}
	if (keystate[SDLK_d]) {
		lightPos += right;
	}
	if (keystate[SDLK_q]) {
		lightPos -= down;
	}
	if (keystate[SDLK_e]) {
		lightPos += down;
	}
}

