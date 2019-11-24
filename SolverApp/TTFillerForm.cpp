#include "TTFillerForm.h"
#include "MatrixSizeRequestDialog.h"
#include <QMessageBox>
#include <QAction>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <vector>
#include <optional>

namespace
{
  std::optional<double> TryParseDouble(QTableWidgetItem* item)
  {
    if (item != nullptr)
    {
      bool flag;
      double value = item->text().toDouble(&flag);
      if (flag)
      {
        return value;
      }
    }
    return std::nullopt;
  }
}

TTFillerForm::TTFillerForm(QWidget* parent)
  : QDialog(parent)
{
  ui.setupUi(this);
  setWindowTitle("Data reading dialog");
  ResizeMatrix(2, 2);
  ui.costsTable->setEditTriggers(QAbstractItemView::EditTrigger::AllEditTriggers);
  QTableWidgetItem* ignored_cell = new QTableWidgetItem;
  ignored_cell->setFlags(ignored_cell->flags() ^ (Qt::ItemIsSelectable | Qt::ItemIsEditable));
  ui.costsTable->setItem(0, 0, ignored_cell);

  QStringList shortcuts;
  shortcuts << "Ctrl+N" << "Ctrl+Shift+N" << "Ctrl+D" << "Ctrl+Shift+D";
  for (quint64 i = 0; i < 4; ++i)
  {
    actions.push_back(new QAction(this));
    actions[i]->setShortcut(shortcuts[i]);
  }

  ui.serializeButton->setShortcut(QKeySequence::Save);
  ui.loadButton->setShortcut(QKeySequence::Open);
  ui.helpButton->setShortcut(QKeySequence::HelpContents);

  QObject::connect(actions[0], &QAction::triggered, this, &TTFillerForm::AddClient);
  QObject::connect(actions[1], &QAction::triggered, this, &TTFillerForm::AddSource);
  QObject::connect(actions[2], &QAction::triggered, this, &TTFillerForm::RemoveClient);
  QObject::connect(actions[3], &QAction::triggered, this, &TTFillerForm::RemoveSource);
  QObject::connect(ui.finishButton, &QPushButton::clicked, this, &TTFillerForm::FinishedEditing);
  QObject::connect(ui.helpButton, &QPushButton::clicked, this, &TTFillerForm::ShowTooltip);
  QObject::connect(ui.serializeButton, &QPushButton::clicked, this, &TTFillerForm::SaveToJSON);
  QObject::connect(ui.loadButton, &QPushButton::clicked, this, &TTFillerForm::LoadFromJSON);

  addActions(actions);

  MatrixSizeRequestDialog dialog;
  QObject::connect(&dialog, &MatrixSizeRequestDialog::MatrixSizeSelected, this, &TTFillerForm::ResizeMatrix);
  dialog.exec();
}

void TTFillerForm::SetHeaderAt(Qt::Orientation header_orientation, int index, const QString& new_header)
{
  QTableWidgetItem* item = new QTableWidgetItem(new_header);
  if (header_orientation == Qt::Horizontal)
  {
    ui.costsTable->setHorizontalHeaderItem(index, item);
  }
  else
  {
    ui.costsTable->setVerticalHeaderItem(index, item);
  }
}

void TTFillerForm::AddClient()
{
  auto columns_count = ui.costsTable->columnCount();
  ui.costsTable->insertColumn(columns_count);
  SetHeaderAt(Qt::Horizontal, columns_count, QString("Client #%1").arg(columns_count));
}

void TTFillerForm::RemoveClient()
{
  auto columns_count = ui.costsTable->columnCount();
  if (columns_count > 2)
  {
    ui.costsTable->removeColumn(columns_count - 1);
  }
}

void TTFillerForm::AddSource()
{
  auto rows_count = ui.costsTable->rowCount();
  ui.costsTable->insertRow(rows_count);
  SetHeaderAt(Qt::Vertical, rows_count, QString("Source #%1").arg(rows_count));
}

void TTFillerForm::RemoveSource()
{
  auto rows_count = ui.costsTable->rowCount();
  if (rows_count > 2)
  {
    ui.costsTable->removeRow(rows_count - 1);
  }
}

void TTFillerForm::FinishedEditing()
{
  const quint64 rows_count = ui.costsTable->rowCount();
  const quint64 columns_count = ui.costsTable->columnCount();
  using Matrix = TransportTask::Matrix<double>;
  using Vector = TransportTask::Vector<double>;
  Vector requirements_data;
  for (int column = 1; column < columns_count; ++column)
  {
    auto item = ui.costsTable->item(0, column);
    if (auto extracted_value = TryParseDouble(item); extracted_value)
    {
      requirements_data.push_back(extracted_value.value());
    }
    else
    {
      QMessageBox::warning(nullptr, "Warning", "Requirements row invalidated");
      return;
    }
  }
  Vector sources_data;
  for (int row = 1; row < rows_count; ++row)
  {
    auto item = ui.costsTable->item(row, 0);
    if (auto extracted_value = TryParseDouble(item); extracted_value)
    {
      sources_data.push_back(extracted_value.value());
    }
    else
    {
      QMessageBox::warning(nullptr, "Warning", "Sources column invalidated");
      return;
    }
  }
  Matrix costs_matrix(rows_count - 1, Vector(columns_count - 1));
  for (int i = 1; i < rows_count; ++i)
  {
    for (int j = 1; j < columns_count; ++j)
    {
      auto item = ui.costsTable->item(i, j);
      if (auto extracted_value = TryParseDouble(item); extracted_value)
      {
        costs_matrix[i - 1][j - 1] = extracted_value.value();
      }
      else
      {
        QMessageBox::warning(nullptr, "Warning", "Costs matrix invalidated");
        return;
      }
    }
  }
  TransportTask::TransportInformation task_information{ costs_matrix, sources_data, requirements_data };
  emit TaskFilled(std::move(task_information));
  QDialog::accept();
}

