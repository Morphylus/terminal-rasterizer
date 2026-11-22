#include "terminal.h"
#include "renderer.h"
#include <stdio.h>
#include <sys/ioctl.h>

extern Window window;

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

void resize(int i) {
  struct winsize ws;
  ioctl(1, TIOCGWINSZ, &ws);
  window.width = ws.ws_col;
  window.height = ws.ws_row * 2;
  // window.width = 80;
  // window.height = 80;
}

void clean_up() {
  printf("\x1b[2J");
  printf("\x1b[?1049l");
  printf("\x1b[?25h");
  exit(0);
}

void setup() {
  printf("\x1B[?1049h");
  printf("\x1B[2J");
  printf("\x1b[?25l");

  signal(SIGWINCH, resize);
  signal(SIGINT, clean_up);
  atexit(clean_up);
}

void setup_tui() {
  terminit();
  setup();
  resize(0);
}

void cleanup_tui() { clean_up(); }
