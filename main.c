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
  char *path = argv[1];

  terminit();
  setup_tui();
  resize(0);

  FILE *fp = fopen(path, "r");
  PPMImage img = read_ppm(fp);
  draw_frame((uint32_t *)img.img_buffer, img.width, img.height);

  char buf;

  printf("\x1B[0m");
  printf("\x1B[%d;0f", window_height);
  printf("Image size: %zux%zu Window size: %dx%d", img.width, img.height,
         window_width, window_height * 2);

  for (;;) {
    scanf("%c", &buf);
    if (buf == 'q') {
      clean_up();
    }
  }

  fclose(fp);
  free(img.img_buffer);

  return 0;
}
