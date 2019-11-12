#include "PotentialCalculator.h"
#include <optional>
#include <queue>

namespace
{
  using namespace TransportTask;

  void CalculatePotentialsAt(MatrixPotentials& i_potentials, const Matrix<double> &i_costs, const PairOf<SizeType>& indexes)
  {
    auto [row, column] = indexes;
    const bool is_valid_row = MatrixPotentials::IsValidPotential(i_potentials.m_rows[row]);
    const bool is_valid_column = MatrixPotentials::IsValidPotential(i_potentials.m_columns[column]);
    if (is_valid_row && !is_valid_column)
    {
      i_potentials.m_columns[column] = i_costs[row][column] - i_potentials.m_rows[row];
    }
    else
    {
      i_potentials.m_rows[row] = i_costs[row][column] - i_potentials.m_columns[column];
    }
  }
}

namespace TransportTask
{
  MatrixPotentials::MatrixPotentials(SizeType i_rows_count, SizeType i_columns_count)
    :m_rows(i_rows_count,empty_value)
    ,m_columns(i_columns_count, empty_value)
  {}

  bool MatrixPotentials::IsValidPotential(double i_potential)
  {
    return i_potential != empty_value;
  }

  double MatrixPotentials::PotentialAt(SizeType i_row, SizeType i_column) const
  {
    return m_rows[i_row] + m_columns[i_column];
  }

  bool MatrixPotentials::IsFullyCalculated() const
  {
    return std::all_of(m_rows.cbegin(), m_rows.cend(), IsValidPotential) &&
      std::all_of(m_columns.cbegin(), m_columns.cend(), IsValidPotential);
  }

  std::ostream& operator<<(std::ostream& output, const MatrixPotentials& potentials)
  {
    output << "\nU[i] = ";
    std::copy(potentials.m_rows.cbegin(), potentials.m_rows.cend(), std::ostream_iterator<double>{output, " "});
    output << "\nV[j] = ";
    std::copy(potentials.m_columns.cbegin(), potentials.m_columns.cend(), std::ostream_iterator<double>{output, " "});
    return output;
  }

  std::optional<MatrixPotentials> TransportTask::CalculatePotentials(const TransportTask::TransportInformation& i_data, const Matrix<double>& i_solution_matrix)
  {
    const SizeType rows_count = i_data.m_resources.size();
    const SizeType columns_count = i_data.m_requirements.size();
    MatrixPotentials potentials(rows_count, columns_count);
    Matrix<bool> visited(i_solution_matrix.size(), Vector<bool>(i_solution_matrix.front().size()));
    potentials.m_rows.front() = 0;
    std::queue<PairOf<SizeType>> processor_stack;
    for (SizeType j = 0; j < i_solution_matrix.front().size(); ++j)
    {
      if (i_solution_matrix.front()[j] != empty_value)
      {
        processor_stack.emplace(0, j);
      }
    }
    while (!processor_stack.empty())
    {
      auto [processed_row, processed_column] = processor_stack.front();
      processor_stack.pop();
      if (!visited[processed_row][processed_column])
      {
        CalculatePotentialsAt(potentials, i_data.m_costs_matrix, { processed_row, processed_column });
        visited[processed_row][processed_column] = true;
        for (SizeType row = 0; row < i_solution_matrix.size(); ++row)
        {
          if (i_solution_matrix[row][processed_column] != empty_value && !visited[row][processed_column] && row != processed_row)
          {
            const auto current_element = std::make_pair(row, processed_column);
            processor_stack.push(current_element);
          }
        }
        for (SizeType column = 0; column < i_solution_matrix[processed_row].size(); ++column)
        {
          if (i_solution_matrix[processed_row][column] != empty_value && !visited[processed_row][column] && column != processed_column)
          {
            const auto current_element = std::make_pair(processed_row, column);
            processor_stack.push(current_element);
          }
        }
      }
    }
    return potentials.IsFullyCalculated() ? potentials : std::optional<MatrixPotentials>{};
  }
}