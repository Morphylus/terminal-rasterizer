#include "geometry.h"

void OBJ_Deinit(OBJ *obj) {
  if (obj != NULL) {
    free(obj->face_indices);
    free(obj->vertices);
    free(obj);
  }
}
