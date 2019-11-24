#include "SolverWindow.h"
#include "..//TTSolver/TaskSolver.h"
#include "..//ThreadPool/ThreadPool.h"
#include "..//ThreadPool/FuntionTimer.h"
#include "ExporterFunctions.h"
#include <QHeaderView>
#include <QTableWidget>
#include <QFileDialog>
#include <QDebug>

namespace
{
  void PrintPotentials(xlnt::worksheet& worksheet, const TransportTask::MatrixPotentials& potentials, std::size_t row, std::size_t column)
  {
    worksheet.cell(column, row).value("Row potentials :");
    auto& row_potentials = potentials.m_columns;
    for (int i = 0; i < row_potentials.size(); ++i)
    {
      worksheet.cell(column + i + 1, row).value(row_potentials[i]);
    }
    worksheet.cell(column, row + 1).value("Column potentials :");
    auto& column_potentials = potentials.m_rows;
    for (int i = 0; i < column_potentials.size(); ++i)
    {
      worksheet.cell(column + i + 1, row + 1).value(column_potentials[i]);
    }
  }

  void PrintSolution(xlnt::worksheet& worksheet, const TransportTask::SolutionInfo& solution)
  {
    constexpr std::size_t description_row_span = 2;
    const std::size_t iterations_count = solution.GetIterationsCount();
    const std::size_t spacing = solution.solution_steps.back().size() + 8;
    const std::size_t description_column_span = solution.solution_steps.back().front().size() + 1;
    const std::size_t rows_count = solution.solution_steps.back().size();
    const std::size_t columns_count = solution.solution_steps.back().front().size();

    xlnt::column_properties column_properties;
    column_properties.width = 50;
    column_properties.custom_width = true;
    worksheet.add_column_properties(1, column_properties);
    std::vector<std::string> horizontal_header;
    std::vector<std::string> vertical_header;
    const std::string columns_prefix = "Client #", rows_prefix = "Source #";
    for (int i = 0; i < std::max(rows_count, columns_count); ++i)
    {
      const std::string converted_number = std::to_string(i + 1);
      if (i < rows_count)
      {
        vertical_header.push_back(rows_prefix + converted_number);
      }
      if (i < columns_count)
      {
        horizontal_header.push_back(columns_prefix + converted_number);
      }
    }
    Excel::TableFormattingSettings settings;
    settings.horizontal_header = std::move(horizontal_header);
    settings.vertical_header = std::move(vertical_header);
    settings.caption = "Feasible solution matrix";
    for (int i = 0; i < iterations_count; ++i)
    {
      auto step_description = TransportTask::GetStepDescription(solution, i);
      const std::size_t description_row = i * spacing + 1;
      Excel::PrintText(worksheet, description_row, 1, step_description, description_column_span, description_row_span);
      PrintPotentials(worksheet, solution.potentials[i], description_row + 2, 1);
      const std::size_t table_row = description_row + 4;
      settings.top_most_row = table_row;
      if (i == iterations_count - 1)
      {
        settings.caption = "Optimal solution matrix";
      }
      Excel::SetupExcelFormatting(worksheet, settings);
      Excel::PrintTableContent(worksheet, settings, solution.solution_steps[i]);
    }
  }
}

SolverWindow::SolverWindow(QWidget *parent)
    :QWidget(parent)
{
    ui.setupUi(this);
    setWindowTitle("Solver");

    auto* header_view = ui.analysisTable->horizontalHeader();
    header_view->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    header_view->setStretchLastSection(true);

    ui.exportButton->setShortcut(QKeySequence::Save);
    QObject::connect(ui.exportButton, &QPushButton::clicked, this, &SolverWindow::ExportToExcel);
}

void SolverWindow::SetSolvedProblem(const TransportTask::TransportInformation& solved_problem)
{
  problem.reset(new TransportTask::TransportInformation(solved_problem));
  SolveProblem(solved_problem);
  auto optimal_solution_index = solutions.front().GetIterationsCount() - 1;
  auto optimal_solution = solutions.front().GetMatrixCostAtStep(optimal_solution_index, solved_problem.m_costs_matrix);
  ui.resultLine->setText(QString::number(optimal_solution));
  DisplayMethodsComparison();
  FillResourcesDistributionInfo();
}

