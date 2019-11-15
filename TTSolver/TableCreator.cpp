#include "pch.h"
#include "TableCreator.h"
#include "PotentialCalculator.h"
#include <set>
#include <algorithm>
#include <numeric>

using namespace TransportTask;

namespace
{
  constexpr double infinity = std::numeric_limits<double>::infinity();

  double GreedyInvestmentAmount(double i_required, double i_available)
  {
    return std::min(i_required, i_available);
  }

  void MakeGreedyInvestment(Matrix<double>& io_formatted_matrix, Vector<double>& io_requirements, Vector<double>& io_resources, const PairOf<SizeType>& index_pair)
  {
    auto [row, column] = index_pair;
    const double investment = GreedyInvestmentAmount(io_requirements[column], io_resources[row]);
    io_formatted_matrix[row][column] = investment;
    io_requirements[column] -= investment;
    io_resources[row] -= investment;
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

  PairOf<SizeType> GetBestIndexPair(const Vector<VogelIndex>& i_filtered_indexes,
                                    const Matrix<double> &i_costs_matrix,
                                    const Vector<double> &i_requirements,
                                    const Vector<double> &i_resources)
  {
    PairOf<SizeType> pivot_indexes;
    double best_value = infinity;
    for (const auto& element_description : i_filtered_indexes)
    {
      if (element_description.is_row)
      {
        const SizeType row_index = element_description.index;
        SizeType min_index;
        double min_value = infinity;
        for (SizeType j = 0; j < i_costs_matrix.front().size(); ++j)
        {
          if (i_resources[row_index] != 0 && i_requirements[j] != 0 && i_costs_matrix[row_index][j] < min_value)
          {
            min_value = i_costs_matrix[row_index][j];
            min_index = j;
          }
        }
        if (best_value > min_value)
        {
          pivot_indexes = std::make_pair(row_index, min_index);
          best_value = min_value;
        }
      }
      else
      {
        const SizeType column_index = element_description.index;
        SizeType min_row_index;
        double min_value = infinity;
        for (SizeType i = 0; i < i_costs_matrix.size(); ++i)
        {
          if (i_resources[i] != 0 && i_requirements[column_index] != 0 && i_costs_matrix[i][column_index] < min_value)
          {
            min_value = i_costs_matrix[i][column_index];
            min_row_index = i;
          }
        }
        if (best_value > min_value)
        {
          pivot_indexes = std::make_pair(min_row_index, column_index);
          best_value = min_value;
        }
      }
    }
    return pivot_indexes;
  }

  OptionalPair<SizeType> GetApproximationElementIndex(const Matrix<double>& i_costs_matrix,
                                                      const Vector<double>& i_resources,
                                                      const Vector<double>& i_requirements)
  {
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

    for (SizeType j = 0; j < i_costs_matrix.front().size(); ++j)
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
      const double max_diff = std::max_element(processed_elements.cbegin(), processed_elements.cend(), [](const VogelIndex& lhs, const VogelIndex& rhs)
      {
        return lhs.min_diff < rhs.min_diff;
      })->min_diff;
      auto first_invalid = std::remove_if(processed_elements.begin(), processed_elements.end(), [&max_diff](const VogelIndex& index)
      {
        return index.min_diff < max_diff;
      });
      processed_elements.erase(first_invalid, processed_elements.end());
      return GetBestIndexPair(processed_elements, i_costs_matrix, i_requirements, i_resources);
    }
    return std::nullopt;
  }

  void VogelFormatter(Matrix<double>& io_edited_matrix, const TransportInformation& i_data)
  {
    auto resources = i_data.m_resources;
    auto requirements = i_data.m_requirements;
    auto indexes = GetApproximationElementIndex(i_data.m_costs_matrix, resources, requirements);
    while (indexes)
    {
      MakeGreedyInvestment(io_edited_matrix, requirements, resources, indexes.value());
      indexes = GetApproximationElementIndex(i_data.m_costs_matrix, resources, requirements);
    }
  }

