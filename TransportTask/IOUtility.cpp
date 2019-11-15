#include "IOUtility.h"
#include <algorithm>

std::optional<bool> ConvertStringToBool(const std::string& str)
{
  std::string converted_string(str.size(), ' ');
  std::transform(str.cbegin(), str.cend(), converted_string.begin(), ::tolower);
  if (converted_string == "y" || converted_string == "yes")
  {
    return true;
  }
  else if (converted_string == "n" || converted_string == "no")
  {
    return false;
  }
  return {};
}

bool GetYNResponse(std::istream& input_stream, std::ostream& output_stream)
{
  std::string responce;
  input_stream >> responce;
  auto converted_bool = ConvertStringToBool(responce);
  while (!converted_bool)
  {
    output_stream << "\nEnter correct answer :";
    input_stream >> responce;
    converted_bool = ConvertStringToBool(responce);
  }
  input_stream.clear();
  return converted_bool.value();
}
