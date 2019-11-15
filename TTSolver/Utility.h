#pragma once
#include "ExportHeader.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <numeric>
#include <optional>

namespace TransportTask
{
  template <typename T>
  using Vector = std::vector<T>;

  template <typename T>
  using Matrix = Vector<Vector<T>>;

  using SizeType = std::size_t;

  template <typename T>
  using PairOf = std::pair<T, T>;

  template <typename T>
  using OptionalPair = std::optional<PairOf<T>>;

  constexpr double empty_value = std::numeric_limits<double>::lowest();

  class TransportInformation
  {
    enum class ResourcesState { Normal, Sufficient, Overflow };
  public:
    SOLVER_API TransportInformation(const Matrix<double>& i_cost, const Vector<double> i_resources, const Vector<double> i_requirements);

    SOLVER_API std::optional<std::string> GetMessageForState() const;

    Matrix<double> m_costs_matrix;
    Vector<double> m_requirements;
    Vector<double> m_resources;
  private:
    ResourcesState m_state = ResourcesState::Normal;
  };

  SOLVER_API TransportInformation ReadTaskFromStream(std::istream& i_stream, std::ostream* o_logger = nullptr);
}

namespace TableProcessor
{
  SOLVER_API void PrintMatrix(const TransportTask::Matrix<double>& i_matrix, std::ostream& output, TransportTask::SizeType i_alignment = 10);
}

namespace std
{
  SOLVER_API std::ostream& operator<<(std::ostream& m_stream, const TransportTask::Matrix<double>& matrix);
}

namespace TransportTask
{
  class OptionalOutputStream
  {
  public:
    OptionalOutputStream(std::ostream* output_stream = nullptr, bool should_throw = false)
      :m_stream{ output_stream }
      ,m_should_throw {should_throw}
    {}

    template <typename T>
    OptionalOutputStream& operator<<(const T& object)
    {
      if (m_stream)
      {
        (*m_stream) << object;
      }
      else if (m_should_throw)
      {
        throw std::runtime_error{ "Failed to show message !" };
      }
      return *this;
    }

  private:
    std::ostream* m_stream;
    bool m_should_throw;
  };
}