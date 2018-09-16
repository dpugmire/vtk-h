//-----------------------------------------------------------------------------
///
/// file: t_vtk-h_dataset.cpp
///
//-----------------------------------------------------------------------------

#include "gtest/gtest.h"

#include <vtkh/vtkh.hpp>
#include <vtkh/DataSet.hpp>
#include <vtkh/rendering/PointRenderer.hpp>
#include <vtkh/rendering/Scene.hpp>
#include "t_test_utils.hpp"

#include <iostream>



//----------------------------------------------------------------------------
TEST(vtkh_point_renderer, vtkh_point_render)
{
  vtkh::DataSet data_set;
 
  const int base_size = 16;
  const int num_blocks = 2; 
  
  for(int i = 0; i < num_blocks; ++i)
  {
    data_set.AddDomain(CreateTestData(i, num_blocks, base_size), i);
  }

  vtkm::Bounds bounds = data_set.GetGlobalBounds();

  vtkm::rendering::Camera camera;
  camera.ResetToBounds(bounds);
  camera.SetPosition(vtkm::Vec<vtkm::Float64,3>(16, 36, -36));
  vtkh::Render render = vtkh::MakeRender(512, 
                                         512, 
                                         camera, 
                                         data_set, 
                                         "render_points");  
  vtkh::PointRenderer renderer;
  renderer.SetInput(&data_set);
  renderer.SetField("point_data"); 

  

  vtkh::Scene scene; 
  scene.AddRenderer(&renderer);
  scene.AddRender(render);
  scene.Render();
 
}

TEST(vtkh_point_renderer, vtkh_variable_point_render)
{
  vtkh::DataSet data_set;
 
  const int base_size = 16;
  const int num_blocks = 2; 
  
  for(int i = 0; i < num_blocks; ++i)
  {
    data_set.AddDomain(CreateTestData(i, num_blocks, base_size), i);
  }

  vtkm::Bounds bounds = data_set.GetGlobalBounds();

  vtkm::rendering::Camera camera;
  camera.ResetToBounds(bounds);
  camera.SetPosition(vtkm::Vec<vtkm::Float64,3>(16, 36, -36));
  vtkh::Render render = vtkh::MakeRender(512, 
                                         512, 
                                         camera, 
                                         data_set, 
                                         "render_var_points");  
  vtkh::PointRenderer renderer;
  renderer.SetInput(&data_set);
  renderer.SetField("point_data"); 
  renderer.UseVariableRadius(true); 
  renderer.SetRadiusDelta(1.0f); 

  

  vtkh::Scene scene; 
  scene.AddRenderer(&renderer);
  scene.AddRender(render);
  scene.Render();
 
}