#include "pch.h"
#include "Utility.h"

namespace TransportTask
{
  TransportInformation::TransportInformation(const Matrix<double>& i_cost, const Vector<double> i_resources, const Vector<double> i_requirements)
    :m_costs_matrix{ i_cost }
    , m_requirements{ i_requirements }
    , m_resources{ i_resources }
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
        return "Last column indicates unused resources";
      }
      else return "Last row indicates sufficient resources";
    }
    return {};
  }

  TransportInformation ReadTaskFromStream(std::istream& i_stream, std::ostream* o_logger)
  {
    OptionalOutputStream output_stream{ o_logger };
    output_stream << "Enter sources count : ";
    SizeType sources_count;
    i_stream >> sources_count;
    Vector<double> resources(sources_count);
    output_stream << "Enter amount of good produced by each source : ";
    for (SizeType i = 0; i < sources_count; ++i)
    {
      i_stream >> resources[i];
    }
    output_stream << "Enter clients count : ";
    SizeType clients_count;
    i_stream >> clients_count;
    Vector<double> requirements(clients_count);
    output_stream << "Enter amount of good required by each client :";
    for (SizeType i = 0; i < clients_count; ++i)
    {
      i_stream >> requirements[i];
    }
    Matrix<double> costs(sources_count, Vector<double>(clients_count));
    output_stream << "Enter costs matrix :\n";
    for (SizeType i = 0; i < sources_count; ++i)
    {
      for (SizeType j = 0; j < clients_count; ++j)
      {
        i_stream >> costs[i][j];
      }
    }
    return { costs, resources, requirements };
  }

  void PrintSummaryToStream(std::ostream& i_output_stream, const Matrix<double> i_solution_matrix)
  {
    i_output_stream << "\nResources distribution :";
    for (SizeType i = 0; i < i_solution_matrix.size(); ++i)
    {
      for (SizeType j = 0; j < i_solution_matrix[i].size(); ++j)
      {
        if (i_solution_matrix[i][j] != empty_value)
        {
          i_output_stream << "Transport " << i_solution_matrix[i][j] << " units of resource from factory #" << i + 1 << " to client #" << j + 1 << '\n';
        }
      }
    }
  }
}

using namespace TransportTask;

void TableProcessor::PrintMatrix(const Matrix<double>& i_matrix, std::ostream& output, SizeType i_alignment)
{
  for (auto row : i_matrix)
  {
    for (auto value : row)
    {
      if (value != empty_value)
      {
        output << std::setw(i_alignment) << value << '\t';
      }
      else
      {
        output << std::setw(i_alignment) << "###" << '\t';
      }
    }
    output << '\n';
  }
}


std::ostream& std::operator<<(std::ostream& m_stream, const TransportTask::Matrix<double>& matrix)
{
  TableProcessor::PrintMatrix(matrix, m_stream);
  return m_stream;
}