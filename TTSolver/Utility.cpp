#include "pch.h"
#include "Utility.h"
#include <sstream>
#include <algorithm>
#include <numeric>

namespace TransportTask
{
  TransportInformation::TransportInformation(const Matrix<double>& i_cost, const Vector<double> i_resources, const Vector<double> i_requirements)
    :m_costs_matrix{ i_cost }
    ,m_requirements{ i_requirements }
    ,m_resources{ i_resources }
  {
    const double requirements_sum = std::accumulate(m_requirements.cbegin(), m_requirements.cend(), 0.0);
    const double resources_sum = std::accumulate(m_resources.cbegin(), m_resources.cend(), 0.0);
    if (resources_sum > requirements_sum)
    {
      m_state = ResourcesState::Overflow;
      m_requirements.push_back(resources_sum - requirements_sum);
      for (auto& row : m_costs_matrix)
        row.push_back(0);
    }
    else if (requirements_sum > resources_sum)
    {
      m_state = ResourcesState::Sufficient;
      m_resources.push_back(requirements_sum - resources_sum);
      m_costs_matrix.emplace_back(m_costs_matrix.front().size(), 0.0);
    }
  }

  std::optional<std::string> TransportInformation::GetMessageForState() const
  {
    if (m_state != ResourcesState::Normal)
    {
      if (m_state == ResourcesState::Overflow)
      {
        return "Last client is fictive and indicates unused resources";
      }
      else
      {
        return "Last source is fictive and indicates sufficient resources";
      }
    }
    return std::nullopt;
  }

  Vector<std::string> GetResoucesDistributionDetails(const Matrix<double>& i_feasible_solution)
  {
    Vector<std::string> distribution_log;
    for (SizeType i = 0; i < i_feasible_solution.size(); ++i)
    {
      for (SizeType j = 0; j < i_feasible_solution[i].size(); ++j)
      {
        const double resource_amount = i_feasible_solution[i][j];
        if (resource_amount != empty_value && resource_amount != 0.0)
        {
          std::ostringstream stream;
          stream << "Transport " <<resource_amount << " units of resource from warehouse #" << i + 1 << " to client #" << j + 1;
          distribution_log.push_back(stream.str());
        }
      }
    }
    return distribution_log;
  }

  MatrixPotentials::MatrixPotentials(SizeType i_rows_count, SizeType i_columns_count)
    :m_rows(i_rows_count, empty_value)
    , m_columns(i_columns_count, empty_value)
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
    return std::all_of(m_rows.cbegin(), m_rows.cend(), IsValidPotential) && std::all_of(m_columns.cbegin(), m_columns.cend(), IsValidPotential);
  }

  double CalculateTransportPrice(const Matrix<double>& i_actual_solution, const Matrix<double>& i_costs)
  {
    const SizeType rows_count = i_actual_solution.size();
    const SizeType columns_count = i_actual_solution.front().size();
    double accumulation = 0;
    for (SizeType row = 0; row < rows_count; ++row)
    {
      for (SizeType column = 0; column < columns_count; ++column)
      {
        const double current_element = i_actual_solution[row][column];
        if (current_element != empty_value)
        {
          accumulation += current_element * i_costs[row][column];
        }
      }
    }
    return accumulation;
  }

  std::string GetStepDescription(const SolutionInfo& prepared_solution, SizeType step_index)
  {
    std::ostringstream string_stream;
    if (step_index == 0)
    {
      string_stream << "Formed initial feasible solution";
    }
    else
    {
      auto [pivot_row, pivot_column] = prepared_solution.rebuilding_pivots[step_index - 1];
      string_stream << "Got " << step_index + 1 << " feasible solution after rebuilding previous matrix with pivot element at index ["
        << pivot_row + 1 << ", " << pivot_column + 1 << "]";
    }

    /*if (step_index == prepared_solution.solution_steps.size() - 1)
    {
      string_stream << "\nNone of ignored elements doesn't violates potentiality, solution recognized as optimal";
    }*/
    return string_stream.str();
  }
  
  SizeType SolutionInfo::GetIterationsCount() const
  {
    return solution_steps.size();
  }
  
  double SolutionInfo::GetMatrixCostAtStep(SizeType step_index, const Matrix<double>& costs_matrix) const
  {
    return CalculateTransportPrice(solution_steps[step_index], costs_matrix);
  }

  SizeType SolutionInfo::GetAmountOfBytesSpent() const
  {
    const SizeType iterations_count = GetIterationsCount();
    const SizeType rows_count = potentials.front().m_rows.size();
    const SizeType columns_count = potentials.front().m_columns.size();

    const SizeType spent_for_matrices = iterations_count * rows_count * columns_count * sizeof(double);
    const SizeType spent_for_potentials = potentials.size() * (rows_count + columns_count) * sizeof(double);
    const SizeType spent_for_pivots = 2 * rebuilding_pivots.size() * sizeof(SizeType);

    return spent_for_matrices + spent_for_pivots + spent_for_potentials;
  }
}