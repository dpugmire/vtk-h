//-----------------------------------------------------------------------------
///
/// file: t_vtk-h_dataset.cpp
///
//-----------------------------------------------------------------------------

#include "gtest/gtest.h"

#include <vtkh/vtkh.hpp>
#include <vtkh/DataSet.hpp>
#include <vtkh/filters/MarchingCubes.hpp>
#include <vtkh/rendering/RayTracer.hpp>
#include <vtkh/rendering/Scene.hpp>
#include "t_test_utils.hpp"

#include <iostream>

vtkh::DataSet ReadVisItFile(const std::string& fname)
{

  std::ifstream visitFile;
  visitFile.open(fname);

  vtkh::DataSet dataSets;

  std::string f;
  std::vector<std::string> fileNames;
  while (visitFile >> f)
    fileNames.push_back(f);

  std::cout<<"VISITFILE: "<<fileNames.size()<<std::endl;
  return dataSets;
}

//----------------------------------------------------------------------------
TEST(vtkh_threaded_filter, vtkh_threaded_filter)
{

  auto bum = ReadVisItFile("../data/chem.1033.visit");
  vtkh::DataSet data_set;

  const int base_size = 32;
  const int num_blocks = 5;

  std::cout<<__FILE__<<" "<<__LINE__<<std::endl;
  for(int i = 0; i < num_blocks; ++i)
  {
    data_set.AddDomain(CreateTestData(i, num_blocks, base_size), i);
  }
  std::cout<<__FILE__<<" "<<__LINE__<<std::endl;

  vtkh::MarchingCubes marcher;
  marcher.SetThreadTask(3);
  marcher.SetInput(&data_set);
  marcher.SetField("point_data_Float64");

  const int num_vals = 1;
  double iso_vals [num_vals];
  iso_vals[0] = (float)base_size * (float)num_blocks * 0.5f;

  marcher.SetIsoValues(iso_vals, num_vals);
  marcher.AddMapField("point_data_Float64");
  marcher.AddMapField("cell_data_Float64");
  marcher.Update();

  vtkh::DataSet *iso_output = marcher.GetOutput();

  vtkm::Bounds bounds = iso_output->GetGlobalBounds();
  float bg_color[4] = { 0.f, 0.f, 0.f, 1.f};
  vtkm::rendering::Camera camera;
  camera.ResetToBounds(bounds);
  vtkh::Render render = vtkh::MakeRender(512,
                                         512,
                                         camera,
                                         *iso_output,
                                         "iso",
                                          bg_color);
  vtkh::RayTracer photonHucker;
  photonHucker.SetInput(iso_output);
  photonHucker.SetField("cell_data_Float64");

  vtkh::Scene scene;
  scene.AddRenderer(&photonHucker);
  scene.AddRender(render);
  scene.Render();

  delete iso_output;
}
