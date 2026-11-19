#pragma once

#include "geometry.h"
#include "vec.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct Camera {
  // All metric
  float film_aperture_height;
  float film_aperture_width;
  float focal_length;
  float near_clip_plane;
  size_t window_width, window_height;
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
  Canvas *canvas;
  uint32_t *frame_buffer;
} Renderer;

Renderer *Renderer_Init(const Camera *camera, const Window *window);
void Renderer_Deinit(Renderer *renderer);
void draw_obj(Renderer *renderer, OBJ *obj);
