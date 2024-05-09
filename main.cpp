#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <cstdint>
#include <cstddef>

#include "color.hpp"

#define RLIGHTS_IMPLEMENTATION
#include "rlight.h"
#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

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

// source of line aliasing issues btw
#define Z_GROUND 0.01f
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

Model gen_voxel_model(const char *texture_path, Shader &s) {
  Mesh mesh         = GenMeshCube(UNIT_SZ, UNIT_SZ, UNIT_SZ);
  Texture2D texture = LoadTexture(texture_path);
  Model cube        = LoadModelFromMesh(mesh);
  cube.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
  cube.materials[0].shader = s;
  return cube;
}

/**
  @brief Takes uint32 and "snaps" voxel to grid.
  @todo  Strictly speaking, this may result in undefined behavior
         due to overflow, we should probably use static assertion.
*/
void draw_voxel_to_grid(Model &m, uint32_t x, uint32_t y, uint32_t h)
{
  DrawModel(m, Vector3{
      (float)x * UNIT_SZ,
      (float)y * UNIT_SZ,
      (((float)h + 0.5f) * UNIT_SZ) + Z_GROUND
    }, 1.0f, WHITE);
}

int main(void) {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "ortholand");
    SetTargetFPS(144);

    Camera3D cam;
    cam.up       = Vector3{0.0f, 0.0f, 1.0f};
    cam.position = Vector3{100.0f * UNIT_SZ, 100.0f * UNIT_SZ, 50.0f * UNIT_SZ};
    cam.target   = Vector3{WORLD_SZ *  UNIT_SZ * 0.5f,
                           WORLD_SZ *  UNIT_SZ * 0.5f,
                           Z_GROUND + 1.0f};
    cam.fovy       = 45.0f;
    cam.projection = CAMERA_ORTHOGRAPHIC;


    // BEGIN: shaders
    Shader lt_shader = LoadShader(TextFormat("resources/light.vs",
                                             GLSL_VERSION),
                                  TextFormat("resources/light.fs",
                                             GLSL_VERSION));
    lt_shader.locs[SHADER_LOC_VECTOR_VIEW] =
      GetShaderLocation(lt_shader, "viewPos");
    int ambientLoc = GetShaderLocation(lt_shader, "ambient");
    float lt_vals[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    SetShaderValue(lt_shader, ambientLoc, lt_vals, SHADER_UNIFORM_VEC4);

    Light sun = CreateLight(LIGHT_POINT, Vector3{0, 0, 10},
                            Vector3Zero(), WHITE, lt_shader);
    sun.enabled = true;
    UpdateLightValues(lt_shader, sun);
    // END: shaders

    Model mdl_test_vox = gen_voxel_model("resources/textures/brick.png",
                                         lt_shader);

    while (!WindowShouldClose()) {
      io_handle_mouse(&cam);
      UpdateCamera(&cam, CAMERA_FREE);

      float cam_view[3] = { cam.position.x, cam.position.y, cam.position.z };
      SetShaderValue(lt_shader, lt_shader.locs[SHADER_LOC_VECTOR_VIEW],
                     cam_view, SHADER_UNIFORM_VEC3);
      BeginDrawing();
        ClearBackground(color::darkgray);
        BeginMode3D(cam);
          draw_checkerboard();
          BeginShaderMode(lt_shader);
            draw_voxel_to_grid(mdl_test_vox, WORLD_SZ * 0.5, WORLD_SZ * 0.5, 0);
          EndShaderMode();
        EndMode3D();
        debug_fovy(cam);
      EndDrawing();
    }

    CloseWindow();
    return EXIT_SUCCESS;
}
