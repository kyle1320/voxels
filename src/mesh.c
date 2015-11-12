#include <stdlib.h>
#include <stdio.h>

#include "main.h"
#include "mesh.h"
#include "color.h"
#include "matrix.h"

extern int frame_buffer_width;
extern int frame_buffer_height;

void rect(Mesh *mesh, float minx, float miny, float maxx, float maxy, float z, vec3 color) {
    GLfloat points[] = { BOX_CORNERS(minx, miny, maxx, maxy, z) };
    GLfloat normals[] ={ REP_4(REP_3(0)) };
    GLfloat colors[] = { REP_4(VALUES(color)) };
    GLuint indices[] = { BOX_INDICES(0) };

    buildMesh(mesh, points, normals, colors, NULL, indices,
              sizeof(points), sizeof(normals),
              sizeof(colors), 0, sizeof(indices),
              (sizeof(indices) / sizeof(GLuint)));
}

void makeCrosshair(Mesh *mesh, int width, int height, int border) {
    GLfloat x[8] = {
        PIXEL_X(-height), PIXEL_X(-width), PIXEL_X(width), PIXEL_X(height),
        PIXEL_X(-height-border), PIXEL_X(-width-border), PIXEL_X(width+border), PIXEL_X(height+border),
    };

    GLfloat y[8] = {
        PIXEL_Y(-height), PIXEL_Y(-width), PIXEL_Y(width), PIXEL_Y(height),
        PIXEL_Y(-height-border), PIXEL_Y(-width-border), PIXEL_Y(width+border), PIXEL_Y(height+border),
    };

    GLfloat points[] = {
        BOX_CORNERS(x[0], y[1], x[3], y[2], 0),
        BOX_CORNERS(x[1], y[0], x[2], y[3], 0),

        BOX_CORNERS(x[4], y[5], x[7], y[6], 0),
        BOX_CORNERS(x[5], y[4], x[6], y[7], 0),
    };

    GLfloat normals[] = {
        REP_4(REP_4(REP_3(0)))
    };

    GLfloat colors[] = {
        REP_8(REP_3(1)),
        REP_8(REP_3(0))
    };

    GLuint indices[] = {
        BOX_INDICES(8),
        BOX_INDICES(12),
        BOX_INDICES(0),
        BOX_INDICES(4),
    };

    buildMesh(mesh, points, normals, colors, NULL, indices,
              sizeof(points), sizeof(normals),
              sizeof(colors), 0, sizeof(indices),
              (sizeof(indices) / sizeof(GLuint)));
}

void makeColorChooser(Mesh *mesh, int chunksx, int chunksy, int chunkwidth, int border) {
    int x, y;
    int ptsx = chunksx + 1;
    int ptsy = chunksy + 1;
    float chunkwidthX = PIXEL_X(chunkwidth);
    float chunkwidthY = PIXEL_Y(chunkwidth);
    //float borderwidthX = PIXEL_X(border);
    float borderwidthY = PIXEL_Y(border);

    GLfloat normals[] = {
        REP_4(REP_3(0)),
        REP_7(REP_7(REP_3(REP_3(0)))),
    };

    int npoints = ptsx * ptsy * 3;
    int nindices = chunksx * chunksy * 6;

    GLfloat *points = malloc(npoints * sizeof(GLfloat));
    GLfloat *colors = malloc(npoints * sizeof(GLfloat));
    GLuint *indices = malloc(nindices * sizeof(GLuint));

    Color c;
    for (x = 0; x <= chunksx; x++) {
        for (y = 0; y <= chunksy; y++) {
            points[(y * ptsx + x) * 3 + 0] = x * chunkwidthX;
            points[(y * ptsx + x) * 3 + 1] = y * chunkwidthY + borderwidthY;
            points[(y * ptsx + x) * 3 + 2] = 0;

            c = getCoordinateColor((float)x / ptsx, (float)(ptsy - y) / ptsy);
            colors[(y * ptsx + x) * 3 + 0] = (float)c.r / 255;
            colors[(y * ptsx + x) * 3 + 1] = (float)c.g / 255;
            colors[(y * ptsx + x) * 3 + 2] = (float)c.b / 255;

            if (x < chunksx && y < chunksy) {
                indices[(y * chunksx + x) * 6 + 0] = (y * ptsx + x + ptsx + 1);
                indices[(y * chunksx + x) * 6 + 1] = (y * ptsx + x + 1);
                indices[(y * chunksx + x) * 6 + 2] = (y * ptsx + x);
                indices[(y * chunksx + x) * 6 + 3] = (y * ptsx + x + ptsx + 1);
                indices[(y * chunksx + x) * 6 + 4] = (y * ptsx + x);
                indices[(y * chunksx + x) * 6 + 5] = (y * ptsx + x + ptsx);
            }
        }
    }

    buildMesh(mesh, points, normals, colors, NULL, indices,
              npoints * sizeof(GLfloat), sizeof(normals),
              npoints * sizeof(GLfloat), 0, nindices * sizeof(GLuint),
              nindices);

    free(points);
    free(colors);
    free(indices);
}

