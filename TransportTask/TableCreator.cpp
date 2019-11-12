#include "TableCreator.h"
#include "PotentialCalculator.h"
#include <set>
#include <algorithm>
#include <numeric>

using namespace TransportTask;

namespace
{
  double GreedyInvestment(double i_required, double i_available)
  {
    return std::min(i_required, i_available);
  }

  struct VogelIndex
  {
    VogelIndex(SizeType i_index, double i_min_dif, bool i_is_row)
      :index{ i_index }
      , min_diff{ i_min_dif }
      , is_row{ i_is_row }
    {}

    SizeType index;
    double min_diff;
    bool is_row;
  };

  OptionalPair<SizeType> GetApproximationElementIndex(const Matrix<double> &i_costs_matrix,
    const Matrix<double> &i_edited_matrix,
    const Vector<double> &i_resources,
    const Vector<double> &i_requirements)
  {
    constexpr double infinity = std::numeric_limits<double>::infinity();
    Vector<VogelIndex> processed_elements;
    for (SizeType i = 0; i < i_costs_matrix.size(); ++i)// Foreach row
    {
      if (i_resources[i] != 0)
      {
        PairOf<double> row_diff{ infinity, infinity };
        for (SizeType j = 0; j < i_costs_matrix[i].size(); ++j)
        {
          if (i_requirements[j] != 0)
          {
            if (i_costs_matrix[i][j] < row_diff.first)
            {
              row_diff.second = row_diff.first;
              row_diff.first = i_costs_matrix[i][j];
            }
            else if (i_costs_matrix[i][j] < row_diff.second)
            {
              row_diff.second = i_costs_matrix[i][j];
            }
          }
        }
        if (row_diff.first != infinity)
        {
          if (row_diff.second == infinity)
            processed_elements.emplace_back(i, 0, true);
          else
            processed_elements.emplace_back(i, row_diff.second - row_diff.first, true);
        }
      }
    }

    for (SizeType j = 0; j < i_costs_matrix.front().size(); ++j)// Foreach column
    {
      if (i_requirements[j] != 0)
      {
        PairOf<double> column_diff{ infinity, infinity };
        for (SizeType i = 0; i < i_costs_matrix.size(); ++i)
        {
          if (i_resources[i] != 0)
          {
            if (i_costs_matrix[i][j] < column_diff.first)
            {
              column_diff.second = column_diff.first;
              column_diff.first = i_costs_matrix[i][j];
            }
            else if (i_costs_matrix[i][j] < column_diff.second)
            {
              column_diff.second = i_costs_matrix[i][j];
            }
          }
        }
        if (column_diff.first != infinity)
        {
          if (column_diff.second == infinity)
            processed_elements.emplace_back(j, 0, false);
          else
            processed_elements.emplace_back(j, column_diff.second - column_diff.first, false);
        }
      }
    }
    if (!processed_elements.empty())
    {
      const auto best_index = *(std::max_element(processed_elements.cbegin(), processed_elements.cend(), [](const VogelIndex &lhs, const VogelIndex &rhs)
      {
        return lhs.min_diff < rhs.min_diff;
      }));
      if (best_index.is_row)
      {
        SizeType min_index;
        double min_value = std::numeric_limits<double>::max();
        for (SizeType j = 0; j < i_costs_matrix.front().size(); ++j)
        {
          if (i_resources[best_index.index] != 0 && i_requirements[j] != 0 && i_costs_matrix[best_index.index][j] < min_value)
          {
            min_value = i_costs_matrix[best_index.index][j];
            min_index = j;
          }
        }
        return std::make_pair(best_index.index, min_index);
      }
      else
      {
        SizeType min_index;
        double min_value = std::numeric_limits<double>::max();
        for (SizeType i = 0; i < i_costs_matrix.size(); ++i)
        {
          if (i_resources[i] != 0 && i_requirements[best_index.index] != 0 && i_costs_matrix[i][best_index.index] < min_value)
          {
            min_value = i_costs_matrix[i][best_index.index];
            min_index = i;
          }
        }
        return std::make_pair(min_index, best_index.index);
      }
    }
    return {};
  }

  void VogelFormatter(Matrix<double>& io_edited_matrix, const TransportInformation &i_data)
  {
    auto resources = i_data.m_resources;
    auto requirements = i_data.m_requirements;
    auto indexes = GetApproximationElementIndex(i_data.m_costs_matrix, io_edited_matrix, resources, requirements);
    while (indexes)
    {
      auto[row, column] = indexes.value();
      const double investment = GreedyInvestment(requirements[column], resources[row]);
      if (investment == 0)
      {
        throw std::runtime_error{ "Something went wrong" };
      }
      io_edited_matrix[row][column] = investment;
      requirements[column] -= investment;
      resources[row] -= investment;
      indexes = GetApproximationElementIndex(i_data.m_costs_matrix, io_edited_matrix, resources, requirements);
    }
  }

