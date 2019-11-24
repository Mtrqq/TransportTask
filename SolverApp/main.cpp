#include "SolverWindow.h"
#include "TTFillerForm.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    TTFillerForm filler_form;
    SolverWindow main_window;
    QObject::connect(&filler_form, &TTFillerForm::TaskFilled, &main_window, [&](TransportTask::TransportInformation info)
    {
      main_window.SetSolvedProblem(std::move(info));
      main_window.show();
    });
    if (filler_form.exec())
    {
      return a.exec();
    }
    return 0;
}
