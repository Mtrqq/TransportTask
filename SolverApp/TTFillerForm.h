#pragma once

#include <QDialog>
#include "ui_TTFillerForm.h"
#include "..//TTSolver/Utility.h"

class TTFillerForm : public QDialog
{
    Q_OBJECT

public:
    TTFillerForm(QWidget *parent = Q_NULLPTR);
    ~TTFillerForm() = default;

signals:
  void TaskFilled(TransportTask::TransportInformation transport_info);
private slots:
  void AddClient();
  void RemoveClient();
  void AddSource();
  void RemoveSource();
  void FinishedEditing();
  void ResizeMatrix(int rows_count, int columns_count);
  void SaveToJSON() const;
  void LoadFromJSON();
private:
    Ui::TTFillerForm ui;
    QList<QAction*> actions;

    void SetHeaderAt(Qt::Orientation header_orientation, int index, const QString& new_header);

    void ShowTooltip() const;
};
