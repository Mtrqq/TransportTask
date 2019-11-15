#include "..//TTSolver/TaskSolver.h"
#include "../ThreadPool/ThreadPool.h"
#include "FunctionTimer.h"
#include "IOUtility.h"

using namespace TransportTask;

ThreadPool& GetThreadPool()
{
  static ThreadPool instance;
  return instance;
}

void SolveUsingDifferentMethods(const TransportInformation& i_information, const Vector<OutputStreamWrapper>& i_output_streams)
{
  SizeType amount_of_methods = static_cast<SizeType>(CreationMethod::LAST);
  using ExecutionResult = std::future<TimerFunctionResult<double>>;
  std::vector<ExecutionResult> execution_results;
  execution_results.reserve(amount_of_methods);
  auto& thread_pool = GetThreadPool();
  for (SizeType i = 0; i < amount_of_methods; ++i)
  {
    auto calculation_method = static_cast<CreationMethod>(i);
    auto* file = i_output_streams[i].get();
    auto actual_task = [&, file, calculation_method]
    {
      return ExecutionTime(GetOptimalSolution, i_information, calculation_method, file);
    };
    execution_results.push_back(thread_pool.Execute(actual_task));
  }
  thread_pool.Wait();
  std::vector<TimerFunctionResult<double>> solutions(amount_of_methods);
  std::transform(execution_results.begin(), execution_results.end(), solutions.begin(), [](ExecutionResult& result)
    {
      return result.get();
    });
  const auto first_result = solutions.front().function_result;
  if (std::all_of(solutions.cbegin() + 1, solutions.cend(), [&first_result](const TimerFunctionResult<double>& result)
    {
      return result.function_result == first_result;
    }))
  {
    std::cout.setf(std::ios::fixed);
    for (SizeType i = 0; i != amount_of_methods; ++i)
    {
      const auto method_name = GetMethodName(static_cast<CreationMethod>(i));
      std::cout << "\nResult of " << method_name << " method execution : " << "\n" << solutions[i] << '\n';
    }
  }
  else throw std::runtime_error{ "Calculation results are not the same !" };
}

int main()
{
  std::ios::sync_with_stdio(false);
  if (auto initial_data_opt = TryReadInitialData(std::cin); initial_data_opt)
  {
    try
    {
      auto transport_information = initial_data_opt->task_info;
      auto file_streams = std::move(initial_data_opt->output_streams);
      SolveUsingDifferentMethods(transport_information, file_streams);
    }
    catch (const std::exception& exception)
    {
      std::cerr << "Calculation failed with message : " << exception.what();
      return 1;
    }
    system("pause");
  }
  return 1;
}