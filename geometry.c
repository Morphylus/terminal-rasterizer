#include "geometry.h"
#include "vec.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define START_CAPACITY 8
#define LINE_BUFFER_CAPACITY 32

void OBJ_Deinit(OBJ *obj) {
  if (obj != NULL) {
    free(obj->face_indices);
    free(obj->vertices);
    free(obj);
  }
}

OBJ *OBJ_from_path(const char *path) {
  FILE *fp = fopen(path, "r");
  char line[LINE_BUFFER_CAPACITY];

  float x, y, z;
  uint32_t v0, v1, v2;
  char t;

  Vec3f *vertices = (Vec3f *)malloc(START_CAPACITY * sizeof(Vec3f));
  Vec3i *faces = (Vec3i *)malloc(START_CAPACITY * sizeof(Vec3i));

  size_t n_vertices = 0;
  size_t n_faces = 0;
  size_t size_v = START_CAPACITY;
  size_t size_f = START_CAPACITY;

  if (fp != NULL) {
    while (fgets(line, sizeof(line), fp)) {
      if (line[0] == '\n')
        continue;

      sscanf(line, "%s", &t);

      if (t == 'v') {
        sscanf(line, "%*s %f %f %f %*s", &x, &y, &z);
        if (n_vertices == size_v) {
          vertices = (Vec3f *)realloc(vertices, (size_v *= 2) * sizeof(Vec3f));
        }
        vertices[n_vertices++] = (Vec3f){x, y, z};
      }

      if (t == 'f') {
        sscanf(line, "%*s %i %i %i %*s", &v0, &v1, &v2);
        if (n_faces == size_f) {
          faces = (Vec3i *)realloc(faces, (size_f *= 2) * sizeof(Vec3i));
        }
        faces[n_faces++] = (Vec3i){v0, v1, v2};
      }
    }
  }

  OBJ *obj = (OBJ *)malloc(sizeof(OBJ));
  obj->vertices = vertices;
  obj->face_indices = faces;
  obj->n_vertices = n_vertices;
  obj->n_faces = n_faces;

  return obj;
}
