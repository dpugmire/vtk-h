//-----------------------------------------------------------------------------
///
/// file: t_vtk-h_dataset.cpp
///
//-----------------------------------------------------------------------------

#include "gtest/gtest.h"

#include <vtkh/vtkh.hpp>
#include <vtkh/DataSet.hpp>
#include <vtkh/filters/Log.hpp>
#include <vtkh/rendering/RayTracer.hpp>
#include <vtkh/rendering/Scene.hpp>
#include "t_test_utils.hpp"

#include <iostream>



//----------------------------------------------------------------------------
TEST(vtkh_log, vtkh_log)
{
  vtkh::DataSet data_set;
 
  const int base_size = 32;
  const int num_blocks = 2; 
  
  for(int i = 0; i < num_blocks; ++i)
  {
    data_set.AddDomain(CreateTestData(i, num_blocks, base_size), i);
  }

  vtkh::Log logger;
  logger.SetInput(&data_set);
  logger.SetField("point_data"); 
  logger.SetResultField("log(point_data)"); 

  logger.Update();
  vtkh::DataSet *output = logger.GetOutput();
  vtkm::Bounds bounds = output->GetGlobalBounds();

  vtkm::rendering::Camera camera;
  camera.SetPosition(vtkm::Vec<vtkm::Float64,3>(-16, -16, -16));
  camera.ResetToBounds(bounds);
  vtkh::Render render = vtkh::MakeRender(512, 
                                         512, 
                                         camera, 
                                         *output, 
                                         "log");  
  vtkh::RayTracer tracer;
  tracer.SetInput(output);
  tracer.SetField("log(point_data)"); 

  vtkh::Scene scene;
  scene.AddRender(render);
  scene.AddRenderer(&tracer);
  scene.Render();

  delete output; 
}
