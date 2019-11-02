#include "TaskSolver.h"
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace TransportTask;

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

bool GetYNResponse()
{
  std::string responce;
  std::cin >> responce;
  auto converted_bool = ConvertStringToBool(responce);
  while (!converted_bool)
  {
    std::cout << "\nEnter correct answer :";
    std::cin >> responce;
    converted_bool = ConvertStringToBool(responce);
  }
  std::cin.clear();
  return converted_bool.value();
}

int main()
{
  std::ios::sync_with_stdio(false);
  std::cout << "Would you like to read data from file ? (y/n)\nAnswer: ";
  std::istream* input_stream = &std::cin;
  std::ostream* input_log_stream = &std::cout;
  auto answer = GetYNResponse();
  if (answer)
  {
    std::string file_name;
    std::cout << "Enter path to file with data : ";
    std::cin >> file_name;
    auto file = new std::ifstream{ file_name };
    if (!file->is_open()) return 1;
    input_stream = file;
    input_log_stream = nullptr;
  }
  std::cout << "Would you like to gain detailed solution ? (y/n)\nAnswer : ";
  std::ostream* output_log = GetYNResponse() ? &std::cout : nullptr;
  try
  {
    auto data = ReadTaskFromStream(*input_stream, input_log_stream);
    auto solution = GetOptimalSolution(data, output_log);
    std::cout << "\nOptimal solution Z = " << solution << std::endl;
  }
  catch (const std::runtime_error& exception)
  {
    std::cout << "\nCalculation failed with message : " << exception.what() << std::endl;
  }
  system("pause");
}