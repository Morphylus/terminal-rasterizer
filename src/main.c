#include "geometry.h"
#include "renderer.h"
#include "terminal.h"
#include <stdio.h>

Window window;

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
                   .position = {2.f, 2.f, 2.f},
                   .up = {0.f, 1.f, 0.f},
                   .look_at = {0.f, 0.f, 0.f}};

  Renderer *renderer = Renderer_Init(&camera, &window);
  draw_obj(renderer, obj);

  char buf;
  for (;;) {
    scanf("%c", &buf);
    if (buf == 'q') {
      cleanup_tui();
    }
  }

  OBJ_Deinit(obj);
  Renderer_Deinit(renderer);

  return 0;
}
