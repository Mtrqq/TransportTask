#pragma once
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

    TransportInformation(const Matrix<double>& i_cost, const Vector<double> i_resources, const Vector<double> i_requirements);

    std::optional<std::string> GetMessageForState() const;

    Matrix<double> m_costs_matrix;
    Vector<double> m_requirements;
    Vector<double> m_resources;
  private:
    ResourcesState m_state = ResourcesState::Normal;
  };

  TransportInformation ReadTaskFromStream(std::istream& i_stream, std::ostream* o_logger = nullptr);
}

namespace TableProcessor
{
  void PrintMatrix(const TransportTask::Matrix<double>& i_matrix, std::ostream& output, TransportTask::SizeType i_alignment = 10);
}

namespace std
{
  std::ostream& operator<<(std::ostream& stream, const TransportTask::Matrix<double>& matrix);
}

namespace TransportTask
{
  class OptionalOutputStream
  {
  public:
    OptionalOutputStream(std::ostream* output_stream = nullptr)
      :stream{ output_stream }
    {}

    template <typename T>
    OptionalOutputStream& operator<<(const T& object)
    {
      if (stream)
      {
        (*stream) << object;
      }
      return *this;
    }

  private:
    std::ostream* stream;
  };
}