#include <iostream>
#include <fstream>

namespace nglib {
#include "../nglib/nglib.h"
}

void generateMesh2D()
{
  using namespace std;
  using namespace nglib;

  Ng_Init(false, false, false);

  auto geom = Ng_NewGeometry_2D();

  double points[][3] = {
    {0,0,0},
    {1,0,0},
    {0,1,0},
    {-1,0,0},
  };
  double h = 2;

  for (int i=0; i<4; ++i)
    Ng_AppendPoint_2D(geom, points[i], h);
  
  //Ng_AppendLineSegment_2D(geom, 1, 2, 1, -1, h);
  //Ng_AppendLineSegment_2D(geom, 2, 3, 1, -1, h);
  Ng_AppendSplinSegment_2D(geom, 1, 2, 3, 1, -1, h);
  Ng_AppendLineSegment_2D(geom, 3, 1, 1, 2, h);
  Ng_AppendLineSegment_2D(geom, 3, 4, 2, -1, h);
  Ng_AppendLineSegment_2D(geom, 4, 1, 2, -1, h);

  Ng_Mesh * mesh = nullptr;
  Ng_Meshing_Parameters mp;
  mp.uselocalh = false;
  mp.maxh = 10;
  mp.minh = 0.5;
  mp.elementsperedge = 2;
  mp.elementspercurve = 5;
  mp.uselocalh = true;
  mp.grading = 0.9;

  auto ok = Ng_GenerateMesh_2D(geom, &mesh, &mp);

  cout << "Points: " << Ng_GetNP(mesh) << endl;
  cout << "Elements: " << Ng_GetNSE(mesh) << endl;

  Ng_ExportMesh(mesh, NG_VTK, "/Users/lloyd/temp/_spline_geom.vtk");

  Ng_DeleteGeometry_2D(geom);
  Ng_DeleteMesh(mesh);
  Ng_Exit();
}

int main (int argc, char ** argv)
{
  generateMesh2D();
  return 0;

  using namespace std;
  using namespace nglib;
  
  Ng_Init();

  Ng_Mesh * mesh = Ng_NewMesh ();

#if 0
  int np = 5;
  double points[5][3] = {
    {0,0,0},
    {1,0,0},
    {1,1,0},
    {1,1,-0.6},
    {1,1,0.6}
  };
  int ne = 2;
  Ng_Volume_Element_Type type = NG_TET;
  int elems[2][4] = {
    {1,2,3,4},
    {1,3,2,5}
  };
#else
  int np = 12;
  double points[12][3] = {
    {0,0,0},
    {1,0,0},
    {1,1,0},
    {0.5,-1,0},
    {0,0,1},
    {1,0,1},
    {1,1,1},
    {0.5,-1,1},
    {1,1,2},
    {0,0,-1},
    {1,0,-1},
    {1,1,-1},
  };
  int ne = 3;
  Ng_Volume_Element_Type type = NG_PRISM;
  int elems[3][6] = {
    {1,2,3,5,6,7},
    {1,4,2,5,8,6},
    {10,11,12,1,2,3}
  };
#endif

  for (int i = 0; i < np; i++)
  {
    double *point = points[i];
    Ng_AddPoint (mesh, point);
  }

  for (int i = 0; i < ne; i++)
  {
    int *el = elems[i];
    Ng_AddVolumeElement (mesh, type, el);
  }
  int tet[] = {5,6,7, 9};
  Ng_AddVolumeElement (mesh, NG_TET, tet);

  if (false)
  {
    Ng_Uniform_Refinement(mesh);
  }
  else
  {
  for (int i = 0; i < ne; i++)
    {
      int *el = elems[i];
      int tri1[] = {el[0], el[1], el[2]};
      Ng_AddSurfaceElement(mesh, NG_TRIG, tri1);
      int tri2[] = {el[3], el[5], el[4]};
      Ng_AddSurfaceElement(mesh, NG_TRIG, tri2);
    }
    {
      int tri2[] = {6,7,9};
      Ng_AddSurfaceElement(mesh, NG_TRIG, tri2);
    }

    int nse = Ng_GetNSE(mesh);
    std::cerr << "nse: " << nse << "\n";
    for (int i = 1; i<=nse; ++i)
      Ng_SetSurfaceRefinementFlag (mesh, i, 0);

    Ng_SetSurfaceRefinementFlag (mesh, nse, 1);
    Ng_SetRefinementFlag (mesh, 1, 0);
    Ng_SetRefinementFlag (mesh, 2, 0);
    Ng_SetRefinementFlag (mesh, 3, 0);
    Ng_SetRefinementFlag (mesh, 4, 0);
    Ng_Refine(mesh);

    Ng_ExportMesh(mesh, Ng_Export_Formats::NG_VTK, "/Users/lloyd/_mesh.vtk");
    //Ng_Meshing_Parameters opt;
    //Ng_OptimizeVolume(mesh, &opt);
  }


  // volume mesh output
  cout << "Points: " << Ng_GetNP(mesh) << endl;
  cout << "Elements: " << Ng_GetNE(mesh) << endl;

  Ng_DeleteMesh(mesh);
  Ng_Exit();

  return 0;
}
