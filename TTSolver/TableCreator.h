#pragma once
#include "Utility.h"
#include "ExportHeader.h"
#include <string>

namespace TransportTask
{
	enum class CreationMethod : SizeType { NorthWestAngle, MinimalCost, VogelApproximation, DoubleMarks, LAST };

  SOLVER_API std::string GetMethodName(CreationMethod i_method);

  SOLVER_API Matrix<double> FormatTask(const TransportInformation &i_data,
							CreationMethod i_method = CreationMethod::VogelApproximation);
}