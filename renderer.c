#include "renderer.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

Renderer *Renderer_Init(const Camera *camera, const Window *window) {
  Renderer *renderer = (Renderer *)malloc(sizeof(Renderer));
  Canvas *canvas = (Canvas *)malloc(sizeof(Canvas));
  uint32_t *frame_buffer =
      (uint32_t *)calloc(window->width * window->height, sizeof(uint32_t));

  float canvas_top =
      ((camera->film_aperture_height / 2) / camera->focal_length) *
      camera->near_clip_plane;
  float canvas_right =
      canvas_top * (camera->film_aperture_width / camera->film_aperture_height);

  canvas->top = canvas_top;
  canvas->bottom = -canvas_top;
  canvas->right = canvas_right;
  canvas->left = -canvas_right;

  renderer->camera = camera;
  renderer->canvas = canvas;
  renderer->window = window;

  return renderer;
}

void Renderer_Deinit(Renderer *renderer) {
  if (renderer != NULL) {
    free(renderer->canvas);
    free(renderer->frame_buffer);
    free(renderer);
  }
}

Vec2i get_pixel_coords(const Vec3f *vertex, const Renderer *renderer) {
  Vec3f screen_coords = {
      .x = renderer->camera->near_clip_plane * vertex->x / -vertex->z,
      .y = renderer->camera->near_clip_plane * vertex->y / -vertex->z,
      .z = -vertex->z};

  Vec3f ndc_coords = {
      .x = 2 * screen_coords.x /
               (renderer->canvas->right - renderer->canvas->left) -
           (renderer->canvas->right + renderer->canvas->left) /
               (renderer->canvas->right - renderer->canvas->left),
      .y = 2 * screen_coords.y /
               (renderer->canvas->top - renderer->canvas->bottom) -
           (renderer->canvas->top + renderer->canvas->bottom) /
               (renderer->canvas->top - renderer->canvas->bottom),
      .z = screen_coords.z};

  return (Vec2i){
      .x = (int)(((1.0 + ndc_coords.x) / 2.0) * renderer->window->width),
      .y = (int)(((1.0 + ndc_coords.y) / 2.0) * renderer->window->height)};
}

void draw_line(const Vec2i *raster_coords, uint32_t *image_buffer, Vec2i p0,
               Vec2i p1, size_t width) {
  int x0 = p0.x;
  int x1 = p1.x;
  int y0 = p0.y;
  int y1 = p1.y;
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, e2;

  while (1) {
    image_buffer[x0 + y0 * width] = 0x00ff0000;
    if (x0 == x1 && y0 == y1)
      break;
    e2 = 2 * err;
    if (e2 > dy) {
      err += dy;
      x0 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
}

void draw_wire(const OBJ *obj, const Vec2i *raster_coords, Renderer *renderer) {
  for (int i = 0; i < obj->n_faces; i++) {
    Vec3i face = obj->face_indices[i];
    Vec2i v0 = raster_coords[face.x];
    Vec2i v1 = raster_coords[face.y];
    Vec2i v2 = raster_coords[face.z];

    draw_line(raster_coords, renderer->frame_buffer, v0, v1,
              renderer->window->width);
    draw_line(raster_coords, renderer->frame_buffer, v0, v2,
              renderer->window->width);
    draw_line(raster_coords, renderer->frame_buffer, v1, v2,
              renderer->window->width);
  }
}

void draw_at_position(const Color *upper_color, const Color *lower_color,
                      const Vec2i *position) {
  printf("\x1B[%i;%iH", position->y, position->x);
  printf("\x1B[48;2;%i;%i;%im", lower_color->r, lower_color->g, lower_color->b);
  printf("\x1B[38;2;%i;%i;%im", upper_color->r, upper_color->g, upper_color->b);
  printf("%s", "\u2580");
}

void draw_frame(Renderer *renderer) {
  for (size_t i = 0; i < renderer->window->height - 1; i += 2) {
    for (size_t j = 0; j < renderer->window->width; j++) {
      uint32_t rgba_u = renderer->frame_buffer[j + i * renderer->window->width];
      uint32_t rgba_l =
          renderer->frame_buffer[j + (i + 1) * renderer->window->width];
      uint8_t r_u = rgba_u >> 24;
      uint8_t g_u = rgba_u >> 16 & 0xff;
      uint8_t b_u = rgba_u >> 8 & 0xff;
      uint8_t r_l = rgba_l >> 24;
      uint8_t g_l = rgba_l >> 16 & 0xff;
      uint8_t b_l = rgba_l >> 8 & 0xff;

      draw_at_position(&(Color){.r = r_u, .g = g_u, .b = b_u},
                       &(Color){.r = r_l, .g = g_l, .b = b_l},
                       &(Vec2i){.x = j, .y = i / 2});
    }
  }
}

void draw_obj(Renderer *renderer, OBJ *obj) {
  Vec2i *raster_coords = (Vec2i *)malloc(obj->n_vertices * sizeof(Vec2i));

  for (int i = 0; i < obj->n_vertices; i++) {
    raster_coords[i] = get_pixel_coords(&obj->vertices[i], renderer);
  }

  draw_wire(obj, raster_coords, renderer);
}
