#pragma once
#include "vec.h"
#include <stddef.h>
#include <stdlib.h>

typedef struct {
  Vec3i *face_indices;
  Vec3f *vertices;
  size_t n_vertices;
  size_t n_faces;
} OBJ;

OBJ *OBJ_from_path(const char *path);
void OBJ_Deinit(OBJ *obj);