void TTFillerForm::ResizeMatrix(int rows_count, int columns_count)
{
  auto max_size = qMax(rows_count, columns_count);
  QStringList horizontal_header{ "" }, vertical_header{ "" };
  for (int i = 1; i <= max_size; ++i)
  {
    horizontal_header << QString("Client #%1").arg(i);
    vertical_header << QString("Source #%1").arg(i);
  }

  ui.costsTable->setRowCount(rows_count);
  ui.costsTable->setVerticalHeaderLabels(vertical_header);
  SetHeaderAt(Qt::Orientation::Vertical, 0, "");
  ui.costsTable->setColumnCount(columns_count);
  ui.costsTable->setHorizontalHeaderLabels(horizontal_header);
  SetHeaderAt(Qt::Orientation::Horizontal, 0, "");
}

void TTFillerForm::SaveToJSON() const
{
  const int rows_count = ui.costsTable->rowCount();
  const int columns_count = ui.costsTable->columnCount();
  QVector<QVector<QString>> saved_matrix(rows_count, QVector<QString>(columns_count, ""));
  for (int row = 0; row < rows_count; ++row)
  {
    for (int column = 0; column < columns_count; ++column)
    {
      auto item = ui.costsTable->item(row, column);
      if (item)
      {
        saved_matrix[row][column] = item->text();
      }
    }
  }

  auto selected_file_url = QFileDialog::getSaveFileUrl(nullptr, "Save as", QUrl(), tr("Transportation task input (*.tti)"));
  if (!selected_file_url.isEmpty())
  {
    QFile saved_file(selected_file_url.toLocalFile());
    if (saved_file.open(QIODevice::WriteOnly))
    {
      QJsonArray input_array_serialized;
      for (auto& row : saved_matrix)
      {
        QJsonArray row_of_strings;
        for (QString cell_value : row)
        {
          row_of_strings.append(cell_value);
        }
        input_array_serialized.append(row_of_strings);
      }
      QJsonObject written_object;
      written_object["array_of_matrix_rows"] = input_array_serialized;
      QJsonDocument document(written_object);
      saved_file.write(document.toBinaryData());
    }
    else
    {
      QMessageBox::warning(nullptr, "Warning", "Something went wrong !");
    }
  }
}

void TTFillerForm::LoadFromJSON()
{
  auto selected_file_path = QFileDialog::getOpenFileName(nullptr, "Select file", "", tr("Transportation task input (*.tti)"));
  QFile json_file{ selected_file_path };
  if (json_file.open(QIODevice::ReadOnly))
  {
    auto file_content = json_file.readAll();
    QJsonDocument document = QJsonDocument::fromBinaryData(file_content);
    if (!document.isNull())
    {
      auto json_object = document.object();
      if (json_object.contains("array_of_matrix_rows") && json_object["array_of_matrix_rows"].isArray())
      {
        auto rows_array = json_object["array_of_matrix_rows"].toArray();
        const int rows_count = rows_array.size();
        const int columns_count = rows_array[0].toArray().size();
        ResizeMatrix(rows_count, columns_count);
        for (int i = 0; i < rows_count; ++i)
        {
          auto& actual_row = rows_array[i].toArray();
          for (int j = 0; j < columns_count; ++j)
          {
            QTableWidgetItem* item = new QTableWidgetItem(actual_row[j].toString());
            ui.costsTable->setItem(i, j, item);
          }
        }
        QTableWidgetItem* ignored_cell = new QTableWidgetItem;
        ignored_cell->setFlags(ignored_cell->flags() ^ (Qt::ItemIsSelectable | Qt::ItemIsEditable));
        ui.costsTable->setItem(0, 0, ignored_cell);
        return;
      }
    }
  }
  QMessageBox::warning(nullptr, "Warning", "Unable to read file !");
}

void TTFillerForm::ShowTooltip() const
{
  const QString tooltip_message = "In this window should fill all table fields with your formalized task data.\n"
                                  "There you have first column with sources, you have to fill this column with amounts of product units\n"
                                  "available for each source. After that you have to fill first row with clients requirements, there you have\n"
                                  "to insert each client's requirement. Then fill costs matrix with prices of transporting from N client to M source.\n"
                                  "Notice : first element in the first row should be ignored, cause it simply makes no sense\n"
                                  "\t\tUsefull shortcuts : \n Ctrl + N - row insertion;\n Ctrl + Shift + N - column insertion;\n Ctrl + D - row deletion;\n Ctrl + Shift + D - column deletion;";
  QMessageBox::information(nullptr, "Help", tooltip_message);
}
