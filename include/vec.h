#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  float x;
  float y;
  float z;
} Vec3f;

typedef struct {
  int32_t x;
  int32_t y;
  int32_t z;
} Vec3i;

typedef struct {
  float x;
  float y;
} Vec2f;

typedef struct {
  int32_t x;
  int32_t y;
} Vec2i;

typedef float Vec4f[4];

typedef struct {
  Vec4f m[4];
} Mat4f;

// Row major ordering
bool mat4f_inv(Mat4f *out, const Mat4f *in);
void mul_mat_vec4f(Vec4f *out, const Mat4f *mat, const Vec4f *vec);
Vec3f normalize(const Vec3f in);
Vec3f sub(const Vec3f a, const Vec3f b);
Vec3f cross(const Vec3f a, const Vec3f b);
Mat4f rotation_matrix(const Vec3f r, const Vec3f pos);