  void MinimalCostFormatter(Matrix<double>& io_edited_matrix, const TransportInformation& i_data)
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
    std::sort(min_cost_queue.begin(), min_cost_queue.end(), [&i_data](const PairOf<SizeType>& lhs, const PairOf<SizeType>& rhs)
      {
        return i_data.m_costs_matrix[lhs.first][lhs.second] < i_data.m_costs_matrix[rhs.first][rhs.second];
      });
    for (auto element_indexes : min_cost_queue)
    {
      if (resources[element_indexes.first] != 0 && requirements[element_indexes.second] != 0)
      {
        MakeGreedyInvestment(io_edited_matrix, requirements, resources, element_indexes);
      }
    }
  }

  void MarkMinimalElements(Matrix<int>& io_marks_matrix, const Matrix<double> &i_costs_matrix)
  {
    const SizeType rows_count = io_marks_matrix.size();
    const SizeType columns_count = io_marks_matrix.front().size();
    for (SizeType i = 0; i < rows_count; ++i)
    {
      SizeType min_index = 0;
      for (SizeType j = 1; j < columns_count; ++j)
      {
        if (i_costs_matrix[i][j] < i_costs_matrix[i][min_index])
          min_index = j;
      }
      ++io_marks_matrix[i][min_index];
    }

    for (SizeType j = 0; j < columns_count; ++j)
    {
      SizeType min_index = 0;
      for (SizeType i = 1; i < rows_count; ++i)
      {
        if (i_costs_matrix[i][j] < i_costs_matrix[min_index][j])
          min_index = i;
      }
      ++io_marks_matrix[min_index][j];
    }
  }

  void DoubleMarksFormatter(Matrix<double>& io_edited_matrix, const TransportInformation& i_data)
  {
    const SizeType rows_count = io_edited_matrix.size();
    const SizeType columns_count = io_edited_matrix.front().size();
    Matrix<int> marks_matrix(rows_count, Vector<int>(columns_count));
    MarkMinimalElements(marks_matrix, i_data.m_costs_matrix);
    using ElementInfo = std::pair<PairOf<SizeType>, int>;
    Vector<ElementInfo> processed_elements;
    Vector<PairOf<SizeType>> left_indexes;
    for (SizeType i = 0; i < rows_count; ++i)
    {
      for (SizeType j = 0; j < columns_count; ++j)
      {
        if (marks_matrix[i][j] != 0)
        {
          processed_elements.emplace_back(std::make_pair(i, j), marks_matrix[i][j]);
        } 
        else
        {
          left_indexes.emplace_back(i, j);
        }
      }
    }
    const auto& costs_matrix = i_data.m_costs_matrix;
    std::sort(processed_elements.begin(), processed_elements.end(), [&costs_matrix](const ElementInfo& lhs, const ElementInfo& rhs)
      {
        return lhs.second > rhs.second || (lhs.second == rhs.second
          && costs_matrix[lhs.first.first][lhs.first.second] < costs_matrix[rhs.first.first][rhs.first.second]);
      });
    auto requirements = i_data.m_requirements;
    auto resources = i_data.m_resources;
    for (auto& element_description : processed_elements)
    {
      auto& index_pair = element_description.first;
      if (resources[index_pair.first] != 0.0 && requirements[index_pair.second] != 0.0)
      {
        MakeGreedyInvestment(io_edited_matrix, requirements, resources, element_description.first);
      }
    }
    std::sort(left_indexes.begin(), left_indexes.end(), [&costs_matrix](const PairOf<SizeType>& lhs, const PairOf<SizeType>& rhs)
    {
        return costs_matrix[lhs.first][lhs.second] < costs_matrix[rhs.first][rhs.second];
    });
    for (auto& index_pair : left_indexes)
    {
      if (resources[index_pair.first] != 0.0 && requirements[index_pair.second] != 0.0)
      {
        MakeGreedyInvestment(io_edited_matrix, requirements, resources, index_pair);
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
        const double investment = GreedyInvestmentAmount(actual_resource - wasted_for_current_row, resources[processed_column_index]);
        io_edited_matrix[row][processed_column_index] = investment;
        resources[processed_column_index] -= investment;
        wasted_for_current_row += investment;
        ++processed_column_index;
      }
      --processed_column_index;
      wasted_for_current_row = 0;
    }
  }

  bool EliminateDegeneracy(Matrix<double>& io_formatted_matrix, const TransportInformation& i_data)
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
    if (count_of_filled_elements == sources_count + clients_count - 2)
    {
      std::set<PairOf<SizeType>> used_combinations;
      const SizeType available_combinations_count = sources_count * clients_count - count_of_filled_elements - 1;
      while (used_combinations.size() != available_combinations_count)
      {
        auto checked_indexes = std::make_pair(rand() % sources_count, rand() % clients_count);
        auto it = used_combinations.find(checked_indexes);
        auto [row, column] = checked_indexes;
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
    else return count_of_filled_elements == sources_count + clients_count - 1;
  }
}

namespace TransportTask
{
  

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
    case TransportTask::CreationMethod::DoubleMarks:
      return "Double marks";
    }
    throw std::runtime_error{ "Undefined creation method" };
  }

  Matrix<double> FormatTask(const TransportInformation& i_data, CreationMethod i_method)
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
    case CreationMethod::DoubleMarks:
      DoubleMarksFormatter(formatted_matrix, i_data);
      break;
    }
    if (!EliminateDegeneracy(formatted_matrix, i_data))
      throw std::runtime_error{ "Elimination of degeneracy failed !" };

    return formatted_matrix;
  }
}