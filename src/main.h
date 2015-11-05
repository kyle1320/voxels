#ifndef TEST_H_
#define TEST_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "matrix.h"
#include "mesh.h"
#include "voxels.h"

void drawMesh(Mesh * mesh);

void init(GLFWwindow *window);
void tick(GLFWwindow *window);
void render();
void finish();

int main(void);

#endif
