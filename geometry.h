#pragma once
#include "vec.h"
#include <stddef.h>

typedef struct {
  Vec3i *face_indices;
  Vec3f *vertices;
  size_t n_vertices;
  size_t n_faces;
} OBJ;
