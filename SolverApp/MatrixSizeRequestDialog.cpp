#include "MatrixSizeRequestDialog.h"
#include <QPushButton>

MatrixSizeRequestDialog::MatrixSizeRequestDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    setWindowTitle("Matrix size selection dialog");

    QObject::connect(ui.okButton, &QPushButton::clicked, this, [this]
    {
      auto rows_count = ui.rowsLine->text().toInt();
      auto columns_count = ui.columnsLine->text().toInt();
      emit MatrixSizeSelected(rows_count + 1, columns_count + 1);
      QDialog::accept();
    });
}