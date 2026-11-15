#pragma once
#include <stdint.h>

typedef struct {
  float x;
  float y;
  float z;
} Vec3f;

typedef struct {
  uint32_t x;
  uint32_t y;
  uint32_t z;
} Vec3i;

typedef struct {
  float x;
  float y;
} Vec2f;

typedef struct {
  uint32_t x;
  uint32_t y;
} Vec2i;

Vec3f vec_add(const Vec3f a, const Vec3f b);
