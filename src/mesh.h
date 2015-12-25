#ifndef MESH_H_
#define MESH_H_

#define PIXEL_X(x) ((float)(2.0 * (x)) / (float)frame_buffer_width)
#define PIXEL_Y(y) ((float)(2.0 * (y)) / (float)frame_buffer_height)

#define BOX_CORNERS(minx, miny, maxx, maxy, z) minx, miny, z, minx, maxy, z, maxx, miny, z, maxx, maxy, z
#define BOX_INDICES(index) index+3, index+2, index, index+3, index, index+1

#define REP_3(x) x, x, x
#define REP_4(x) x, x, x, x
#define REP_5(x) x, x, x, x, x
#define REP_6(x) x, x, x, x, x, x
#define REP_7(x) x, x, x, x, x, x, x
#define REP_8(x) x, x, x, x, x, x, x, x

#define EMPTY_MESH (Mesh){GL_TRIANGLES, 0, 0, 0, 0, 0, 0, {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}}

#include <GL/glew.h>

#include "matrix.h"

typedef struct Mesh_S {
    GLenum type;

    GLuint vertexvbo;
    GLuint normalvbo;
    GLuint colorvbo;
    GLuint texvbo;
    GLuint buffer;

    int size;

    mat4 modelMatrix;
} Mesh;

void rect(Mesh *mesh, float minx, float miny, float maxx, float maxy, float z, vec3 color);

void makeCrosshair(Mesh *mesh, int width, int height, int border);
void makeSelectionBox(Mesh *mesh, int boxsize, int boxborder, int outerborder, int innerborder);
void makeColorChooser(Mesh *mesh, int chunksx, int chunksy, int chunkwidth, int border);
void makeBlockChooser(Mesh *mesh, int height, int padding, int border);
void makeCubeMapLayout(Mesh *mesh, int width);
void makeSkybox(Mesh *mesh);
void makeBox(Mesh *mesh, float minx, float miny, float minz, float width, float height, float length, vec3 color);

Mesh *createMesh();
void freeMesh(Mesh *mesh);
void buildMesh(Mesh *mesh, GLfloat *points, GLfloat *normals, GLfloat *colors, GLfloat *texuvs, GLuint *indices,
               int spoints, int snormals, int scolors, int stexuvs, int sindices, int nindices);

#endif
