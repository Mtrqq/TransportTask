#pragma once
#include "Utility.h"
#include <string>

namespace TransportTask
{
	enum class CreationMethod { NorthWestAngle, MinimalCost, VogelApproximation, LAST };

  CreationMethod ReadCreationMethod(std::istream &i_input, std::ostream* o_logger);

  std::string GetMethodName(CreationMethod i_method);

	Matrix<double> FormatTask(const TransportInformation &i_data,
							CreationMethod i_method = CreationMethod::VogelApproximation);
}