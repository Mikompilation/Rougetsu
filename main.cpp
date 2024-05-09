#include "graph3d/gra3dSGD.h"
#include "graph3d/sgd_types.h"
#include "packfile.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include "raylib.h"

char *ReadFullFile(std::filesystem::path filename)
{
  if (!std::filesystem::exists(filename))
  {
    exit(-1);
  }

  auto fileSize = std::filesystem::file_size(filename);

  char *buffer = new char[fileSize];
  std::ifstream infile(filename, std::ios::binary);
  infile.read(buffer, fileSize);

  infile.close();

  return buffer;
}

int main(int argc, const char *argv[])
{  
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " <sgd>" << std::endl;
    return -1;
  }
  
  auto pakFile = (PK2_HEAD *) ReadFullFile(argv[1]);
  auto mdlPak = (PK2_HEAD *) GetFileInPak(pakFile, 0);
  
  /// For Maps
  //auto sgdTop = (SGDFILEHEADER *) mdlPak;

  /// For characters
  auto sgdTop = (SGDFILEHEADER *) GetFileInPak(mdlPak, 0);
  sgdRemap(sgdTop);
  
  InitWindow(800, 450, "Rougetsu");
  Camera3D camera = { 0 };
  camera.position = { 10.0f, 10.0f, 10.0f }; // Camera position
  camera.target = { 0.0f, 0.0f, 0.0f };      // Camera looking at point
  camera.up = { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
  camera.fovy = 45.0f;                                // Camera field-of-view Y
  camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

  DisableCursor();                    // Limit cursor to relative movement inside the window

  SetTargetFPS(60); 
  
  while (!WindowShouldClose())
  {
    UpdateCamera(&camera, CAMERA_FREE);
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawFPS(0, 0); 
    BeginMode3D(camera);
    /// Simulate DrawGirl
    _gra3dDrawSGD(sgdTop, SRT_REALTIME, nullptr, -1);
    DrawGrid(10, 1.0f);
    EndMode3D();
    EndDrawing();
  }
  
  CloseWindow();
  
  return 0;
}
