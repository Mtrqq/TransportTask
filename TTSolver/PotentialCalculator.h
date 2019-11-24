#pragma once
#include "Utility.h"
#include "ExportHeader.h"
#include <optional>

namespace TransportTask
{
  SOLVER_API std::optional<MatrixPotentials> CalculatePotentials(const TransportTask::TransportInformation& i_data, const Matrix<double>& i_solution_matrix);
}