void makeCubeMapLayout(Mesh *mesh, int width) {
    float sizex = PIXEL_X(width);
    float sizey = PIXEL_Y(width);

    GLfloat x[] = {
        0, sizex, sizex*2, sizex*3, sizex*4
    };

    GLfloat y[] = {
        0, sizey, sizey*2, sizey*3
    };

    GLfloat points[] = {
        BOX_CORNERS(x[0], y[1], x[1], y[2], 0),
        BOX_CORNERS(x[1], y[1], x[2], y[2], 0),
        BOX_CORNERS(x[2], y[1], x[3], y[2], 0),
        BOX_CORNERS(x[3], y[1], x[4], y[2], 0),

        BOX_CORNERS(x[1], y[0], x[2], y[1], 0),
        BOX_CORNERS(x[1], y[2], x[2], y[3], 0),
    };

    GLfloat normals[] = {
        REP_6(REP_6(REP_3(0)))
    };

    GLfloat colors[] = {
        REP_6(REP_6(REP_3(1))),
    };

    GLfloat texuvs[] = {
        -1,  1, -1, -1, -1, -1, -1,  1,  1, -1, -1,  1,
        -1,  1,  1, -1, -1,  1,  1,  1,  1,  1, -1,  1,
         1,  1,  1,  1, -1,  1,  1,  1, -1,  1, -1, -1,
         1,  1, -1,  1, -1, -1, -1,  1, -1, -1, -1, -1,

        -1, -1,  1, -1, -1, -1,  1, -1,  1,  1, -1, -1,
        -1,  1, -1, -1,  1,  1,  1,  1, -1,  1,  1,  1,
    };

    GLuint indices[] = {
        BOX_INDICES(0),
        BOX_INDICES(4),
        BOX_INDICES(8),
        BOX_INDICES(12),
        BOX_INDICES(16),
        BOX_INDICES(20),
    };

    /*GLfloat points[] = { BOX_CORNERS(0, 0, sizex, sizey, 0) };
    GLfloat normals[]= { REP_6(REP_3(0)) };
    GLfloat colors[] = { REP_6(REP_3(1)) };
    GLfloat texuvs[] = { BOX_CORNERS(0, 0, 1, 1, 0) };
    GLuint indices[] = { BOX_INDICES(0) };*/

    buildMesh(mesh, points, normals, colors, texuvs, indices,
              sizeof(points), sizeof(normals),
              sizeof(colors), sizeof(texuvs),
              sizeof(indices),
              sizeof(indices) / sizeof(GLuint));
}

