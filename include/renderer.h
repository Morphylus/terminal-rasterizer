#pragma once

#include "geometry.h"
#include "vec.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct Camera {
  // All metric
  float film_aperture_height;
  float film_aperture_width;
  float angle_of_view;
  float near_clip_plane;
  float far_clip_plane;
  Vec3f position;
  Vec3f look_at;
  Vec3f up;
} Camera;

typedef struct Canvas {
  float top;
  float bottom;
  float left;
  float right;
} Canvas;

typedef struct Window {
  size_t width;
  size_t height;
} Window;

typedef struct {
  u_int8_t r;
  u_int8_t g;
  u_int8_t b;
} Color;

typedef struct Renderer {
  const Camera *camera;
  const Window *window;
  uint32_t *main_buffer;
  uint32_t *alt_buffer;
  bool is_first_draw;
  Mat4f *world_to_camera;
  Mat4f *projection_matrix;
} Renderer;

Renderer *Renderer_Init(const Camera *camera, const Window *window);
void Renderer_Deinit(Renderer *renderer);
void draw_obj(Renderer *renderer, OBJ *obj);
void set_projection_matrix(Mat4f *mat, const Camera *camera);
void look_at(Mat4f *mat, const Vec3f from, const Vec3f target, const Vec3f up);
