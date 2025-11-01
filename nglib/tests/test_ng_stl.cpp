#include <iostream>
#include <fstream>

namespace nglib
{
#include "../nglib.h"
}

int main (int argc, char** argv)
{
  using namespace nglib;

  // Define pointer to a new Netgen Mesh
  Ng_Mesh* mesh = nullptr;

  // Define pointer to STL Geometry
  Ng_STL_Geometry* stl_geom = nullptr;

  // Result of Netgen Operations
  Ng_Result ng_res;

  // Initialise the Netgen Core library
  Ng_Init();

  // Actually create the mesh structure
  mesh = Ng_NewMesh();

  int np, ne;

#if 1
  // Add simple geometry to stl_geom
  stl_geom = Ng_STL_NewGeometry();

  // Define cube vertices (unit cube from 0 to 1)
  double vertices[8][3] = {
    {0.0, 0.0, 0.0}, // 0: (0,0,0)
    {1.0, 0.0, 0.0}, // 1: (1,0,0)
    {1.0, 1.0, 0.0}, // 2: (1,1,0)
    {0.0, 1.0, 0.0}, // 3: (0,1,0)
    {0.0, 0.0, 1.0}, // 4: (0,0,1)
    {1.0, 0.0, 1.0}, // 5: (1,0,1)
    {1.0, 1.0, 1.0}, // 6: (1,1,1)
    {0.0, 1.0, 1.0}  // 7: (0,1,1)
  };

  // Add triangles for each face of the cube (12 triangles total)
  // Bottom face (z = 0) - triangles with outward normal (0,0,-1)
  Ng_STL_AddTriangle(stl_geom, vertices[0], vertices[2], vertices[1]);
  Ng_STL_AddTriangle(stl_geom, vertices[0], vertices[3], vertices[2]);

  // Top face (z = 1) - triangles with outward normal (0,0,1)
  Ng_STL_AddTriangle(stl_geom, vertices[4], vertices[5], vertices[6]);
  Ng_STL_AddTriangle(stl_geom, vertices[4], vertices[6], vertices[7]);

  // Front face (y = 0) - triangles with outward normal (0,-1,0)
  Ng_STL_AddTriangle(stl_geom, vertices[0], vertices[1], vertices[5]);
  Ng_STL_AddTriangle(stl_geom, vertices[0], vertices[5], vertices[4]);

  // Back face (y = 1) - triangles with outward normal (0,1,0)
  Ng_STL_AddTriangle(stl_geom, vertices[3], vertices[7], vertices[6]);
  Ng_STL_AddTriangle(stl_geom, vertices[3], vertices[6], vertices[2]);

  // Left face (x = 0) - triangles with outward normal (-1,0,0)
  Ng_STL_AddTriangle(stl_geom, vertices[0], vertices[4], vertices[7]);
  Ng_STL_AddTriangle(stl_geom, vertices[0], vertices[7], vertices[3]);

  // Right face (x = 1) - triangles with outward normal (1,0,0)
  Ng_STL_AddTriangle(stl_geom, vertices[1], vertices[2], vertices[6]);
  Ng_STL_AddTriangle(stl_geom, vertices[1], vertices[6], vertices[5]);
#else
    stl_geom = Ng_STL_LoadGeometry("/Users/lloyd/Data/sphere.stl", true);
#endif

  // Set the Meshing Parameters to be used
  Ng_Meshing_Parameters mp;
  mp.maxh = 0.5;
  mp.fineness = 0.4;
  mp.second_order = 0;
  mp.optsteps_3d = 5;
  mp.optimize3d = "cmdmustm";

  std::cout << "Initialise the STL Geometry structure...." << std::endl;
  ng_res = Ng_STL_InitSTLGeometry(stl_geom);
  if (ng_res != NG_OK)
    {
      std::cout << "Error Initialising the STL Geometry....Aborting!!" << std::endl;
      return 1;
    }

  std::cout << "Start Edge Meshing...." << std::endl;
  ng_res = Ng_STL_MakeEdges(stl_geom, mesh, &mp);
  if (ng_res != NG_OK)
    {
      std::cout << "Error in Edge Meshing....Aborting!!" << std::endl;
      return 1;
    }

  std::cout << "Start Surface Meshing...." << std::endl;
  ng_res = Ng_STL_GenerateSurfaceMesh(stl_geom, mesh, &mp);
  if (ng_res != NG_OK)
    {
      std::cout << "Error in Surface Meshing....Aborting!!" << std::endl;
      return 1;
    }

  std::cout << "Start Volume Meshing...." << std::endl;
  ng_res = Ng_GenerateVolumeMesh(mesh, &mp);
  if (ng_res != NG_OK)
    {
      std::cout << "Error in Volume Meshing....Aborting!!" << std::endl;
      return 1;
    }

  std::cout << "Meshing successfully completed....!!" << std::endl;

  // volume mesh output
  np = Ng_GetNP(mesh);
  std::cout << "Points: " << np << std::endl;

  ne = Ng_GetNE(mesh);
  std::cout << "Elements: " << ne << std::endl;

  std::cout << "Saving Mesh in VOL Format...." << std::endl;
  Ng_SaveMesh(mesh, "test.vol");

  Ng_STL_Uniform_Refinement(stl_geom, mesh);
  Ng_OptimizeVolume(mesh, &mp);

  std::cout << "Refinement successfully completed....!!" << std::endl;
  np = Ng_GetNP(mesh);
  std::cout << "Points: " << np << std::endl;

  ne = Ng_GetNE(mesh);
  std::cout << "Elements: " << ne << std::endl;

  Ng_STL_DeleteGeometry(stl_geom);
  Ng_DeleteMesh(mesh);
  Ng_Exit();

  return 0;
}
