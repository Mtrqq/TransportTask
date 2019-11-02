#pragma once
#include "Utility.h"
#include "TableCreator.h"

namespace TransportTask
{
  double GetOptimalSolution(const TaskData& i_task, std::ostream *o_logger = nullptr);
}