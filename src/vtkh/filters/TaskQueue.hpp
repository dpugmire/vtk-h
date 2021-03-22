//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================
#ifndef vtk_h_filter_TaskQueue_h
#define vtk_h_filter_TaskQueue_h

#include <vtkm/cont/PartitionedDataSet.h>
#include <vtkm/cont/DataSet.h>

#include <thread>
#include <queue>

namespace vtkh
{
template <typename T>
class TaskQueue
{
public:
  TaskQueue() = default;

  //Add a task to the Queue.
  void Push(T&& item)
  {
    std::unique_lock<std::mutex> lock(this->Lock);
    if (this->Queue.empty())
      this->Index = 0;
    else
      this->Index++;

    this->Queue.push(item);
  }

  bool HasTasks()
  {
    std::unique_lock<std::mutex> lock(this->Lock);
    return !(this->Queue.empty());
  }

  bool GetTask(T& item)
  {
    std::unique_lock<std::mutex> lock(this->Lock);
    if (this->Queue.empty())
      return false;

    item = this->Queue.front();
    this->Queue.pop();
    return true;
  }

  T Pop()
  {
    T item;
    std::unique_lock<std::mutex> lock(this->Lock);
    if (!this->Queue.empty())
    {
      item = this->Queue.front();
      this->Queue.pop();
    }

    return item;
  }

private:
  std::mutex Lock;
  std::queue<T> Queue;
  vtkm::Id Index = -1;

  //don't want copies of this
  TaskQueue(const TaskQueue& rhs) = delete;
  TaskQueue& operator=(const TaskQueue& rhs) = delete;
  TaskQueue(TaskQueue&& rhs) = delete;
  TaskQueue& operator=(TaskQueue&& rhs) = delete;
};


class DataSetQueue : public TaskQueue<vtkm::cont::DataSet>
{
public:
  DataSetQueue(vtkh::DataSet& input)
  {
    const int numDoms = input.GetNumberOfDomains();
    for (int i = 0; i < numDoms; i++)
    {
      vtkm::cont::DataSet ds;
      vtkm::Id id;

      input.GetDomain(i, ds, id);
      this->Push(std::move(ds));
    }
  }
/*
  DataSetQueue(const vtkm::cont::PartitionedDataSet& input)
  {
    for (auto ds : input)
      this->Push(std::move(ds));
  }
*/

  DataSetQueue() {}

  vtkh::DataSet Get()
  {
    vtkh::DataSet pds;

    vtkm::cont::DataSet ds;
    vtkm::Id id = 0;
    while (this->GetTask(ds))
      pds.AddDomain(ds, id++);

    return pds;
  }

private:
};

}

#endif
