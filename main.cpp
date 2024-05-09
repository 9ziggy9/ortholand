#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <cstdint>
#include <cstddef>

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

Model generate_voxel_model_from_mesh(const char *texture_path) {
  Mesh mesh         = GenMeshCube(UNIT_SZ, UNIT_SZ, UNIT_SZ);
  Texture2D texture = LoadTexture(texture_path);
  Model cube        = LoadModelFromMesh(mesh);
  cube.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
  return cube;
}

Model generate_voxel_model(Texture2D *ts[6]) {
  Mesh mesh;
  mesh.vertexCount = 24;
  mesh.triangleCount = 12;

  float verts[] = {
    // front
    -UNIT_SZ , -UNIT_SZ, UNIT_SZ,  0.0f, 0.0f,
     UNIT_SZ , -UNIT_SZ, UNIT_SZ,  1.0f, 0.0f,
     UNIT_SZ ,  UNIT_SZ, UNIT_SZ,  1.0f, 1.0f,
    -UNIT_SZ ,  UNIT_SZ, UNIT_SZ,  0.0f, 1.0f,
    // back
    -UNIT_SZ , -UNIT_SZ, -UNIT_SZ, 0.0f, 0.0f,
     UNIT_SZ , -UNIT_SZ, -UNIT_SZ, 1.0f, 0.0f,
     UNIT_SZ ,  UNIT_SZ, -UNIT_SZ, 1.0f, 1.0f,
    -UNIT_SZ ,  UNIT_SZ, -UNIT_SZ, 0.0f, 1.0f,
    // top
    -UNIT_SZ , UNIT_SZ, -UNIT_SZ,  0.0f, 0.0f,
     UNIT_SZ , UNIT_SZ, -UNIT_SZ,  1.0f, 0.0f,
     UNIT_SZ , UNIT_SZ, UNIT_SZ,   1.0f, 1.0f,
    -UNIT_SZ , UNIT_SZ, UNIT_SZ,   0.0f, 1.0f,
    // bottom
    -UNIT_SZ , -UNIT_SZ, -UNIT_SZ, 0.0f, 0.0f,
     UNIT_SZ , -UNIT_SZ, -UNIT_SZ, 1.0f, 0.0f,
     UNIT_SZ , -UNIT_SZ, UNIT_SZ,  1.0f, 1.0f,
    -UNIT_SZ , -UNIT_SZ, UNIT_SZ,  0.0f, 1.0f,
    // left
    -UNIT_SZ , -UNIT_SZ, -UNIT_SZ, 0.0f, 0.0f,
    -UNIT_SZ , -UNIT_SZ, UNIT_SZ,  1.0f, 0.0f,
    -UNIT_SZ ,  UNIT_SZ, UNIT_SZ,  1.0f, 1.0f,
    -UNIT_SZ ,  UNIT_SZ, -UNIT_SZ, 0.0f, 1.0f,
    // right
    UNIT_SZ  , -UNIT_SZ, -UNIT_SZ, 0.0f, 0.0f,
    UNIT_SZ  , -UNIT_SZ, UNIT_SZ,  1.0f, 0.0f,
    UNIT_SZ  ,  UNIT_SZ, UNIT_SZ,  1.0f, 1.0f,
    UNIT_SZ  ,  UNIT_SZ, -UNIT_SZ, 0.0f, 1.0f,
  };

  int idxs[] = {
    0  ,  1,  2,  2,  3,  0, // front
    4  ,  5,  6,  6,  7,  4, // back
    8  ,  9, 10, 10, 11,  8, // top
    12 , 13, 14, 14, 15, 12, // bottom
    16 , 17, 18, 18, 19, 16, // left
    20 , 21, 22, 22, 23, 20, // right
  };

  mesh.vertices = (float *)
    MemAlloc((uint32_t)mesh.vertexCount * 3 * sizeof(float));

  mesh.texcoords = (float *)
    MemAlloc((uint32_t)mesh.vertexCount * 2 * sizeof(float));

  mesh.indices = (uint16_t *)
    MemAlloc((uint32_t)mesh.triangleCount*3*sizeof(uint16_t));

  for (int i = 0; i < 3 * mesh.vertexCount; i += 3) {
    mesh.vertices[i] = verts[i];
    mesh.vertices[i + 1] = verts[i + 1];
    mesh.vertices[i + 2] = verts[i + 2];
    mesh.texcoords[i / 3 * 2] = verts[i + 3];
    mesh.texcoords[i / 3 * 2 + 1] = verts[i + 4];
  }

  for (int i = 0; i < 3 * mesh.triangleCount; i++)
    mesh.indices[i] = (uint16_t)idxs[i];

  Model model = LoadModelFromMesh(mesh);

  for (int i = 0; i < 6; i++)
    model.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = *ts[i];

  return model;
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

    Model mdl_test_vox
      = generate_voxel_model_from_mesh("resources/textures/brick.png");

    while (!WindowShouldClose()) {
      ClearBackground(color::darkgray);
      io_handle_mouse(&cam);
      BeginDrawing();
        BeginMode3D(cam);
          draw_checkerboard();
          draw_voxel_to_grid(mdl_test_vox, WORLD_SZ * 0.5, WORLD_SZ * 0.5, 0);
        EndMode3D();
        debug_fovy(cam);
      EndDrawing();
    }

    CloseWindow();
    return EXIT_SUCCESS;
}
