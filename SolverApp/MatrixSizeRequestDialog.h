#pragma once

#include <QDialog>
#include "ui_MatrixSizeRequestDialog.h"

class MatrixSizeRequestDialog : public QDialog
{
    Q_OBJECT

public:
    MatrixSizeRequestDialog(QWidget *parent = Q_NULLPTR);
    ~MatrixSizeRequestDialog() = default;

    signals:
  void MatrixSizeSelected(int rows_count, int columns_count);
private:
    Ui::MatrixSizeRequestDialog ui;
};
