#include "geometry.h"
#include "renderer.h"
#include "terminal.h"
#include <_stdio.h>
#include <math.h>
#include <poll.h>
#include <stdio.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <unistd.h>

Window window;

typedef struct KeyboardInputHandler {
  long long first_press_time;
  long long key_hit_time;
  char ch;
  bool is_pressed;
  struct pollfd fds[1];
} KeyboardInputHandler;

long long get_time_us() {
  struct timeval now;
  gettimeofday(&now, NULL);
  return now.tv_sec * 1000000 + now.tv_usec;
}

void handle_events(KeyboardInputHandler *input) {
  long long now = get_time_us();

  char buf[256];
  bool got_input = false;
  while (poll(input->fds, 1, 0) > 0 && (input->fds[0].revents & POLLIN)) {
    ssize_t n = read(input->fds[0].fd, buf, sizeof buf);
    if (n <= 0)
      break;
    input->ch = buf[n - 1];
    got_input = true;
  }

  if (got_input) {
    if (input->first_press_time > 0)
      input->key_hit_time = now;
    else
      input->first_press_time = now;
  }

  input->is_pressed = (now - input->first_press_time < 300000) ||
                      (now - input->key_hit_time < 100000);

  if (!input->is_pressed) {
    input->first_press_time = 0;
    input->ch = 0;
  }
}

void handle_user_input(const KeyboardInputHandler *handler,
                       const Renderer *renderer) {}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: ./main mesh.obj\n");
    return 1;
  }

  if (argc > 2) {
    printf("ERROR: Too many argument\n");
    return 1;
  }

  setup_tui();
  KeyboardInputHandler event_handler;
  event_handler.fds[0] = (struct pollfd){.fd = fileno(stdin), .events = POLLIN};
  event_handler.first_press_time = 0;
  event_handler.key_hit_time = 0;

  const char *path = argv[1];
  OBJ *obj = OBJ_from_path(path);

  Camera camera = {.film_aperture_height = 1.0,
                   .film_aperture_width =
                       (float)window.width / (float)window.height,
                   .angle_of_view = 60.f,
                   .far_clip_plane = 100.0,
                   .near_clip_plane = 0.1,
                   .position = {0.f, 0.f, 0.f},
                   .up = {0.f, 1.f, 0.f},
                   .look_at = {0.f, 0.f, 0.f}};

  Renderer *renderer = Renderer_Init(&camera, &window);

  float fps = 60.f;

  long long frame_time_us = (long long)(1000 * 1000 / fps);
  long long last_frame_time = get_time_us();
  float radius = 2.f;
  float theta = M_PI / 2;
  float phi = 0.f;
  float speed = 1.5f;

  for (;;) {
    long long now = get_time_us();
    float dt = (float)(now - last_frame_time) / 1000000.f;
    last_frame_time = now;

    handle_events(&event_handler);

    if (event_handler.is_pressed) {
      switch (event_handler.ch) {
      case 'W':
        theta -= speed * dt;
        break;
      case 'S':
        theta += speed * dt;
        break;
      case 's':
        radius += speed * dt;
        break;
      case 'w':
        radius -= speed * dt;
        break;
      case 'a':
        phi -= speed * dt;
        break;
      case 'd':
        phi += speed * dt;
        break;
      }
    }
    camera.position.z = radius * sinf(theta) * cosf(phi);
    camera.position.x = radius * sinf(theta) * sinf(phi);
    camera.position.y = radius * cosf(theta);

    draw_obj(renderer, obj);

    long long elapsed = get_time_us() - now;
    if (elapsed < frame_time_us)
      usleep((useconds_t)(frame_time_us - elapsed));
  }

  OBJ_Deinit(obj);
  Renderer_Deinit(renderer);

  return 0;
}