  void MinimalCostFormatter(Matrix<double>& io_edited_matrix, const TransportInformation &i_data)
  {
    Vector<PairOf<SizeType>> min_cost_queue;
    auto resources = i_data.m_resources;
    auto requirements = i_data.m_requirements;
    min_cost_queue.reserve(requirements.size() * resources.size());
    for (SizeType i = 0; i < resources.size(); ++i)
    {
      for (SizeType j = 0; j < requirements.size(); ++j)
      {
        min_cost_queue.emplace_back(i, j);
      }
    }
    std::sort(min_cost_queue.begin(), min_cost_queue.end(), [&i_data](const PairOf<SizeType> &lhs, const PairOf<SizeType> &rhs)
    {
      return i_data.m_costs_matrix[lhs.first][lhs.second] < i_data.m_costs_matrix[rhs.first][rhs.second];
    });
    for (auto[row, column] : min_cost_queue)
    {
      if (resources[row] != 0 && requirements[column] != 0)
      {
        const double investment = GreedyInvestment(requirements[column], resources[row]);
        resources[row] -= investment;
        requirements[column] -= investment;
        io_edited_matrix[row][column] = investment;
      }
    }
  }

  void NorthWestFormatter(Matrix<double>& io_edited_matrix, const Vector<double>& i_resources, const Vector<double>& i_requirements)
  {
    auto resources = i_requirements;
    const SizeType processed_height = io_edited_matrix.size();
    const SizeType processed_width = io_edited_matrix.front().size();
    SizeType processed_column_index = 0;
    double wasted_for_current_row = 0;
    for (SizeType row = 0; row < processed_height; ++row)
    {
      const double actual_resource = i_resources[row];
      while (wasted_for_current_row < actual_resource && processed_column_index < processed_width)
      {
        const double investment = GreedyInvestment(actual_resource - wasted_for_current_row, resources[processed_column_index]);
        io_edited_matrix[row][processed_column_index] = investment;
        wasted_for_current_row += investment;
        resources[processed_column_index] -= investment;
        ++processed_column_index;
      }
      --processed_column_index;
      wasted_for_current_row = 0;
    }
  }

  bool EliminateDegeneracy(Matrix<double> &io_formatted_matrix, const TransportInformation &i_data)
  {
    SizeType count_of_filled_elements = 0;
    for (const auto& row : io_formatted_matrix)
    {
      for (double value : row)
      {
        if (value != empty_value)
        {
          ++count_of_filled_elements;
        }
      }
    }
    const SizeType sources_count = io_formatted_matrix.size();
    const SizeType clients_count = io_formatted_matrix.front().size();
    if (count_of_filled_elements != sources_count + clients_count - 1)
    {
      std::set<PairOf<SizeType>> used_combinations;
      const SizeType available_combinations_count = sources_count * clients_count - count_of_filled_elements - 1;
      while (used_combinations.size() != available_combinations_count)
      {
        auto checked_indexes = std::make_pair(rand() % sources_count, rand() % clients_count);
        auto it = used_combinations.find(checked_indexes);
        auto[row, column] = checked_indexes;
        if (io_formatted_matrix[row][column] == empty_value && it == used_combinations.end())
        {
          io_formatted_matrix[row][column] = 0.0;
          if (CalculatePotentials(i_data, io_formatted_matrix))
            return true;
          io_formatted_matrix[row][column] = empty_value;
          used_combinations.insert(it, checked_indexes);
        }
      }
      return false;
    }
    return true;
  }
}

namespace TransportTask
{
  CreationMethod ReadCreationMethod(std::istream & i_input, std::ostream * o_logger)
  {
    OptionalOutputStream output{ o_logger };
    output << "Select creation method :\n";
    constexpr int last_enum_element = static_cast<int>(CreationMethod::LAST);
    for (int i = 0; i != last_enum_element; ++i)
    {
      output << i + 1 << "." << GetMethodName(static_cast<CreationMethod>(i)) << '\n';
    }
    output << "Answer :";
    for (;;)
    {
      int selected_method_index;
      i_input >> selected_method_index;
      if (selected_method_index <= last_enum_element && selected_method_index > 0)
      {
        return static_cast<CreationMethod>(selected_method_index - 1);
      }
      output << "Enter valid answer :";
    }
  }

  std::string GetMethodName(CreationMethod i_method)
  {
    switch (i_method)
    {
    case TransportTask::CreationMethod::NorthWestAngle:
      return "North-west angle";
    case TransportTask::CreationMethod::MinimalCost:
      return "Minimal cost";
    case TransportTask::CreationMethod::VogelApproximation:
      return "Vogel approximation";
    }
    throw std::runtime_error{ "Undefined creation method" };
  }

  Matrix<double> FormatTask(const TransportInformation &i_data, CreationMethod i_method)
  {
    _ASSERT(i_data.m_resources.size() != 0 && i_data.m_requirements.size() != 0);
    Matrix<double> formatted_matrix(i_data.m_resources.size(), Vector<double>(i_data.m_requirements.size(), empty_value));
    switch (i_method)
    {
    case CreationMethod::NorthWestAngle:
      NorthWestFormatter(formatted_matrix, i_data.m_resources, i_data.m_requirements);
      break;
    case CreationMethod::MinimalCost:
      MinimalCostFormatter(formatted_matrix, i_data);
      break;
    case CreationMethod::VogelApproximation:
      VogelFormatter(formatted_matrix, i_data);
      break;
    }
    if (!EliminateDegeneracy(formatted_matrix, i_data))
      throw std::runtime_error{ "Elimination of degeneracy failed !" };

    return formatted_matrix;
  }
}