#pragma once
#include <optional>
#include <string>
#include <iostream>

std::optional<bool> ConvertStringToBool(const std::string& str);

bool GetYNResponse(std::istream& input_stream, std::ostream& output_stream);