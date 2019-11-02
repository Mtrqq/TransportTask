#pragma once
#include "Utility.h"
#include <optional>
#include <iterator>
#include <algorithm>

namespace TransportTask
{
  struct MatrixPotentials
  {
    MatrixPotentials(SizeType i_rows_count, SizeType i_columns_count);

    Vector<double> m_rows;
    Vector<double> m_columns;

    static bool IsValidPotential(double i_potential);

    double PotentialAt(SizeType i_row, SizeType i_column) const;

    bool IsFullyCalculated() const;
  };

  std::ostream& operator<<(std::ostream& output, const MatrixPotentials& potentials);

  std::optional<MatrixPotentials> CalculatePotentials(const TransportTask::TaskData& i_data, const Matrix<double>& i_solution_matrix);
}