void makeBox(Mesh *mesh, float minx, float miny, float minz, float width, float height, float length, vec3 color) {
    float maxx = minx + width;
    float maxy = miny + height;
    float maxz = minz + length;

    GLfloat points[] = {
        maxx, maxy, maxz, maxx, miny, maxz, maxx, miny, minz, maxx, maxy, maxz, maxx, miny, minz, maxx, maxy, minz,
        maxx, maxy, maxz, maxx, maxy, minz, minx, maxy, minz, maxx, maxy, maxz, minx, maxy, minz, minx, maxy, maxz,
        maxx, maxy, maxz, minx, maxy, maxz, minx, miny, maxz, maxx, maxy, maxz, minx, miny, maxz, maxx, miny, maxz,
        minx, miny, minz, minx, miny, maxz, minx, maxy, maxz, minx, miny, minz, minx, maxy, maxz, minx, maxy, minz,
        minx, miny, minz, maxx, miny, minz, maxx, miny, maxz, minx, miny, minz, maxx, miny, maxz, minx, miny, maxz,
        minx, miny, minz, minx, maxy, minz, maxx, maxy, minz, minx, miny, minz, maxx, maxy, minz, maxx, miny, minz,
    };

    GLfloat normals[] = {
         1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,
         0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,
         0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,
        -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0,
         0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,
         0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,
    };

    GLfloat colors[] = { REP_6(REP_6(VALUES(color))) };

    buildMesh(mesh, points, normals, colors, NULL, NULL,
              sizeof(points), sizeof(normals),
              sizeof(colors), 0, 0, 36);
}

void makeSkybox(Mesh *mesh) {
    GLfloat points[] = { BIN_3(-1, 1) };
    GLfloat normals[] ={ REP_8(REP_3(0)) };
    GLfloat colors[] = { REP_8(REP_3(1)) };
    //GLfloat texuvs[] = { BIN_3(-1, 1) };
    GLuint indices[] = { 7, 4, 5, 7, 6, 4, 7, 2, 6, 7, 3, 2, 7, 1, 3, 7, 5, 1, 0, 3, 1, 0, 2, 3, 0, 5, 4, 0, 1, 5, 0, 6, 2, 0, 4, 6 };

    buildMesh(mesh, points, normals, colors, NULL, indices,
              sizeof(points), sizeof(normals),
              sizeof(colors), 0,
              sizeof(indices),
              sizeof(indices) / sizeof(GLuint));
}

void freeMesh(Mesh *mesh) {
    glDeleteBuffers(1, &mesh->vertexvbo);
    glDeleteBuffers(1, &mesh->normalvbo);
    glDeleteBuffers(1, &mesh->colorvbo);
    glDeleteBuffers(1, &mesh->texvbo);
    glDeleteBuffers(1, &mesh->buffer);
}

void buildMesh(Mesh *mesh, GLfloat *points, GLfloat *normals, GLfloat *colors, GLfloat *texuvs, GLuint *indices,
               int spoints, int snormals, int scolors, int stexuvs, int sindices, int nindices) {
    GLuint bufs[5] = {0, 0, 0, 0, 0};

    glGenBuffers(3, bufs);

    glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
    glBufferData(GL_ARRAY_BUFFER, spoints, points, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, bufs[1]);
    glBufferData(GL_ARRAY_BUFFER, snormals, normals, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, bufs[2]);
    glBufferData(GL_ARRAY_BUFFER, scolors, colors, GL_STATIC_DRAW);

    if (texuvs) {
        glGenBuffers(1, &bufs[3]);
        glBindBuffer(GL_ARRAY_BUFFER, bufs[3]);
        glBufferData(GL_ARRAY_BUFFER, stexuvs, texuvs, GL_STATIC_DRAW);
    }

    if (indices) {
        glGenBuffers(1, &bufs[4]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufs[4]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sindices, indices, GL_STATIC_DRAW);
    }

    *mesh = (Mesh){GL_TRIANGLES, bufs[0], bufs[1], bufs[2], bufs[3], bufs[4], nindices};
    identity_m4(mesh->modelMatrix);
}
