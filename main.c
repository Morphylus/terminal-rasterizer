#include "geometry.h"
#include "renderer.h"
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

Window window;

void resize(int i) {
  struct winsize ws;
  ioctl(1, TIOCGWINSZ, &ws);
  window = (Window){.width = ws.ws_col, .height = ws.ws_row * 2};
}

static inline size_t flat_index(const Window win, Vec2i coords) {
  return coords.x + coords.y * win.width;
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

void clear_frame(uint32_t *image_buffer, size_t width, size_t height) {
  memset(image_buffer, 0, width * height * sizeof(uint32_t));
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

  Camera camera = {.film_aperture_height = 1.0,
                   .film_aperture_width = 1.0,
                   .focal_length = 35.0,
                   .near_clip_plane = 0.1,
                   .window_width = window.width,
                   .window_height = window.height};

  Renderer *renderer = Renderer_Init(&camera, &window);
  draw_obj(renderer, obj);

  char buf;
  for (;;) {
    scanf("%c", &buf);
    if (buf == 'q') {
      clean_up();
    }
  }

  OBJ_Deinit(obj);
  Renderer_Deinit(renderer);

  return 0;
}
