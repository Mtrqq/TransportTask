#pragma once
#include "Utility.h"
#include "TableCreator.h"

namespace TransportTask
{
  double GetOptimalSolution(const TransportInformation& i_data,
                            CreationMethod i_method = CreationMethod::VogelApproximation,
                            std::ostream *o_logger = nullptr);
}