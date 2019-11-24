#pragma once
#include "ExportHeader.h"
#include <vector>
#include <optional>
#include <string>

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

  SOLVER_API Vector<std::string> GetResoucesDistributionDetails(const Matrix<double>& i_feasible_solution);

  struct MatrixPotentials
  {
    SOLVER_API MatrixPotentials(SizeType i_rows_count, SizeType i_columns_count);

    Vector<double> m_rows;
    Vector<double> m_columns;

    SOLVER_API static bool IsValidPotential(double i_potential);

    SOLVER_API double PotentialAt(SizeType i_row, SizeType i_column) const;

    SOLVER_API bool IsFullyCalculated() const;
  };  

  SOLVER_API double CalculateTransportPrice(const Matrix<double>& i_actual_solution, const Matrix<double>& i_costs);

  struct SolutionInfo
  {
    Vector<Matrix<double>> solution_steps;
    Vector<MatrixPotentials> potentials;
    Vector<PairOf<SizeType>> rebuilding_pivots;

    SOLVER_API SizeType GetIterationsCount() const;

    SOLVER_API double GetMatrixCostAtStep(SizeType step_index, const Matrix<double>& costs_matrix) const;

    SOLVER_API SizeType GetAmountOfBytesSpent() const;
  };

  SOLVER_API std::string GetStepDescription(const SolutionInfo& prepared_solution, SizeType step_index);
}