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

  setup_tui(&window);

  const char *path = argv[1];
  OBJ *obj = OBJ_from_path(path);

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
      cleanup_tui();
    }
  }

  OBJ_Deinit(obj);
  Renderer_Deinit(renderer);

  return 0;
}
