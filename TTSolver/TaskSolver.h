#pragma once
#include "Utility.h"
#include "TableCreator.h"
#include "ExportHeader.h"

namespace TransportTask
{
  SOLVER_API SolutionInfo GetOptimalSolution(const TransportInformation& i_data, CreationMethod i_method);
}