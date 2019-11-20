#include "IOUtility.h"
#include <Windows.h>
#include <algorithm>
#include <fstream>
#include <memory>

namespace
{
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
    return std::nullopt;
  }
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

std::optional<InitialData> TryReadInitialData(std::istream& input_stream)
{
  using namespace TransportTask;
  const std::string solutions_folder_path = "Solutions";
  try
  {
    DataExtractor extractor{ std::cin };
    auto task_data = extractor.ReadTaskData();
    constexpr SizeType amount_of_methods = static_cast<SizeType>(CreationMethod::LAST);
    Vector<OutputStreamWrapper> output_files(amount_of_methods);
    std::cout << "Would you like to gain detailed solution ? (y/n)\nAnswer : ";
    if (bool answer = GetYNResponse(std::cin, std::cout); answer)
    {
      output_files = extractor.GetSolutionFiles(solutions_folder_path);
    }
    return InitialData{ std::move(output_files), task_data };
  }
  catch (const std::exception& exception)
  {
    std::cerr << "Reading data failed with message : " << exception.what();
    return std::nullopt;
  }
}

inline std::vector<OutputStreamWrapper> DataExtractor::GetSolutionFiles(const std::string& i_folder_path)
{
  using namespace TransportTask;
  auto actual_solution_folder = i_folder_path + dir_separator + GetFileName(m_solved_file_path);
  if (CreateFolderIfNotExists(i_folder_path) && CreateFolderIfNotExists(actual_solution_folder))
  {
    const SizeType count_of_methods = static_cast<SizeType>(CreationMethod::LAST);
    Vector<OutputStreamWrapper> file_streams;
    file_streams.reserve(count_of_methods);
    for (SizeType i = 0; i != count_of_methods; ++i)
    {
      auto method_name = GetMethodName(static_cast<CreationMethod>(i));
      std::replace(method_name.begin(), method_name.end(), ' ', '_');
      auto file_path = actual_solution_folder + dir_separator + method_name + ".log";
      auto file_stream = std::make_unique<std::ofstream>(file_path);
      file_streams.push_back(std::move(file_stream));
    }
    return file_streams;
  }
  throw std::runtime_error{ "Unable to create solution folders !" };
}

TransportTask::TransportInformation DataExtractor::ReadTaskData()
{
  return TransportTask::ReadTaskFromStream(*m_data_stream, m_log_stream);
}

inline std::istream* DataExtractor::SelectDataStream(std::istream& i_input, std::ostream& o_output)
{
  std::cout << "Would you like to read data from file ? (y/n)\nAnswer: ";
  if (bool answer = GetYNResponse(std::cin, std::cout); answer)
  {
    auto file_path = SelectAnyFile();
    auto file_stream = new std::ifstream{ file_path };
    if (file_stream->is_open())
    {
      m_solved_file_path = file_path;
      return file_stream;
    }
    return nullptr;
  }
  m_solved_file_path = "ConsoleInput";
  return &std::cin;
}

inline bool DataExtractor::CreateFolderIfNotExists(const std::string& i_folder_name)
{
  if (!std::filesystem::exists(i_folder_name))
  {
    return std::filesystem::create_directory(i_folder_name);
  }
  return true;
}

std::string DataExtractor::GetFileName(const std::string &i_file_path)
{
  std::string converted_string = i_file_path;
  std::replace(converted_string.begin(), converted_string.end(), '/', dir_separator);
  auto last_separator_pos = converted_string.find_last_of(dir_separator);
  auto last_dot_pos = converted_string.find_last_of('.');
  if (last_separator_pos == std::string::npos)
    last_separator_pos = 0;
  else ++last_separator_pos;
  return converted_string.substr(last_separator_pos, last_dot_pos - last_separator_pos);
}

std::string DataExtractor::SelectAnyFile()
{
  OPENFILENAME ofn; // Common dialog box structure.
  constexpr std::size_t buffer_size = 256;
  auto file_path_buffer = std::make_unique<char[]>(buffer_size);
  auto file_title_buffer = std::make_unique<char[]>(buffer_size);
  memset(file_path_buffer.get(), '\0', buffer_size);
  HWND actual_hwnd = GetActiveWindow();
  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = actual_hwnd;
  ofn.lpstrFile = file_path_buffer.get();
  ofn.nMaxFile = buffer_size;
  ofn.lpstrFilter = "All Files\0*.*\0\0";
  ofn.nFilterIndex = 2; // ??
  ofn.lpstrFileTitle = file_title_buffer.get();
  ofn.nMaxFileTitle = buffer_size;
  ofn.lpstrInitialDir = NULL;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
  if (!GetOpenFileName(&ofn))
    throw std::runtime_error{ "File selection error !" };

  return { file_path_buffer.get() , buffer_size };
}
