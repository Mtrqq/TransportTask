#include "ExporterFunctions.h"

namespace
{
  void ApplyStyleToCellsRange(xlnt::worksheet& processed_sheet, const Excel::TableFormattingSettings& settings)
  {
    xlnt::alignment center_alignment;
    center_alignment.horizontal(xlnt::horizontal_alignment::center);

    auto all_border_sides = xlnt::border::all_sides();
    xlnt::border::border_property border_propery;
    border_propery.style(settings.border_style);
    xlnt::border cell_border;
    for (auto& side : all_border_sides)
    {
      cell_border.side(side, border_propery);
    }

    const auto start_row = settings.top_most_row;
    const auto start_column = settings.left_most_column;

    for (std::size_t column = 0; column <= settings.horizontal_header.size(); ++column)
    {
      for (std::size_t row = 0; row <= settings.vertical_header.size() + 1; ++row)
      {
        auto actual_cell = processed_sheet.cell(column + start_column, row + start_row);
        actual_cell.border(cell_border);
        if (column != 0)
        {
          actual_cell.alignment(center_alignment);
        }
      }
    }
    processed_sheet.cell(start_column, start_row).alignment(center_alignment);
  }
}

namespace Excel
{
  std::vector<std::string> TableFormattingSettings::GetVerticalHeader()
  {
    std::vector<std::string> header;
    header.emplace_back("North-west angle");
    header.emplace_back("Minimal cost");
    header.emplace_back("Vogel approximation");
    header.emplace_back("Double marks");

    return header;
  }

  std::vector<std::string> TableFormattingSettings::GetHorizontalHeader()
  {
    std::vector<std::string> header;
    header.emplace_back("Execution time(µs)");
    header.emplace_back("Amount of iterations");
    header.emplace_back("Result");
    header.emplace_back("Used memory(bytes)");

    return header;
  }

  TableFormattingSettings::TableFormattingSettings(std::size_t left_most_column, std::size_t top_most_row,
    std::size_t cell_width, const std::string& caption,
    xlnt::border_style border_style,
    const std::vector<std::string>& horizontal_header,
    const std::vector<std::string>& vertical_header)
    :left_most_column{ left_most_column }
    ,top_most_row{ top_most_row }
    ,cell_width{ cell_width }
    ,caption{ caption }
    ,border_style{ border_style }
    ,horizontal_header{ horizontal_header }
    ,vertical_header{ vertical_header }
  {}

  void SetupExcelFormatting(xlnt::worksheet& processed_sheet, const TableFormattingSettings& settings)
  {
    ApplyStyleToCellsRange(processed_sheet, settings);

    const auto start_row = settings.top_most_row;
    const auto start_column = settings.left_most_column;

    processed_sheet.merge_cells(xlnt::range_reference(start_column, start_row, start_column + settings.horizontal_header.size(), start_row));
    processed_sheet.cell(start_column, start_row).value(settings.caption.c_str());

    xlnt::column_properties column_properties;
    column_properties.width = settings.cell_width;
    column_properties.custom_width = true;
    for (std::size_t i = 0; i <= settings.horizontal_header.size(); ++i)
    {
      processed_sheet.add_column_properties(i + start_column, column_properties);
    }

    for (std::size_t i = 0; i < settings.horizontal_header.size(); ++i)
    {
      processed_sheet.cell(i + start_column + 1, start_row + 1).value(settings.horizontal_header[i].c_str());
    }

    for (std::size_t i = 0; i < settings.vertical_header.size(); ++i)
    {
      processed_sheet.cell(start_column, i + start_row + 2).value(settings.vertical_header[i].c_str());
    }
  }

  void PrintTableContent(xlnt::worksheet& processed_sheet, const TableFormattingSettings& settings, const std::vector<std::vector<double>>& numeric_matrix)
  {
    auto first_row = settings.top_most_row + 2;
    auto first_column = settings.left_most_column + 1;
    for (std::size_t i = 0; i < numeric_matrix.size(); ++i)
    {
      for (std::size_t j = 0; j < numeric_matrix[i].size(); ++j)
      {
        auto current_cell = processed_sheet.cell(first_column + j, first_row + i);
        current_cell.value(numeric_matrix[i][j]);
      }
    }
  }

  void PrintText(xlnt::worksheet& processed_sheet, std::size_t row, std::size_t column, const std::string& text, std::size_t column_span, std::size_t row_span)
  {
    if (column_span != 1 && row_span != 1)
    {
      processed_sheet.merge_cells(xlnt::range_reference( column, row, column + column_span - 1, row + row_span - 1 ));
    }
    processed_sheet.cell(column, row).value(text.c_str());
  }
}