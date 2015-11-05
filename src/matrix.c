#include <math.h>
#include <stdio.h>
#include <string.h>

#include "matrix.h"

void translate_v3f(vec3 vec, const float x, const float y, const float z) {
	vec[0] += x;
	vec[1] += y;
	vec[2] += z;
}

void translate_v3v(vec3 vec, const vec3 op) {
	vec[0] += op[0];
	vec[1] += op[1];
	vec[2] += op[2];
}

void subtract_v3v(vec3 vec, const vec3 sub) {
	vec[0] -= sub[0];
	vec[1] -= sub[1];
	vec[2] -= sub[2];
}

void rotate_X_v3(vec3 vec, const float t) {
	float c = cos(t), s = sin(t);

	vec[0] = vec[0];
	vec[1] = c*vec[1] + s*vec[2];
	vec[2] = -s*vec[1] + c*vec[2];
}

void rotate_Y_v3(vec3 vec, const float t) {
	float c = cos(t), s = sin(t);

	vec[0] = c*vec[0] - s*vec[2];
	vec[1] = vec[1];
	vec[2] = s*vec[0] + c*vec[2];
}

void rotate_Z_v3(vec3 vec, const float t) {
	float c = cos(t), s = sin(t);

	vec[0] = c*vec[0] + s*vec[1];
	vec[1] = -s*vec[0] + c*vec[1];
	vec[2] = vec[2];
}

void scale_v3(vec3 vec, const float scale) {
	vec[0] = vec[0] * scale;
	vec[1] = vec[1] * scale;
	vec[2] = vec[2] * scale;
}

void scale_v3f(vec3 vec, const float sx, const float sy, const float sz) {
	vec[0] = vec[0] * sx;
	vec[1] = vec[1] * sy;
	vec[2] = vec[2] * sz;
}

void scale_v3v(vec3 vec, const vec3 scale) {
	vec[0] = vec[0] * scale[0];
	vec[1] = vec[1] * scale[0];
	vec[2] = vec[2] * scale[0];
}

void multiply_v3(vec3 v, const vec3 b) {
	v[0] *= b[0];
	v[1] *= b[1];
	v[2] *= b[2];
}

void normalize_v3(vec3 vec) {
	float len = sqrt(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);

	vec[0] /= len;
	vec[1] /= len;
	vec[2] /= len;
}

void cross_v3(vec3 dest, const vec3 a, const vec3 b) {
	dest[0] = a[1]*b[2] - a[2]*b[1];
	dest[1] = a[2]*b[0] - a[0]*b[2];
	dest[2] = a[0]*b[1] - a[1]*b[0];
}

