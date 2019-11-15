#pragma once

#ifdef SOLVER_EXPORT
   #define SOLVER_API __declspec(dllexport)
#else
   #define SOLVER_API __declspec(dllimport)
#endif // SOLVER_EXPORT