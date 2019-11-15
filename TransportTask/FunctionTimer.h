#pragma once
#include <vector>
#include <iostream>
#include <chrono>

template <typename FunctionResult>
struct TimerFunctionResult
{
  FunctionResult function_result;
  double timing;

  TimerFunctionResult() = default;
};

template <typename FunctionResult>
std::ostream& operator<<(std::ostream& output, const TimerFunctionResult<FunctionResult>& result)
{
  return output << "Time :" << result.timing << '\t' << "Function result :" << result.function_result << std::endl;
}

template <typename FunctionType, typename ...Args>
auto ExecutionTime(FunctionType function, Args&& ...args)
{
  auto start = std::chrono::steady_clock::now();
  TimerFunctionResult<decltype(function(args...))> result;
  result.function_result = function(std::forward<Args>(args)...);
  result.timing = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
  return result;
}
