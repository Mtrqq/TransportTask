#include "TableCreator.h"
#include <algorithm>
#include <numeric>

using namespace TransportTask;

namespace
{
  void NWCreator(Matrix<double>& io_edited_matrix, const Vector<double>& i_resources, const Vector<double>& i_requirements)
  {
    auto res_needed = i_requirements;
    const SizeType processed_height = io_edited_matrix.size();
    const SizeType processed_width = io_edited_matrix.front().size();
    SizeType processed_column_index = 0;
    double wasted_for_current_row = 0;
    for (SizeType row = 0; row < processed_height; ++row)
    {
      const double actual_resource = i_resources[row];
      while (wasted_for_current_row < actual_resource && processed_column_index < processed_width)
      {
        const double invest = std::min(actual_resource - wasted_for_current_row, res_needed[processed_column_index]);
        io_edited_matrix[row][processed_column_index] = invest;
        wasted_for_current_row += invest;
        res_needed[processed_column_index] -= invest;
        ++processed_column_index;
      }
      --processed_column_index;
      wasted_for_current_row = 0;
    }
  }
}

namespace TransportTask
{
  Matrix<double> FormatTask(const Vector<double>& i_resources, const Vector<double>& i_requirements)
  {
    _ASSERT(i_resources.size() != 0 && i_requirements.size() != 0);
    Matrix<double> formatted_matrix(i_resources.size(), Vector<double>(i_requirements.size(), empty_value));
    NWCreator(formatted_matrix, i_resources, i_requirements);
    return formatted_matrix;
  }
}