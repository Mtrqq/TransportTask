#pragma once
#include <QtWidgets/QWidget>
#include "ui_SolverWindow.h"
#include <QScopedPointer>

#include "..\\TTSolver\Utility.h"

class SolverWindow : public QWidget
{
    Q_OBJECT
public:
    SolverWindow(QWidget *parent = Q_NULLPTR);

public slots:
    void SetSolvedProblem(const TransportTask::TransportInformation& solved_problem);
private slots:
    void ExportToExcel();
private:
    Ui::SolverWindowClass ui;
    QScopedPointer<TransportTask::TransportInformation> problem;
    QVector<TransportTask::SolutionInfo> solutions;
    QVector<quint64> timings;

    void SolveProblem(const TransportTask::TransportInformation& solved_problem);
    void DisplayMethodsComparison();
    void FillResourcesDistributionInfo();

    void SetTableValueAt(quint64 row, quint64 column, double value);
};
