#include "geometry.h"
#include "renderer.h"
#include "terminal.h"
#include <math.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

Window window;

long long get_time_us() {
  struct timeval now;
  gettimeofday(&now, NULL);
  return now.tv_sec * 1000000 + now.tv_usec;
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

  setup_tui();

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

  char buf;
  long long frame_time_us = (long long)(1000 * 1000 / fps);
  long long last_draw, curr;
  float radius = 2.f;
  float theta = M_PI / 2;
  float phi = 0.f;

  for (;;) {
    curr = get_time_us();

    if (curr - last_draw >= frame_time_us) {
      if (buf == 'W') {
        theta -= 0.1f;
      }
      if (buf == 'S') {
        theta += 0.1f;
      }
      if (buf == 'w') {
        radius -= 0.1f;
      }
      if (buf == 's') {
        radius += 0.1f;
      }
      if (buf == 'a') {
        phi -= 0.1f;
      }
      if (buf == 'd') {
        phi += 0.1f;
      }
      camera.position.z = radius * sinf(theta) * cosf(phi);
      camera.position.x = radius * sinf(theta) * sinf(phi);
      camera.position.y = radius * cosf(theta);

      clear_frame(renderer);
      draw_obj(renderer, obj);
      last_draw = get_time_us();
    }
    scanf("%c", &buf);
  }

  OBJ_Deinit(obj);
  Renderer_Deinit(renderer);

  return 0;
}
