#pragma once
#include <vector>
#include <string>

#include <xlnt/xlnt.hpp>

namespace Excel
{
  class TableFormattingSettings
  {
  public:
    TableFormattingSettings(std::size_t left_most_column = 1, std::size_t top_most_row = 1,
      std::size_t cell_width = 30, const std::string & caption = "Caption",
      xlnt::border_style border_style = xlnt::border_style::thin,
      const std::vector<std::string> & horizontal_header = GetHorizontalHeader(),
      const std::vector<std::string> & vertical_header = GetVerticalHeader());

    std::size_t left_most_column;
    std::size_t top_most_row;
    std::size_t cell_width;
    std::string caption;
    xlnt::border_style border_style;
    std::vector<std::string> horizontal_header;
    std::vector<std::string> vertical_header;

  private:
    static std::vector<std::string> GetHorizontalHeader();
    static std::vector<std::string> GetVerticalHeader();
  };

  void SetupExcelFormatting(xlnt::worksheet& processed_sheet, const TableFormattingSettings& settings);

  void PrintTableContent(xlnt::worksheet& processed_sheet, const TableFormattingSettings &settings,const std::vector<std::vector<double>>& numeric_matrix);

  void PrintText(xlnt::worksheet& processed_sheet, std::size_t row, std::size_t column, const std::string& text, std::size_t column_span = 1, std::size_t row_span = 1);
}