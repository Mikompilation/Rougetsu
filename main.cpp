#include "common/packfile.h"
#include "graph3d/gra3dSGD.h"
#include "graph3d/sgd_types.h"
#include "render/Vu1Mem.h"
#include <filesystem>
#include <fstream>
#include <iostream>

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
  
  if (InitializeVu1Memory())
  {
    return -1;
  }
  
  InitializeShaders();
  
  auto pakFile = (PK2_HEAD *) ReadFullFile(argv[1]);
  auto mdlPak = (PK2_HEAD *) GetFileInPak(pakFile, 0);
  
  /// For Maps
  //auto sgdTop = (SGDFILEHEADER *) mdlPak;

  /// For characters
  auto sgdTop = (SGDFILEHEADER *) GetFileInPak(mdlPak, 0);
  sgdRemap(sgdTop);
  
  while (!glfwWindowShouldClose(window))
  {    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    const float ratio = width / (float) height;
    
    context->Viewport(0, 0, width, height);
    context->Clear(GL_COLOR_BUFFER_BIT);
    
    /* Render here */    
    /// Simulate DrawGirl
    _gra3dDrawSGD(sgdTop, SRT_REALTIME, nullptr, -1);
    
    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }
  
  glfwDestroyWindow(window);
  glfwTerminate();
  
  return 0;
}
