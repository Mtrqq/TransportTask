#pragma once
#include "Utility.h"
#include "ExportHeader.h"
#include <optional>
#include <iterator>
#include <algorithm>

namespace TransportTask
{
  struct MatrixPotentials
  {
    SOLVER_API MatrixPotentials(SizeType i_rows_count, SizeType i_columns_count);

    Vector<double> m_rows;
    Vector<double> m_columns;

    SOLVER_API static bool IsValidPotential(double i_potential);

    SOLVER_API double PotentialAt(SizeType i_row, SizeType i_column) const;

    SOLVER_API bool IsFullyCalculated() const;
  };

  SOLVER_API std::ostream& operator<<(std::ostream& output, const MatrixPotentials& potentials);

  SOLVER_API std::optional<MatrixPotentials> CalculatePotentials(const TransportTask::TransportInformation& i_data, const Matrix<double>& i_solution_matrix);
}