void SolverWindow::SolveProblem(const TransportTask::TransportInformation& solved_problem)
{
  quint64 amount_of_methods = static_cast<quint64>(TransportTask::CreationMethod::LAST);
  ThreadPool thread_pool(amount_of_methods);
  using ExecutionResult = std::future<TimerFunctionResult<TransportTask::SolutionInfo>>;
  TransportTask::Vector<ExecutionResult> execution_results;
  execution_results.reserve(amount_of_methods);
  for (quint64 i = 0; i != amount_of_methods; ++i)
  {
    auto calculation_method = static_cast<TransportTask::CreationMethod>(i);
    auto actual_task = [&, calculation_method]
    {
      return ExecutionTime(TransportTask::GetOptimalSolution, solved_problem, calculation_method);
    };
    execution_results.push_back(std::move(thread_pool.Execute(actual_task)));
  }
  thread_pool.Wait();
  for (ExecutionResult &future_result : execution_results)
  {
    auto solution_info = std::move(future_result.get());
    solutions.push_back(std::move(solution_info.function_result));
    timings.push_back(solution_info.timing);
  }
}

void SolverWindow::DisplayMethodsComparison()
{
  constexpr quint64 execution_time_index = 0;
  constexpr quint64 iterations_index = 1;
  constexpr quint64 result_index = 2;
  constexpr quint64 memory_usage = 3;
  
  for (quint64 i = 0; i < solutions.size(); ++i)
  {
    auto& solution = solutions[i];
    SetTableValueAt(i, execution_time_index, timings[i]);
    auto iterations_count = solution.GetIterationsCount();
    SetTableValueAt(i, iterations_index, iterations_count);
    SetTableValueAt(i, result_index, solution.GetMatrixCostAtStep(iterations_count - 1, problem->m_costs_matrix));
    SetTableValueAt(i, memory_usage, solution.GetAmountOfBytesSpent());
  }
}

void SolverWindow::FillResourcesDistributionInfo()
{
  auto& solution_matrix = solutions.front().solution_steps.back();
  auto distribution_messages = TransportTask::GetResoucesDistributionDetails(solution_matrix);
  for (auto& message : distribution_messages)
  {
    QString inserted_label = QString::fromStdString(std::move(message));
    ui.distributionList->addItem(inserted_label);
  }
}

void SolverWindow::SetTableValueAt(quint64 row, quint64 column, double value)
{
  QTableWidgetItem* item = new QTableWidgetItem(QString::number(value));
  ui.analysisTable->setItem(row, column, item);
}

void SolverWindow::ExportToExcel()
{
  auto saved_file_url = QFileDialog::getSaveFileUrl(this, "Save as", QUrl(), "Excel files (*.xlsx)");
  if (!saved_file_url.isEmpty())
  {
    xlnt::workbook excel_document;
    auto methods_count = static_cast<int>(TransportTask::CreationMethod::LAST);
    std::vector<std::string> methods_names;
    std::vector<xlnt::worksheet> solution_pages;
    auto analysis_page = excel_document.active_sheet();
    for (int i = 0; i < methods_count; ++i)
    {
      auto method = static_cast<TransportTask::CreationMethod>(i);
      auto method_name = TransportTask::GetMethodName(method);
      auto new_worksheet = excel_document.copy_sheet(analysis_page);
      new_worksheet.title(method_name);
      solution_pages.push_back(std::move(new_worksheet));
      methods_names.push_back(std::move(method_name));
    }
    analysis_page.title("Analysis");
    auto rows_count = ui.analysisTable->rowCount();
    auto columns_count = ui.analysisTable->columnCount();
    std::vector<std::vector<double>> matrix_data(rows_count, std::vector<double>(columns_count));
    for (int i = 0; i < rows_count; ++i)
    {
      for (int j = 0; j < columns_count; ++j)
      {
        auto item_data = ui.analysisTable->item(i, j)->text();
        matrix_data[i][j] = item_data.toDouble();
      }
    }

    std::vector<std::string> horizontal_header{ "Execution time", "Iterations count", "Result", "Memory used(bytes)" };
    Excel::TableFormattingSettings settings;
    settings.caption = "Comparison of different solution methods";
    settings.horizontal_header = std::move(horizontal_header);
    settings.vertical_header = std::move(methods_names);
    Excel::SetupExcelFormatting(analysis_page, settings);
    Excel::PrintTableContent(analysis_page, settings, matrix_data);
    for (int i = 0; i != methods_count; ++i)
    {
      xlnt::worksheet& current_worksheet = solution_pages[i];
      PrintSolution(current_worksheet, solutions[i]);
    }
    excel_document.save(saved_file_url.toLocalFile().toStdString().c_str());
  }
}