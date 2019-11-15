#include "pch.h"
#include "TaskSolver.h"
#include "PotentialCalculator.h"
#include <algorithm>
#include <string>

namespace
{
  using namespace TransportTask;

  bool MarkAnyCycle(const Matrix<double> &i_solution, Matrix<int>& io_marks, 
                    const PairOf<SizeType> &i_actual, const PairOf<SizeType> &i_dest, 
                    bool walk_by_columns = false)
  {
    if (walk_by_columns)
    {
      for (SizeType j = 0; j < i_solution.front().size(); ++j)
      {
        if (i_solution[i_actual.first][j] != empty_value && j != i_actual.second)
        {
          const auto checked_pos = std::make_pair(i_actual.first, j);
          if (MarkAnyCycle(i_solution, io_marks, checked_pos, i_dest, false))
          {
            io_marks[i_actual.first][i_actual.second] = 1;
            return true;
          }
        }
      }
    }
    else
    {
      if (i_actual.second == i_dest.second)
      {
        io_marks[i_actual.first][i_actual.second] = -1;
        return true;
      }

      for (SizeType i = 0; i < i_solution.size(); ++i)
      {
        if (i_solution[i][i_actual.second] != empty_value && i != i_actual.first)
        {
          const auto checked_pos = std::make_pair(i ,i_actual.second);
          if (MarkAnyCycle(i_solution, io_marks, checked_pos, i_dest, true))
          {
            io_marks[i_actual.first][i_actual.second] = -1;
            return true;
          }
        }
      }
    }
    return false;
  }

  void RecalculateMarkedCells(Matrix<double>& io_solution, const Matrix<int>& i_marks)
  {
    double min_diff_value = std::numeric_limits<double>::max();
    PairOf<SizeType> best_indexes;
    const SizeType height = io_solution.size();
    const SizeType width = io_solution.front().size();

    for (SizeType i = 0; i < height; ++i)
    {
      for (SizeType j = 0; j < width; ++j)
      {
        if (i_marks[i][j] == -1 && io_solution[i][j] < min_diff_value)
        {
          min_diff_value = io_solution[i][j];
          best_indexes = std::make_pair(i, j);
        }
      }
    }

    for (SizeType i = 0; i < height; ++i)
    {
      for (SizeType j = 0; j < width; ++j)
      {
        io_solution[i][j] += i_marks[i][j] * min_diff_value;
      }
    }
    io_solution[best_indexes.first][best_indexes.second] = empty_value;
  }

  void RebuildSolutionMatrix(Matrix<double> &io_solution,const PairOf<SizeType> &i_pivot_indexes)
  {
    Matrix<int> marks_matrix(io_solution.size(), Vector<int>(io_solution.front().size()));
    auto [pivot_row, pivot_column] = i_pivot_indexes;
    marks_matrix[pivot_row][pivot_column] = 1;
    io_solution[pivot_row][pivot_column] = 0.0;
    auto& processed_row = io_solution[pivot_row];
    for (SizeType j = 0; j < processed_row.size(); ++j)
    {
      if (processed_row[j] != empty_value && j != i_pivot_indexes.second)
      {
        const auto start = std::make_pair(pivot_row, j);
        if (MarkAnyCycle(io_solution, marks_matrix, start, i_pivot_indexes))
        {
          marks_matrix[start.first][start.second] = -1;
          RecalculateMarkedCells(io_solution, marks_matrix);
          return;
        }
      }
    }
    throw std::runtime_error{ "Matrix degenerated !" };
  }

  double CalculateTransportPrice(const Matrix<double>& i_actual_solution, const Matrix<double>& i_costs)
  {
    double accumulation = 0;
    const SizeType rows_count = i_actual_solution.size();
    const SizeType columns_count = i_actual_solution.front().size();
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

  OptionalPair<SizeType> GetInvalidElementIndexes(const Matrix<double>& i_costs, const Matrix<double> &i_solution_matrix, const MatrixPotentials& i_potentials)
  {
    OptionalPair<SizeType> pivot_indexes;
    const SizeType rows_count = i_potentials.m_rows.size();
    const SizeType columns_count = i_potentials.m_columns.size();
    for (SizeType i = 0; i < rows_count; ++i)
    {
      for (SizeType j = 0; j < columns_count; ++j)
      {
        if (i_solution_matrix[i][j] == empty_value)
        {
          const double potential = i_potentials.PotentialAt(i, j);
          if (potential > i_costs[i][j])
          {
            if (!pivot_indexes)
            {
              pivot_indexes = std::make_pair(i, j);
            }
            else
            {
              auto& indexes = pivot_indexes.value();
              const double best_diff = i_costs[indexes.first][indexes.second] - i_potentials.PotentialAt(indexes.first, indexes.second);
              const double actual_diff = i_costs[i][j] - potential;
              if (actual_diff > best_diff)
                indexes = std::make_pair(i, j);
            }
          }
        }
      }
    }
    return pivot_indexes;
  }
}

namespace TransportTask
{
  double GetOptimalSolution(const TransportInformation& i_data, CreationMethod i_method, std::ostream *o_logger)
  {
    OptionalOutputStream output_stream{ o_logger };
    auto solution_matrix = FormatTask(i_data, i_method);
    output_stream << "Initial matrix :\n\n" << solution_matrix;
    output_stream << "\n\nInitial price = " << CalculateTransportPrice(solution_matrix, i_data.m_costs_matrix);
    SizeType iterations_count = 0;
    for(;;)
    {
      if (auto potentials = CalculatePotentials(i_data, solution_matrix); potentials)
      {
        output_stream << potentials.value() << '\n';
        ++iterations_count;
        if (auto indexes = GetInvalidElementIndexes(i_data.m_costs_matrix, solution_matrix, potentials.value()); indexes)
        {
          output_stream << "\nRebuilding matrix with pivot element at : [" << indexes->first << ", " << indexes->second << "]\n\n";
          RebuildSolutionMatrix(solution_matrix, indexes.value());
          output_stream << "Matrix after rebuild :\n\n" << solution_matrix
            << "\n\nZ = " << CalculateTransportPrice(solution_matrix, i_data.m_costs_matrix);
        }
        else break;
      }
      else throw std::runtime_error{ "Matrix degenerated !" };
    }
    auto optimal_price = CalculateTransportPrice(solution_matrix, i_data.m_costs_matrix);
    output_stream << "Matrix with optimal plane found using " << iterations_count << " iteration(s) \n\n" << solution_matrix
                  << "\n\nOptimal function value = " << optimal_price << '\n';
    if (o_logger) PrintSummaryToStream(*o_logger, solution_matrix);
    if (auto state_message = i_data.GetMessageForState(); state_message)
    {
      output_stream << "\nMessage : " << state_message.value() << "\n";
    }
    return optimal_price;
  }
}