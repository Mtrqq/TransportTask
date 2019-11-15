#pragma once
#include "..//TTSolver/TableCreator.h"
#include <optional>
#include <string>
#include <iostream>
#include <filesystem>

using OutputStreamWrapper = std::unique_ptr<std::ostream>;

class DataExtractor
{
  constexpr static char dir_separator = std::filesystem::path::preferred_separator;
public:
  DataExtractor(std::istream& i_stream)
    :m_communication_stream{i_stream}
  {
    if (auto data_stream_opt = SelectDataStream(m_communication_stream, std::cout); data_stream_opt)
    {
      m_data_stream = data_stream_opt;
      m_log_stream = m_data_stream == &std::cin ? &std::cout : nullptr;
    }
    else throw std::runtime_error{ "Invalid file path, unable to read data !" };
  }

  std::vector<OutputStreamWrapper> GetSolutionFiles(const std::string& i_folder_path);

  TransportTask::TransportInformation ReadTaskData();

private:
  std::istream& m_communication_stream;
  std::istream* m_data_stream;
  std::ostream* m_log_stream;
  std::string m_solved_file_path;

  std::istream* SelectDataStream(std::istream& i_input, std::ostream& o_output);

  static bool CreateFolderIfNotExists(const std::string& i_folder_name);

  static std::string GetFileName(const std::string& file_path);
};

bool GetYNResponse(std::istream& input_stream, std::ostream& output_stream);

struct InitialData
{
  std::vector<OutputStreamWrapper> output_streams;
  TransportTask::TransportInformation task_info;
};

std::optional<InitialData> TryReadInitialData(std::istream& input_stream);