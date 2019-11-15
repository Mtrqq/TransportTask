#include "..//TTSolver/TaskSolver.h"
#include "../ThreadPool/ThreadPool.h"
#include "FunctionTimer.h"
#include "IOUtility.h"
#include <fstream>

using namespace TransportTask;

ThreadPool& GetThreadPool()
{
  static ThreadPool instance;
  return instance;
}

using FileWrapper = std::unique_ptr<std::ostream>;

void ExecuteWithDifferentMethods(const TransportInformation& i_information, const std::vector<FileWrapper>& i_output_streams)
{
  SizeType amount_of_methods = static_cast<SizeType>(CreationMethod::LAST);
  using ExecutionResult = std::future<TimerFunctionResult<double>>;
  std::vector<ExecutionResult> execution_results;
  execution_results.reserve(amount_of_methods);
  auto& thread_pool = GetThreadPool();
  for (SizeType i = 0; i != amount_of_methods; ++i)
  {
    auto& file = *i_output_streams[i];
    auto calculation_method = static_cast<CreationMethod>(i);
    auto actual_task = [&file, &i_information, calculation_method]
    {
      return ExecutionTime(GetOptimalSolution, i_information, calculation_method, &file);
    };
    execution_results.push_back(thread_pool.Execute(actual_task));
  }
  std::vector<TimerFunctionResult<double>> solutions(amount_of_methods);
  std::transform(execution_results.begin(), execution_results.end(), solutions.begin(), [](ExecutionResult& result)
  {
    return result.get();
  });
  thread_pool.Wait();
  auto first_result = solutions.front().function_result;
  if (std::all_of(solutions.cbegin(), solutions.cend(), [&first_result](const TimerFunctionResult<double>& result)
    {
      return result.function_result == first_result;
    }))
  {
    for (SizeType i = 0; i != amount_of_methods; ++i)
    {
      std::cout << "\nResult of " << GetMethodName(static_cast<CreationMethod>(i)) << " method execution : "
                << "\n" << solutions[i] << '\n';
    }
  }
  else throw std::runtime_error{ "calculation results are not the same !" };
}

int main()
{
  std::ios::sync_with_stdio(false);
  std::cout << "Would you like to read data from file ? (y/n)\nAnswer: ";
  std::istream* input_stream = &std::cin;
  std::ostream* input_log_stream = &std::cout;
  auto answer = GetYNResponse(std::cin, std::cout);
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
  std::ostream* output_log = GetYNResponse(std::cin, std::cout) ? &std::cout : nullptr;
  try
  {
    auto data = ReadTaskFromStream(*input_stream, input_log_stream);
    SizeType amount_of_methods = static_cast<SizeType>(CreationMethod::LAST);
    std::vector<FileWrapper> files_vec;
    for (SizeType i = 0; i != amount_of_methods; ++i)
    {
      auto file = std::make_unique<std::ofstream>(GetMethodName(static_cast<CreationMethod>(i)) + ".log");
      files_vec.push_back(std::move(file));
    }
    ExecuteWithDifferentMethods(data, files_vec);
  }
  catch (const std::runtime_error& exception)
  {
    std::cerr << "\nCalculation failed with message : " << exception.what() << std::endl;
  }
  system("pause");
}