#pragma once
#include <chrono>

template <typename FunctionResult>
struct TimerFunctionResult
{
  FunctionResult function_result;
  double timing;

  TimerFunctionResult() = default;
};

template <typename FunctionType, typename ...Args>
auto ExecutionTime(FunctionType function, Args&& ...args)
{
  using namespace std::chrono;
  TimerFunctionResult<decltype(function(args...))> result;
  auto start = high_resolution_clock::now();
  result.function_result = function(std::forward<Args>(args)...);
  auto stop = high_resolution_clock::now();
  result.timing = duration_cast<milliseconds>(stop - start);
  return result;
}
