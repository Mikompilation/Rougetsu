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
  auto pakFile = (PK2_HEAD *) ReadFullFile(argv[1]);
  auto mdlPak = (PK2_HEAD *) GetFileInPak(pakFile, 0);
  
  /// For Maps
  //auto sgdTop = (SGDFILEHEADER *) mdlPak;

  /// For characters
  auto sgdTop = (SGDFILEHEADER *) GetFileInPak(mdlPak, 0);
  sgdRemap(sgdTop);
  
  InitWindow(800, 450, "raylib [core] example - basic window");
  
  while (!WindowShouldClose())
  {
    BeginDrawing();
    /// Simulate DrawGirl
    _gra3dDrawSGD(sgdTop, SRT_REALTIME, nullptr, -1);
    EndDrawing();
  }
  
  CloseWindow();
  
  return 0;
}
