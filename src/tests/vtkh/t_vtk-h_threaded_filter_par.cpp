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

#include <vtkm/io/VTKDataSetReader.h>

#include <iostream>
#include <mpi.h>

int rank = 0, numRanks = 0;

vtkh::DataSet ReadVisItFile(const std::string& dir, const std::string& fname)
{
  std::ifstream visitFile;
  visitFile.open(dir + "/" + fname);

  vtkh::DataSet dataSets;

  std::string f;
  std::vector<std::string> fileNames;
  int cnt = 0;
  while (visitFile >> f)
  {
    if (cnt > 1) //skip !NBLOCKS N
      fileNames.push_back(f);
    cnt++;
  }

  int numDS = fileNames.size();
  int numPerRank = numDS / numRanks;
  int b0 = rank*numPerRank;
  int b1 = b0 + numPerRank;
  int leftOver = numDS % numRanks;

  std::vector<int> domainIds;
  std::vector<std::string> myFiles;
  for (int i = b0; i < b1; i++)
  {
    myFiles.push_back(fileNames[i]);
    domainIds.push_back(i);
  }
  if (leftOver > 0 && rank < leftOver)
  {
    myFiles.push_back(fileNames[numPerRank*numRanks + rank]);
    domainIds.push_back(numPerRank*numRanks + rank);
  }

  for (int i = 0; i < myFiles.size(); i++)
  {
    vtkm::io::VTKDataSetReader reader(dir + "/" + myFiles[i]);
    auto ds = reader.ReadDataSet();
    dataSets.AddDomain(ds, domainIds[i]);
  }
  return dataSets;
}

//----------------------------------------------------------------------------
TEST(vtkh_threaded_filter, vtkh_threaded_filter)
{
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numRanks);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  vtkh::SetMPICommHandle(MPI_Comm_c2f(MPI_COMM_WORLD));

  auto dataSets = ReadVisItFile("../data", "chem.1033.visit");

  vtkh::MarchingCubes marcher;
  marcher.SetThreadTask(3);
  marcher.SetInput(&dataSets);
  marcher.SetField("temp_point");
  marcher.AddMapField("temp_point");

    double vals[1] = {850};
  marcher.SetIsoValues(vals, 1);
  marcher.Update();

  vtkh::DataSet *iso = marcher.GetOutput();

  vtkm::Bounds bounds = iso->GetGlobalBounds();
  float bg_color[4] = { 0.f, 0.f, 0.f, 1.f};
  vtkm::rendering::Camera camera;
  camera.ResetToBounds(bounds);
  vtkh::Render render = vtkh::MakeRender(512,
                                         512,
                                         camera,
                                         *iso,
                                         "iso",
                                          bg_color);
  vtkh::RayTracer photonHucker;
  photonHucker.SetInput(iso);
  photonHucker.SetField("temp_point");

  vtkh::Scene scene;
  scene.AddRenderer(&photonHucker);
  scene.AddRender(render);
  scene.Render();

  delete iso;

  MPI_Finalize();

#if 0
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
  marcher.SetInput(&dataSets);
  marcher.SetField("point_data_Float64");
  double vals[1] = {850};
  marcher.SetIsoValues(vals, 1);
  marcher.AddMapField("point_data_Float64");
  marcher.Update();

  vtkh::DataSet *iso = marcher.GetOutput();

  /*
  const int num_vals = 1;
  double iso_vals [num_vals];
  iso_vals[0] = (float)base_size * (float)num_blocks * 0.5f;

  marcher.SetIsoValues(iso_vals, num_vals);
  marcher.AddMapField("point_data_Float64");
  marcher.AddMapField("cell_data_Float64");
  marcher.Update();
  */


  /*
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
  */
#endif
}
