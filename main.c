#include "geometry.h"
#include "vec.h"
#include <_time.h>
#include <_types.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>

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

uint16_t window_width;
uint16_t window_height;

void resize(int i) {
  struct winsize ws;
  ioctl(1, TIOCGWINSZ, &ws);
  window_width = ws.ws_col;
  window_height = ws.ws_row;
}

typedef struct {
  u_int8_t r;
  u_int8_t g;
  u_int8_t b;
} Color;

typedef struct {
  int x;
  int y;
} Position;

typedef struct {
  u_int32_t *img_buffer;
  size_t width;
  size_t height;
} PPMImage;

void draw_at_position(const Color *upper_color, const Color *lower_color,
                      const Position *position) {
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

PPMImage read_ppm(FILE *fp) {
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int image_width;
  int image_height;
  int r, g, b;
  int spec;

  getline(&line, &len, fp);
  fscanf(fp, "%d %d", &image_width, &image_height);
  fscanf(fp, "%d", &spec);
  uint32_t *image_buffer =
      (uint32_t *)malloc(image_width * image_height * sizeof(uint32_t));

  for (int i = 0; i < image_width; i++) {
    for (int j = 0; j < image_height; j++) {
      fscanf(fp, "%d %d %d", &r, &g, &b);
      uint32_t rgba = r << 24 | g << 16 | b << 8;
      image_buffer[j + i * image_width] = rgba;
    }
  }

  return (PPMImage){
      .img_buffer = image_buffer, .height = image_height, .width = image_width};
}

void draw_frame(const uint32_t *image_buffer, size_t width, size_t height) {
  for (size_t i = 0; i < width - 1; i += 2) {
    for (size_t j = 0; j < height; j++) {
      uint32_t rgba_u = image_buffer[j + i * width];
      uint32_t rgba_l = image_buffer[j + (i + 1) * width];
      uint8_t r_u = rgba_u >> 24;
      uint8_t g_u = rgba_u >> 16 & 0xff;
      uint8_t b_u = rgba_u >> 8 & 0xff;
      uint8_t r_l = rgba_l >> 24;
      uint8_t g_l = rgba_l >> 16 & 0xff;
      uint8_t b_l = rgba_l >> 8 & 0xff;

      draw_at_position(&(Color){.r = r_u, .g = g_u, .b = b_u},
                       &(Color){.r = r_l, .g = g_l, .b = b_l},
                       &(Position){.x = j, .y = i / 2});
    }
  }
}

int main(int argc, char *argv[]) {
  terminit();
  setup_tui();
  resize(0);

  Vec3f *vertices = (Vec3f *)malloc(8 * sizeof(Vec3f));
  Vec3i *face_indices = (Vec3i *)malloc(12 * sizeof(Vec3i));

  vertices[0] = (Vec3f){.x = -0.1, .y = -0.1, .z = 10.0}; // LLF
  vertices[1] = (Vec3f){.x = 0.1, .y = -0.1, .z = 10.0};  // LRF
  vertices[2] = (Vec3f){.x = -0.1, .y = 0.1, .z = 10.0};  // ULF
  vertices[3] = (Vec3f){.x = 0.1, .y = 0.1, .z = 10.0};   // URF
  vertices[4] = (Vec3f){.x = -0.1, .y = -0.1, .z = 20.0}; // LLB
  vertices[5] = (Vec3f){.x = 0.1, .y = -0.1, .z = 20.0};  // LRB
  vertices[6] = (Vec3f){.x = -0.1, .y = 0.1, .z = 20.0};  // ULB
  vertices[7] = (Vec3f){.x = 0.1, .y = 0.1, .z = 20.0};   // URB

  face_indices[0] = (Vec3i){.x = 0, .y = 1, .z = 2};
  face_indices[1] = (Vec3i){.x = 2, .y = 1, .z = 3};
  face_indices[2] = (Vec3i){.x = 4, .y = 6, .z = 5};
  face_indices[3] = (Vec3i){.x = 6, .y = 7, .z = 5};
  face_indices[4] = (Vec3i){.x = 0, .y = 2, .z = 4};
  face_indices[5] = (Vec3i){.x = 2, .y = 6, .z = 4};
  face_indices[6] = (Vec3i){.x = 1, .y = 5, .z = 3};
  face_indices[7] = (Vec3i){.x = 3, .y = 5, .z = 7};
  face_indices[8] = (Vec3i){.x = 0, .y = 4, .z = 1};
  face_indices[9] = (Vec3i){.x = 1, .y = 4, .z = 5};
  face_indices[10] = (Vec3i){.x = 2, .y = 3, .z = 6};
  face_indices[11] = (Vec3i){.x = 3, .y = 7, .z = 6};

  OBJ cube = {.vertices = vertices,
              .face_indices = face_indices,
              .n_vertices = 8,
              .n_faces = 12};

  Vec3f *screen_coords = (Vec3f *)malloc(cube.n_vertices * sizeof(Vec3f));
  Vec3f *ndc_coords = (Vec3f *)malloc(cube.n_vertices * sizeof(Vec3f));
  Vec2i *raster_coords = (Vec2i *)malloc(cube.n_vertices * sizeof(Vec2i));

  // All metric
  float film_aperture_height = 1.0;
  float film_aperture_width = 1.0;
  float focal_length = 35.0;
  float near_clip_plane = 0.1;

  float canvas_top =
      ((film_aperture_height / 2) / focal_length) * near_clip_plane;
  float canvas_bottom = -canvas_top;
  float canvas_right =
      canvas_top * (film_aperture_width / film_aperture_height);
  float canvas_left = -canvas_right;

  for (size_t i = 0; i < cube.n_vertices; i++) {
    screen_coords[i] =
        (Vec3f){.x = near_clip_plane * cube.vertices[i].x / -cube.vertices[i].z,
                .y = near_clip_plane * cube.vertices[i].y / -cube.vertices[i].z,
                .z = -cube.vertices[i].z};

    ndc_coords[i] = (Vec3f){
        .x = 2 * screen_coords[i].x / (canvas_right - canvas_left) -
             (canvas_right + canvas_left) / (canvas_right - canvas_left),
        .y = 2 * screen_coords[i].y / (canvas_top - canvas_bottom) -
             (canvas_top + canvas_bottom) / (canvas_top - canvas_bottom),
        .z = screen_coords[i].z};

    raster_coords[i] =
        (Vec2i){.x = (int)(((1.0 + ndc_coords[i].x) / 2.0) * window_width),
                .y = (int)(((1.0 + ndc_coords[i].y) / 2.0) * window_height)};
  }

  for (int i = 0; i < 8; i++) {
    draw_at_position(
        &(Color){.r = 0, .g = 255, .b = 0}, &(Color){.r = 0, .g = 0, .b = 0},
        &(Position){.x = raster_coords[i].x, .y = raster_coords[i].y});
  }

  char buf;
  for (;;) {
    scanf("%c", &buf);
    if (buf == 'q') {
      clean_up();
    }
  }

  free(vertices);
  free(face_indices);

  return 0;
}
