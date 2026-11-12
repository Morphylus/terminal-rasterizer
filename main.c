#include <_time.h>
#include <_types.h>
#include <signal.h>
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

uint16_t width;
uint16_t height;

void resize(int i) {
  struct winsize ws;
  ioctl(1, TIOCGWINSZ, &ws);
  width = ws.ws_col;
  height = ws.ws_row;
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

void draw_at_position(const Color *color, const Position *position) {
  printf("\x1B[%i;%iH", position->x, position->y);
  printf("\x1B[38;2;%i;%i;%im", color->r, color->g, color->b);
  printf("%s", "\u2588");
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

int main(int argc, char *argv[]) {
  char *file = argv[1];

  terminit();
  setup_tui();
  resize(0);

  FILE *fp = fopen(file, "r");
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
      malloc(image_width * image_height * sizeof(uint32_t));

  for (int i = 0; i < image_width; i++) {
    for (int j = 0; j < image_height; j++) {
      fscanf(fp, "%d %d %d", &r, &g, &b);
      uint32_t rgba = r << 24 | g << 16 | b << 8;
      image_buffer[j + i * image_width] = rgba;
    }
  }

  for (int i = 0; i < image_width; i++) {
    for (int j = 0; j < image_height; j++) {
      uint32_t rgba = image_buffer[i + j * image_width];
      uint8_t r = rgba >> 24;
      uint8_t g = rgba >> 16 & 0xff;
      uint8_t b = rgba >> 8 & 0xff;
      draw_at_position(&(Color){.r = r, .g = g, .b = b},
                       &(Position){.x = j, .y = i});
    }
  }
  char buf;

  for (;;) {
    scanf("%c", &buf);
    if (buf == 'q') {
      clean_up();
    }
  }

  fclose(fp);
  free(image_buffer);

  return 0;
}
