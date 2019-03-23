/*
    Copyright 2019 Joel Svensson	svenssonjoel@yahoo.se

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include "qcustomplot.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_testerConnectPushButton_clicked();
    void on_testeeConnectPushButton_clicked();

    void on_TesterSerialReadyRead();
    void on_TesteeSerialReadyRead();

    void on_testerCommandSendPushButton_clicked();

    void on_testeeCommandSendPushButton_clicked();

    void on_xvisorBootPushButton_clicked();

    void on_clearPlotsPushButton_clicked();

    void on_readyParse();

    void on_clearPlots2PushButton_clicked();

    void on_CollectDataRadioButton_toggled(bool checked);

signals:
    void readyParse();

private:
    Ui::MainWindow *ui;

    void updatePlot0();
    void updatePlot1();
    void updatePlot2();

    QSerialPort *mTesterSerialPort;
    QSerialPort *mTesteeSerialPort;

    QVector<double> mKeys;
    QVector<double> mData;
    QVector<double> mPerfData;
    QVector<double> mBuckets;
    QVector<double> mCounts;
    QVector<QString> mTickLabels;

    QVector<QString> mTickLabelsMeanPlot;
    QVector<double> mBucketsMeanPlot;
    QVector<double> mCountsMeanPlot;


    QCPBars *mCountBars;
    QCPBars *mCountMeanBars;

    QString mParseString;
};

#endif // MAINWINDOW_H
