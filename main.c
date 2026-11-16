#include "geometry.h"
#include "vec.h"
#include <_time.h>
#include <_types.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

struct termios initial;

void restore() { tcsetattr(1, TCSANOW, &initial); }

void terminit() {
  struct termios t;
  tcgetattr(1, &t);
  initial = t;
  atexit(restore);
  t.c_lflag &= (~ECHO & ~ICANON);
  tcsetattr(1, TCSANOW, &t);
}

typedef struct Window {
  size_t width;
  size_t height;
} Window;

typedef struct {
  u_int8_t r;
  u_int8_t g;
  u_int8_t b;
} Color;

Window window;

void resize(int i) {
  struct winsize ws;
  ioctl(1, TIOCGWINSZ, &ws);
  window = (Window){.width = ws.ws_col, .height = ws.ws_row * 2};
}

void draw_at_position(const Color *upper_color, const Color *lower_color,
                      const Vec2i *position) {
  printf("\x1B[%i;%iH", position->y, position->x);
  printf("\x1B[48;2;%i;%i;%im", lower_color->r, lower_color->g, lower_color->b);
  printf("\x1B[38;2;%i;%i;%im", upper_color->r, upper_color->g, upper_color->b);
  printf("%s", "\u2580");
}

void clean_up() {
  printf("\x1b[2J");
  printf("\x1b[?1049l");
  printf("\x1b[?25h");
  exit(0);
}

void setup_tui() {
  printf("\x1B[?1049h");
  printf("\x1B[2J");
  printf("\x1b[?25l");

  signal(SIGWINCH, resize);
  signal(SIGINT, clean_up);
  atexit(clean_up);
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

void draw_wire(const OBJ *obj, const Vec2i *raster_coords,
               uint32_t *image_buffer, Window window) {
  for (int i = 0; i < obj->n_faces; i++) {
    Vec3i face = obj->face_indices[i];
    Vec2i v0 = raster_coords[face.x];
    Vec2i v1 = raster_coords[face.y];
    Vec2i v2 = raster_coords[face.z];

    draw_line(raster_coords, image_buffer, v0, v1, window.width);
    draw_line(raster_coords, image_buffer, v0, v2, window.width);
    draw_line(raster_coords, image_buffer, v1, v2, window.width);
  }
}

void clear_frame(uint32_t *image_buffer, size_t width, size_t height) {
  memset(image_buffer, 0, width * height * sizeof(uint32_t));
}

void draw_frame(const uint32_t *image_buffer, Window window) {
  for (size_t i = 0; i < window.height - 1; i += 2) {
    for (size_t j = 0; j < window.width; j++) {
      uint32_t rgba_u = image_buffer[j + i * window.width];
      uint32_t rgba_l = image_buffer[j + (i + 1) * window.width];
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

typedef struct CameraConfig {
  // All metric
  float film_aperture_height;
  float film_aperture_width;
  float focal_length;
  float near_clip_plane;
  size_t window_width, window_height;
} CameraConfig;

void calculate_raster_coords(Vec2i *raster_coords, const CameraConfig cfg,
                             Vec3f *screen_coords, Vec3f *ndc_coords,
                             const Vec3f *vertices, const size_t n_vertices,
                             const Vec3i *faces, const size_t n_faces) {
  float canvas_top =
      ((cfg.film_aperture_height / 2) / cfg.focal_length) * cfg.near_clip_plane;
  float canvas_bottom = -canvas_top;
  float canvas_right =
      canvas_top * (cfg.film_aperture_width / cfg.film_aperture_height);
  float canvas_left = -canvas_right;

  for (size_t i = 0; i < n_vertices; i++) {
    screen_coords[i] =
        (Vec3f){.x = cfg.near_clip_plane * vertices[i].x / -vertices[i].z,
                .y = cfg.near_clip_plane * vertices[i].y / -vertices[i].z,
                .z = -vertices[i].z};

    ndc_coords[i] = (Vec3f){
        .x = 2 * screen_coords[i].x / (canvas_right - canvas_left) -
             (canvas_right + canvas_left) / (canvas_right - canvas_left),
        .y = 2 * screen_coords[i].y / (canvas_top - canvas_bottom) -
             (canvas_top + canvas_bottom) / (canvas_top - canvas_bottom),
        .z = screen_coords[i].z};

    raster_coords[i] =
        (Vec2i){.x = (int)(((1.0 + ndc_coords[i].x) / 2.0) * window.width),
                .y = (int)(((1.0 + ndc_coords[i].y) / 2.0) * window.height)};
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: ./main mesh.obj\n");
    return 1;
  }

  if (argc > 2) {
    printf("ERROR: Too many argument\n");
    return 1;
  }

  const char *path = argv[1];
  OBJ *obj = OBJ_from_path(path);

  terminit();
  setup_tui();
  resize(0);

  Vec3f *vertices = (Vec3f *)malloc(8 * sizeof(Vec3f));
  Vec3i *face_indices = (Vec3i *)malloc(12 * sizeof(Vec3i));

  CameraConfig camera_config = {.film_aperture_height = 1.0,
                                .film_aperture_width = 1.0,
                                .focal_length = 35.0,
                                .near_clip_plane = 0.1,
                                .window_width = window.width,
                                .window_height = window.height};

  Vec3f *screen_coords = (Vec3f *)malloc(obj->n_vertices * sizeof(Vec3f));
  Vec3f *ndc_coords = (Vec3f *)malloc(obj->n_vertices * sizeof(Vec3f));
  Vec2i *raster_coords = (Vec2i *)malloc(obj->n_vertices * sizeof(Vec2i));
  uint32_t *frame_buffer =
      (uint32_t *)calloc(window.width * window.height, sizeof(uint32_t));

  calculate_raster_coords(raster_coords, camera_config, screen_coords,
                          ndc_coords, obj->vertices, obj->n_vertices,
                          obj->face_indices, obj->n_faces);

  draw_wire(obj, raster_coords, frame_buffer, window);
  draw_frame(frame_buffer, window);

  char buf;
  for (;;) {
    scanf("%c", &buf);
    if (buf == 'q') {
      clean_up();
    }
  }

  OBJ_Deinit(obj);

  free(screen_coords);
  free(ndc_coords);
  free(raster_coords);
  free(frame_buffer);

  return 0;
}