float dot_v3(const vec3 a, const vec3 b) {
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

void zero_v3(vec3 v) {
	v[0] = 0.0;
	v[1] = 0.0;
	v[2] = 0.0;
}

void multiply_v3_m4(vec3 v, const mat4 m, const float w) {
	vec3 n = {
		v[0]*m[ 0] + v[1]*m[ 1] + v[2]*m[ 2] + w*m[ 3],
		v[0]*m[ 4] + v[1]*m[ 5] + v[2]*m[ 6] + w*m[ 7],
		v[0]*m[ 8] + v[1]*m[ 9] + v[2]*m[10] + w*m[11]
	};

	copy_v3(v, n);
}

void multiply_v4_m4(vec4 v, const mat4 m) {
	vec4 n = {
		v[0]*m[ 0] + v[1]*m[ 1] + v[2]*m[ 2] + v[3]*m[ 3],
		v[0]*m[ 4] + v[1]*m[ 5] + v[2]*m[ 6] + v[3]*m[ 7],
		v[0]*m[ 8] + v[1]*m[ 9] + v[2]*m[10] + v[3]*m[11],
		v[0]*m[12] + v[1]*m[13] + v[2]*m[14] + v[3]*m[15]
	};

	copy_v4(v, n);
}

void translate_m4(mat4 m, const float x, const float y, const float z) {
	mat4 n = {
		m[ 0]+x*m[12], 	m[ 1]+x*m[13], 	m[ 2]+x*m[14], 	m[ 3]+x*m[15],
		m[ 4]+y*m[12], 	m[ 5]+y*m[13], 	m[ 6]+y*m[14], 	m[ 7]+y*m[15],
		m[ 8]+z*m[12], 	m[ 9]+z*m[13], 	m[10]+z*m[14], 	m[11]+z*m[15],
		        m[12], 	        m[13], 	        m[14], 	        m[15]
	};

	copy_m4(m, n);
}

void translation_m4(mat4 mat, const float x, const float y, const float z) {
	mat4 t = {
		1, 	0, 	0, 	x,
		0, 	1, 	0, 	y,
		0, 	0, 	1, 	z,
		0, 	0, 	0, 	1
	};

	copy_m4(mat, t);
}

void rotate_X_m4(mat4 m, const float t) {
	float c = cos(t);
	float s = sin(t);

	mat4 n = {
		  m[ 0], 			  m[ 1], 			  m[ 2], 			  m[ 3],
		c*m[ 4] - s*m[ 8], 	c*m[ 5] - s*m[ 9], 	c*m[ 6] - s*m[10], 	c*m[ 7] - s*m[11],
		s*m[ 4] + c*m[ 8], 	s*m[ 5] + c*m[ 9], 	s*m[ 6] + c*m[10], 	s*m[ 7] + c*m[11],
		  m[12], 			  m[13], 			  m[14], 			  m[15]
	};

	copy_m4(m, n);
}

void rotate_Y_m4(mat4 m, const float t) {
	float c = cos(t);
	float s = sin(t);

	mat4 n = {
		 c*m[ 0] + s*m[ 8],  c*m[ 1] + s*m[ 9],  c*m[ 2] + s*m[10],  c*m[ 3] + s*m[11],
		   m[ 4], 			   m[ 5], 			   m[ 6], 			   m[ 7],
		-s*m[ 0] + c*m[ 8], -s*m[ 1] + c*m[ 9], -s*m[ 2] + c*m[10], -s*m[ 3] + c*m[11],
		   m[12], 			   m[13], 			   m[14], 			   m[15]
	};

	copy_m4(m, n);
}

void rotate_Z_m4(mat4 m, const float t) {
	float c = cos(t);
	float s = sin(t);

	mat4 n = {
		c*m[ 0] - s*m[ 4], 	c*m[ 1] - s*m[ 5], 	c*m[ 2] - s*m[ 6], 	c*m[ 3] - s*m[ 7],
		s*m[ 0] + c*m[ 4], 	s*m[ 1] + c*m[ 5], 	s*m[ 2] + c*m[ 6], 	s*m[ 3] + c*m[ 7],
		  m[ 8], 			  m[ 9], 			  m[10], 			  m[11],
		  m[12], 			  m[13], 			  m[14], 			  m[15],
	};

	copy_m4(m, n);
}

void rotate_m4(mat4 m, const float theta, const float u, const float v, const float w) {
	float u2 = u*u, v2 = v*v, w2 = w*w;
	float l = u2 + v2 + w2;
	float sl = sqrt(l);
	float s = sin(theta);
	float c = cos(theta), ic = 1-c;

	float uvic = u*v*ic;
	float vwic = v*w*ic;
	float uwic = u*w*ic;
	float usls = u*sl*s;
	float vsls = v*sl*s;
	float wsls = w*sl*s;

	mat4 r = {
		(u2+(v2+w2)*c)/l, 	(uvic - wsls)/l, 	(uwic + vsls)/l, 	0,
		(uvic + wsls)/l, 	(v2+(u2+w2)*c)/l, 	(vwic - usls)/l, 	0,
		(uwic - vsls)/l, 	(vwic + usls)/l, 	(w2+(u2+v2)*c)/l, 	0,
		0, 					0, 					0, 					1
	};

	mat4 n = {
		r[ 0]*m[ 0] + r[ 1]*m[ 4] + r[ 2]*m[ 8],
		r[ 0]*m[ 1] + r[ 1]*m[ 5] + r[ 2]*m[ 9],
		r[ 0]*m[ 2] + r[ 1]*m[ 6] + r[ 2]*m[10],
		r[ 0]*m[ 3] + r[ 1]*m[ 7] + r[ 2]*m[11],

		r[ 4]*m[ 0] + r[ 5]*m[ 4] + r[ 6]*m[ 8],
		r[ 4]*m[ 1] + r[ 5]*m[ 5] + r[ 6]*m[ 9],
		r[ 4]*m[ 2] + r[ 5]*m[ 6] + r[ 6]*m[10],
		r[ 4]*m[ 3] + r[ 5]*m[ 7] + r[ 6]*m[11],

		r[ 8]*m[ 0] + r[ 9]*m[ 4] + r[10]*m[ 8],
		r[ 8]*m[ 1] + r[ 9]*m[ 5] + r[10]*m[ 9],
		r[ 8]*m[ 2] + r[ 9]*m[ 6] + r[10]*m[10],
		r[ 8]*m[ 3] + r[ 9]*m[ 7] + r[10]*m[11],

		m[12],
		m[13],
		m[14],
		m[15]
	};

	copy_m4(m, n);
}

void scale_m4(mat4 m, const float s) {
	mat4 n = {
		s*m[ 0], 	s*m[ 1], 	s*m[ 2], 	s*m[ 3],
		s*m[ 4], 	s*m[ 5], 	s*m[ 6], 	s*m[ 7],
		s*m[ 8], 	s*m[ 9], 	s*m[10], 	s*m[11],
		m[12], 		m[13], 		m[14], 		m[15]
	};

	copy_m4(m, n);
}

void identity_m4(mat4 mat) {
	mat4 i = {
		1, 	0, 	0, 	0,
		0, 	1, 	0, 	0,
		0, 	0, 	1, 	0,
		0,  0,  0,  1
	};

	copy_m4(mat, i);
}

void rotation_X_m4(mat4 mat, const float t) {
	float c = cos(t);
	float s = sin(t);

	mat4 r = {
		1, 	0, 	0, 	0,
		0, 	c, 	-s, 0,
		0, 	s, 	c, 	0,
		0,  0,  0, 	1
	};

	copy_m4(mat, r);
}

void rotation_Y_m4(mat4 mat, const float t) {
	float c = cos(t);
	float s = sin(t);

	mat4 r = {
		c, 	0, 	s, 0,
		0, 	1, 	0, 0,
		-s, 0, 	c, 0,
		0,  0, 	0, 1
	};

	copy_m4(mat, r);
}

void rotation_Z_m4(mat4 mat, const float t) {
	float c = cos(t);
	float s = sin(t);

	mat4 r = {
		c, 	-s,	0, 	0,
		s, 	c, 	0, 	0,
		0, 	0, 	1, 	0,
		0,  0,  0, 	1
	};

	copy_m4(mat, r);
}

void rotation_m4(mat4 mat, const float theta, const float u, const float v, const float w) {
	float u2 = u*u, v2 = v*v, w2 = w*w;
	float l = u2 + v2 + w2;
	float sl = sqrt(l);
	float s = sin(theta);
	float c = cos(theta), ic = 1-c;

	mat4 r = {
		(u2+(v2+w2)*c)/l, 	(u*v*ic-w*sl*s)/l, 	(u*w*ic+v*sl*s)/l, 	0,
		(u*v*ic+w*sl*s)/l, 	(v2+(u2+w2)*c)/l, 	(v*w*ic-u*sl*s)/l, 	0,
		(u*w*ic-v*sl*s)/l, 	(v*w*ic+u*sl*s)/l, 	(w2+(u2+v2)*c)/l, 	0,
		0, 					0, 					0, 					1
	};

	copy_m4(mat, r);
}

void transpose_m4(mat4 m) {
	mat4 n = {
		m[ 0], m[ 4], m[ 8], m[12],
		m[ 1], m[ 5], m[ 9], m[13],
		m[ 2], m[ 6], m[10], m[14],
		m[ 3], m[ 7], m[11], m[15]
	};

	copy_m4(m, n);
}

void multiply_m4(mat4 m, const mat4 b) {
	mat4 n = {
		b[ 0]*m[ 0] + b[ 1]*m[ 4] + b[ 2]*m[ 8] + b[ 3]*m[12],
		b[ 0]*m[ 1] + b[ 1]*m[ 5] + b[ 2]*m[ 9] + b[ 3]*m[13],
		b[ 0]*m[ 2] + b[ 1]*m[ 6] + b[ 2]*m[10] + b[ 3]*m[14],
		b[ 0]*m[ 3] + b[ 1]*m[ 7] + b[ 2]*m[11] + b[ 3]*m[15],

		b[ 4]*m[ 0] + b[ 5]*m[ 4] + b[ 6]*m[ 8] + b[ 7]*m[12],
		b[ 4]*m[ 1] + b[ 5]*m[ 5] + b[ 6]*m[ 9] + b[ 7]*m[13],
		b[ 4]*m[ 2] + b[ 5]*m[ 6] + b[ 6]*m[10] + b[ 7]*m[14],
		b[ 4]*m[ 3] + b[ 5]*m[ 7] + b[ 6]*m[11] + b[ 7]*m[15],

		b[ 8]*m[ 0] + b[ 9]*m[ 4] + b[10]*m[ 8] + b[11]*m[12],
		b[ 8]*m[ 1] + b[ 9]*m[ 5] + b[10]*m[ 9] + b[11]*m[13],
		b[ 8]*m[ 2] + b[ 9]*m[ 6] + b[10]*m[10] + b[11]*m[14],
		b[ 8]*m[ 3] + b[ 9]*m[ 7] + b[10]*m[11] + b[11]*m[15],

		b[12]*m[ 0] + b[13]*m[ 4] + b[14]*m[ 8] + b[15]*m[12],
		b[12]*m[ 1] + b[13]*m[ 5] + b[14]*m[ 9] + b[15]*m[13],
		b[12]*m[ 2] + b[13]*m[ 6] + b[14]*m[10] + b[15]*m[14],
		b[12]*m[ 3] + b[13]*m[ 7] + b[14]*m[11] + b[15]*m[15]
	};

	copy_m4(m, n);
}

void perspective(mat4 mat, const float ar, const float fov, const float zNear, const float zFar) {
	float r = zNear - zFar;
	float f = 1.0 / tan(fov * PI / 360);

	mat4 p = {
		f/ar, 	0, 	0, 					0,
		0, 		f, 	0, 					0,
		0, 		0, 	-(zFar+zNear)/r, 	(2*zFar*zNear)/r,
		0,  	0,  1, 					0
	};

	copy_m4(mat, p);
}

void print_v3(const vec3 v) {
	int i;

	for (i = 0; i < 3; i++) {
			printf("%.2f \t", v[i]);
	}
	puts("\n");
}

void print_v4(const vec4 v) {
	int i;

	for (i = 0; i < 4; i++) {
			printf("%.2f \t", v[i]);
	}
	puts("\n");
}

void print_m4(const mat4 m) {
	int i, j;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			printf("%.2f \t", m[i*4+j]);
		}
		puts("");
	}
	puts("");
}

void print_iv3(const ivec3 v) {
	int i;

	for (i = 0; i < 3; i++) {
			printf("%d \t", v[i]);
	}
	puts("\n");
}

void copy_v3(vec3 dest, const vec3 src) {
	memcpy(dest, src, sizeof(vec3));
}

void copy_v4(vec4 dest, const vec4 src) {
	memcpy(dest, src, sizeof(vec4));
}

void copy_m4(mat4 dest, const mat4 src) {
	memcpy(dest, src, sizeof(mat4));
}

void copy_iv3(ivec3 dest, const ivec3 src) {
	memcpy(dest, src, sizeof(ivec3));
}