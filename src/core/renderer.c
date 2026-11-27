#include "renderer.h"
#include "vec.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void world_to_camera(Mat4f *mat, const Camera *camera) {
  Mat4f transform;

  look_at(&transform, camera->position, camera->look_at, camera->up);
  mat4f_inv(mat, &transform);
}

Renderer *Renderer_Init(const Camera *camera, const Window *window) {
  Renderer *renderer = (Renderer *)malloc(sizeof(Renderer));
  Mat4f *world_to_camera = (Mat4f *)calloc(1, sizeof(Mat4f));
  Mat4f *projection_matrix = (Mat4f *)calloc(1, sizeof(Mat4f));
  uint32_t *frame_buffer_a =
      (uint32_t *)calloc(window->width * window->height, sizeof(uint32_t));
  uint32_t *frame_buffer_b =
      (uint32_t *)calloc(window->width * window->height, sizeof(uint32_t));

  renderer->camera = camera;
  renderer->is_first_draw = true;
  renderer->world_to_camera = world_to_camera;
  renderer->projection_matrix = projection_matrix;
  renderer->main_buffer = frame_buffer_a;
  renderer->alt_buffer = frame_buffer_b;
  renderer->window = window;

  return renderer;
}

void Renderer_Deinit(Renderer *renderer) {
  if (renderer != NULL) {
    free(renderer->main_buffer);
    free(renderer->alt_buffer);
    free(renderer->world_to_camera);
    free(renderer->projection_matrix);
    free(renderer);
  }
}

void set_projection_matrix(Mat4f *mat, const Camera *camera) {
  float scale = 1.f / tanf((camera->angle_of_view * 0.5) * (M_PI / 180.f));
  Vec4f *m = (Vec4f *)mat;
  m[0][0] = scale;
  m[1][1] = -scale;
  m[2][2] = -camera->far_clip_plane /
            (camera->far_clip_plane - camera->near_clip_plane);
  m[2][3] = -(camera->far_clip_plane * camera->near_clip_plane) /
            (camera->far_clip_plane - camera->near_clip_plane);
  m[3][2] = -1.f;
  m[3][3] = 0.f;
};

void look_at(Mat4f *mat, const Vec3f from, const Vec3f target, const Vec3f up) {
  Vec3f forward = normalize(sub(from, target));
  Vec3f right = normalize(cross(up, forward));
  Vec3f upwards = cross(forward, right);

  Vec4f *m = (Vec4f *)mat->m;
  m[0][0] = right.x;
  m[1][0] = right.y;
  m[2][0] = right.z;
  m[3][0] = 0.f;
  m[0][1] = upwards.x;
  m[1][1] = upwards.y;
  m[2][1] = upwards.z;
  m[3][1] = 0.f;
  m[0][2] = forward.x;
  m[1][2] = forward.y;
  m[2][2] = forward.z;
  m[3][2] = 0.f;
  m[0][3] = from.x;
  m[1][3] = from.y;
  m[2][3] = from.z;
  m[3][3] = 1.f;
}

Vec2i get_pixel_coords(const Vec3f *vertex, const Renderer *renderer) {
  const Window *window = renderer->window;

  Vec4f camera_coords;
  Vec4f screen_coords;
  mul_mat_vec4f(&camera_coords, renderer->world_to_camera,
                &(Vec4f){vertex->x, vertex->y, vertex->z, 1.f});

  Vec4f *proj = renderer->projection_matrix->m;
  float w = proj[3][0] * camera_coords[0] + proj[3][1] * camera_coords[1] +
            proj[3][2] * camera_coords[2] + proj[3][3];
  mul_mat_vec4f(&screen_coords, renderer->projection_matrix, &camera_coords);
  screen_coords[0] /= w;
  screen_coords[1] /= w;
  screen_coords[2] /= w;
  screen_coords[3] /= w;

  Vec2i raster_coords = {
      .x = (int32_t)((screen_coords[0] + 1.f) * 0.5 * window->width),
      .y = (int32_t)((screen_coords[1] + 1.f) * 0.5 * window->height)};

  return raster_coords;
}

void draw_line(const Vec2i *raster_coords, uint32_t *image_buffer, Vec2i p0,
               Vec2i p1, const Window *window) {
  int x0 = p0.x;
  int x1 = p1.x;
  int y0 = p0.y;
  int y1 = p1.y;
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, e2;

  while (1) {
    if (0 <= x0 && x0 < window->width && 0 <= y0 && y0 < window->height) {
      image_buffer[x0 + y0 * window->width] = 0x00ff0000;
    }
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

    draw_line(raster_coords, renderer->main_buffer, v0, v1, renderer->window);
    draw_line(raster_coords, renderer->main_buffer, v0, v2, renderer->window);
    draw_line(raster_coords, renderer->main_buffer, v1, v2, renderer->window);
  }
}

void draw_frame_diff(Renderer *renderer) {
  printf("\x1B[?2026h");
  for (size_t i = 0; i < renderer->window->height - 1; i += 2) {
    for (size_t j = 0; j < renderer->window->width; j++) {
      size_t index_upper = j + i * renderer->window->width;
      size_t index_lower = j + (i + 1) * renderer->window->width;

      // Skip drawing if its the same
      if (renderer->main_buffer[index_upper] ==
              renderer->alt_buffer[index_upper] &&
          renderer->main_buffer[index_lower] ==
              renderer->alt_buffer[index_lower] &&
          !renderer->is_first_draw) {
        continue;
      }

      uint32_t rgba_u = renderer->main_buffer[index_upper];
      uint32_t rgba_l = renderer->main_buffer[index_lower];
      uint8_t r_u = rgba_u >> 24;
      uint8_t g_u = rgba_u >> 16 & 0xff;
      uint8_t b_u = rgba_u >> 8 & 0xff;
      uint8_t r_l = rgba_l >> 24;
      uint8_t g_l = rgba_l >> 16 & 0xff;
      uint8_t b_l = rgba_l >> 8 & 0xff;

      printf("\x1B[%i;%iH", (int)i / 2, (int)j);
      printf("\x1B[48;2;%i;%i;%im", r_l, g_l, b_l);
      printf("\x1B[38;2;%i;%i;%im", r_u, g_u, b_u);
      printf("%s", "\u2580");
    }
  }
  printf("\x1B[?2026l");
  uint32_t *tmp_pointer = (uint32_t *)renderer->alt_buffer;
  renderer->alt_buffer = renderer->main_buffer;
  renderer->main_buffer = tmp_pointer;
  renderer->is_first_draw = false;

  // After done drawing buffer we clear the old one immediately
  memset(renderer->main_buffer, 0,
         renderer->window->width * renderer->window->height * sizeof(uint32_t));
}

void draw_obj(Renderer *renderer, OBJ *obj) {
  Vec2i *raster_coords = (Vec2i *)malloc(obj->n_vertices * sizeof(Vec2i));
  set_projection_matrix(renderer->projection_matrix, renderer->camera);
  world_to_camera(renderer->world_to_camera, renderer->camera);

  for (int i = 0; i < obj->n_vertices; i++) {
    raster_coords[i] = get_pixel_coords(&obj->vertices[i], renderer);
  }

  draw_wire(obj, raster_coords, renderer);
  draw_frame_diff(renderer);

  free(raster_coords);
}
