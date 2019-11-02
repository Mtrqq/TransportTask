#pragma once
#include "Utility.h"

namespace TransportTask
{
  Matrix<double> FormatTask(const Vector<double>& i_sources,
                            const Vector<double>& i_requirements);
}