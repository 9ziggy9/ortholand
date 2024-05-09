#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <cstdint>
#include "color.hpp"

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 720

void debug_fovy(Camera3D &cam) {
  #define FONT_SIZE 16
  char txt_fovy[32];
  std::sprintf(txt_fovy, "fovy: %f", cam.fovy);
  int txt_fovy_width = MeasureText(txt_fovy, FONT_SIZE) + 4;
  DrawText(txt_fovy, SCREEN_WIDTH - txt_fovy_width, 1, FONT_SIZE, GREEN);
}

void io_handle_mouse(Camera3D *cam) {
  #define ZOOM_SPEED 10.0f
  cam->fovy -= GetMouseWheelMove() * ZOOM_SPEED;
  cam->fovy = Clamp(cam->fovy, 8.0f, 92.5f);
}

// Slightly higher to prevent aliasing issues
#define Z_GROUND 0.1f
#define UNIT_SZ  2.0f
#define WORLD_SZ 100

/**
  @brief Procedurally draws checkerboard.
  @todo  When camera position has settled in, draw to texture.
*/
void draw_checkerboard(void) {
  #define CHECKER_SCL  Vector3{UNIT_SZ, UNIT_SZ, Z_GROUND}
  for (int y = 0; y < WORLD_SZ; y++) {
    for (int x = 0; x < WORLD_SZ; x++) {
      Vector3 p = { (float)x * UNIT_SZ, (float)y * UNIT_SZ, 0.0f };
      DrawCubeV(p, CHECKER_SCL, ((y + x) % 2 == 0) ? BLACK : color::darkgray);
    }
  }
}

/**
  @brief Takes uint32 and "snaps" voxel to grid.
  @todo  Strictly speaking, this may result in undefined behavior
         due to overflow, we should probably use static assertion.
  @todo  Do something about drawing wires indiscriminately.
*/
void draw_voxel_to_grid(uint32_t x, uint32_t y, uint32_t h, Color c) {
  Vector3 cb_pos   = Vector3{(float)x * UNIT_SZ,
                             (float)y * UNIT_SZ,
                             (float)((h + 0.5f) * UNIT_SZ) + Z_GROUND};
  Vector3 cb_scale = Vector3{UNIT_SZ, UNIT_SZ, UNIT_SZ};
  Color   cb_color = c;
  DrawCubeV(cb_pos, cb_scale, cb_color);
  DrawCubeWiresV(cb_pos, cb_scale, color::white);
}

int main(void) {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "ortholand");

    Camera3D cam;
    cam.up         = Vector3{0.0f, 0.0f, 1.0f};
    cam.position   = Vector3{100.0f * UNIT_SZ,
                             100.0f * UNIT_SZ,
                              50.0f * UNIT_SZ};
    cam.target = Vector3{WORLD_SZ *  UNIT_SZ * 0.5f,
                         WORLD_SZ *  UNIT_SZ * 0.5f,
                         Z_GROUND + 1.0f};
    cam.fovy       = 45.0f;
    cam.projection = CAMERA_ORTHOGRAPHIC;

    while (!WindowShouldClose()) {
      ClearBackground(color::darkgray);
      BeginDrawing();
        BeginMode3D(cam);
          io_handle_mouse(&cam);
          draw_checkerboard();
          draw_voxel_to_grid(WORLD_SZ * 0.5, WORLD_SZ * 0.5, 0,
                             color::darkgreen);
          draw_voxel_to_grid(WORLD_SZ * 0.5, WORLD_SZ * 0.5, 1,
                             color::darkgreen);
          draw_voxel_to_grid(WORLD_SZ * 0.5, WORLD_SZ * 0.5, 2,
                             color::darkgreen);
          draw_voxel_to_grid(WORLD_SZ * 0.5, WORLD_SZ * 0.5, 3,
                             color::darkgreen);
        EndMode3D();
        debug_fovy(cam);
      EndDrawing();
    }

    CloseWindow();
    return EXIT_SUCCESS;
}
