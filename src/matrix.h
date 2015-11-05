#ifndef MATRIX_H_
#define MATRIX_H_

#include <GL/glew.h>

#define VALUES(vec) vec[0], vec[1], vec[2]
#define MAT4_VALUES(mat) mat[0], mat[1], mat[2], mat[3], mat[4], mat[5], mat[6], mat[7], mat[8], mat[9], mat[10], mat[11], mat[12], mat[13], mat[14], mat[15]
#define MAT4_TRANSPOSED_VALUES(mat) mat[0], mat[4], mat[8], mat[12], mat[1], mat[5], mat[9], mat[13], mat[2], mat[6], mat[10], mat[14], mat[3], mat[7], mat[11], mat[15]
#define PI 3.1415926536

typedef GLfloat mat4[16];
typedef GLfloat vec3[3];
typedef GLfloat vec4[4];

typedef int ivec3[3];

void translate_v3f(vec3 vec, const float x, const float y, const float z);
void translate_v3v(vec3 vec, const vec3 op);

void subtract_v3v(vec3 vec, const vec3 sub);

void rotate_X_v3(vec3 vec, const float radians);
void rotate_Y_v3(vec3 vec, const float radians);
void rotate_Z_v3(vec3 vec, const float radians);

void scale_v3(vec3 vec, const float scale);
void scale_v3f(vec3 vec, const float sx, const float sy, const float sz);
void scale_v3v(vec3 vec, const vec3 scale);

void multiply_v3(vec3 v, const vec3 b);

void normalize_v3(vec3 vec);

void cross_v3(vec3 dest, const vec3 a, const vec3 b);

float dot_v3(const vec3 a, const vec3 b);

void zero_v3(vec3 v);

void multiply_v3_m4(vec3 vec, const mat4 mat, const float w);
void multiply_v4_m4(vec4 v, const mat4 m);

void translate_m4(mat4 mat, const float x, const float y, const float z);
void rotate_X_m4(mat4 mat, const float radians);
void rotate_Y_m4(mat4 mat, const float radians);
void rotate_Z_m4(mat4 mat, const float radians);
void rotate_m4(mat4 mat, const float theta, const float u, const float v, const float w);

void scale_m4(mat4 mat, const float scale);

void identity_m4(mat4 mat);

void translation_m4(mat4 mat, const float x, const float y, const float z);
void rotation_X_m4(mat4 mat, const float radians);
void rotation_Y_m4(mat4 mat, const float radians);
void rotation_Z_m4(mat4 mat, const float radians);
void rotation_m4(mat4 mat, const float theta, const float u, const float v, const float w);

void transpose_m4(mat4 m);

void multiply_m4(mat4 m, const mat4 b);

void perspective(mat4 mat, const float ar, const float fov, const float zNear, const float zFar);

void print_v3(const vec3 vec);
void print_v4(const vec4 vec);
void print_m4(const mat4 mat);
void print_iv3(const ivec3 vec);

void copy_v3(vec3 dest, const vec3 src);
void copy_v4(vec4 dest, const vec4 src);
void copy_m4(mat4 dest, const mat4 src);
void copy_iv3(ivec3 dest, const ivec3 src);

#endif
