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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mTesterSerialPort(new QSerialPort(this)),
    mTesteeSerialPort(new QSerialPort(this))
{
    ui->setupUi(this);

    connect(mTesterSerialPort, &QSerialPort::readyRead,
            this, &MainWindow::on_TesterSerialReadyRead);
    connect(mTesteeSerialPort, &QSerialPort::readyRead,
            this, &MainWindow::on_TesteeSerialReadyRead);

    // hmm
    connect(this, &MainWindow::readyParse,
            this, &MainWindow::on_readyParse);

    connect(ui->testerCommandLineEdit, &QLineEdit::returnPressed,
            this, &MainWindow::on_testerCommandSendPushButton_clicked);

    connect(ui->testeeCommandLineEdit, &QLineEdit::returnPressed,
            this, &MainWindow::on_testeeCommandSendPushButton_clicked);

    // setup plot
    ui->customPlot0->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    ui->customPlot0->legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    ui->customPlot0->legend->setFont(legendFont);
    ui->customPlot0->legend->setSelectedFont(legendFont);
    ui->customPlot0->legend->setSelectableParts(QCPLegend::spItems);
    ui->customPlot0->yAxis->setLabel("usec");
    ui->customPlot0->xAxis->setLabel("Sample number");
    ui->customPlot0->clearGraphs();
    ui->customPlot0->addGraph();
    //ui->customPlot0->xAxis->setRangeReversed(true);
    ui->customPlot0->graph()->setPen(QPen(Qt::black));
    ui->customPlot0->graph()->setData(mKeys,mData);
    ui->customPlot0->graph()->setName("AXI Timer");
    ui->customPlot0->addGraph();
    ui->customPlot0->graph()->setPen(QPen(Qt::red));
    ui->customPlot0->graph()->setData(mKeys, mPerfData);
    ui->customPlot0->graph()->setName("Perf reg");
    ui->customPlot0->replot();

    ui->customPlot1->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    textTicker->setTicks(mBuckets, mTickLabels);
    ui->customPlot1->yAxis->setLabel("Num samples");
    ui->customPlot1->xAxis->setLabel("usecs");
    ui->customPlot1->xAxis->setTicker(textTicker);
    ui->customPlot1->xAxis->setTickLabelRotation(60);
    ui->customPlot1->xAxis->setSubTicks(false);
    ui->customPlot1->xAxis->setTickLength(0, 10);

    mCountBars = new QCPBars(ui->customPlot1->xAxis, ui->customPlot1->yAxis);
    mCountBars->setName("TESTING");

    //mCountBars->setData(mBuckets,mCounts);
    ui->customPlot1->rescaleAxes();
    ui->customPlot1->replot();

    ui->customPlot2->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    QSharedPointer<QCPAxisTickerText> textTicker2(new QCPAxisTickerText);
    textTicker2->setTicks(mBucketsMeanPlot, mTickLabelsMeanPlot);
    ui->customPlot2->yAxis->setLabel("Num samples");
    ui->customPlot2->xAxis->setLabel("usecs");
    ui->customPlot2->xAxis->setTicker(textTicker2);
    ui->customPlot2->xAxis->setTickLabelRotation(60);
    ui->customPlot2->xAxis->setSubTicks(false);
    ui->customPlot2->xAxis->setTickLength(0, 10);

    mCountMeanBars= new QCPBars(ui->customPlot2->xAxis, ui->customPlot2->yAxis);
    mCountMeanBars->setName("TESTING");

    //mCountBars->setData(mBuckets,mCounts);
    ui->customPlot2->rescaleAxes();
    ui->customPlot2->replot();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_testerConnectPushButton_clicked()
{
   QString s = ui->testerLineEdit->text();

   mTesterSerialPort->setPortName(s);
   mTesterSerialPort->setBaudRate(QSerialPort::Baud115200);
   mTesterSerialPort->setDataBits(QSerialPort::Data8);
   mTesterSerialPort->setParity(QSerialPort::NoParity);
   mTesterSerialPort->setStopBits(QSerialPort::OneStop);
   mTesterSerialPort->setFlowControl(QSerialPort::NoFlowControl);

   if(mTesterSerialPort->open(QIODevice::ReadWrite)) {
       qDebug() << "TESTER SERIAL: OK!";
   } else {
       qDebug() << "TESTER SERIAL: ERROR!";
   }

}

void MainWindow::on_testeeConnectPushButton_clicked()
{
    QString s = ui->testeeLineEdit->text();

    mTesteeSerialPort->setPortName(s);
    mTesteeSerialPort->setBaudRate(QSerialPort::Baud115200);
    mTesteeSerialPort->setDataBits(QSerialPort::Data8);
    mTesteeSerialPort->setParity(QSerialPort::NoParity);
    mTesteeSerialPort->setStopBits(QSerialPort::OneStop);
    mTesteeSerialPort->setFlowControl(QSerialPort::NoFlowControl);

    if(mTesteeSerialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "TESTEEE SERIAL: OK!";
    } else {
        qDebug() << "TESTEE SERIAL: ERROR!";
    }
}

void MainWindow::on_TesterSerialReadyRead()
{
    QByteArray data = mTesterSerialPort->readAll();
    QString str = QString(data);
    ui->testerTextBrowser->insertPlainText(str);
    QScrollBar *sb = ui->testerTextBrowser->verticalScrollBar();
    sb->setValue(sb->maximum());

    if (ui->CollectDataRadioButton->isChecked()) {
        mParseString.append(str);
        emit readyParse();
    }
}

void MainWindow::on_TesteeSerialReadyRead()
{
    QByteArray data = mTesteeSerialPort->readAll();
    ui->testeeTextBrowser->insertPlainText(QString(data));
    QScrollBar *sb = ui->testeeTextBrowser->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void MainWindow::on_testerCommandSendPushButton_clicked()
{
    if( mTesterSerialPort->isOpen()) {

        QString str = ui->testerCommandLineEdit->text();
        ui->testerCommandLineEdit->clear();
        str.append("\n");

        mTesterSerialPort->write(str.toLocal8Bit());
    }
}

void MainWindow::on_testeeCommandSendPushButton_clicked()
{
    if( mTesteeSerialPort->isOpen()) {

        QString str = ui->testeeCommandLineEdit->text();
        ui->testeeCommandLineEdit->clear();
        str.append("\n");

        mTesteeSerialPort->write(str.toLocal8Bit());
    }
}

void MainWindow::on_xvisorBootPushButton_clicked()
{
    mTesteeSerialPort->blockSignals(true);
    QStringList sl = {"ext4load mmc 0:7 0x200000 uvmm.bin\n",
                      "ext4load mmc 0:7 0x800000 one_guest_virt-v8.dtb\n",
                      "ext4load mmc 0:7 0x2000000 udisk.img\n",
                      "bootm 0x200000 0x2000000 0x800000\n"};
    char buf[256];

    if (mTesteeSerialPort->isOpen()) {
        for (QString s : sl) {
          qDebug() << "Writing: " << s;
          mTesteeSerialPort->write(s.toLocal8Bit());
          //mTesteeSerialPort->waitForReadyRead();
          //mTesteeSerialPort->readLine(buf,256);
          //qDebug() << "Read: " << buf;
        }
    }
    mTesteeSerialPort->blockSignals(false);
}

void MainWindow::on_clearPlotsPushButton_clicked()
{
    mKeys.clear(); mData.clear(); mPerfData.clear();
    ui->customPlot0->graph(0)->setData(mKeys,mData);
    ui->customPlot0->graph(1)->setData(mKeys,mPerfData);
    ui->customPlot0->replot();

    mBuckets.clear();
    mCounts.clear();
    //ui->customPlot1->graph(0)->setData(mBuckets,mCounts);
    //ui->customPlot1->replot();
    mCountBars->setData(mBuckets,mCounts);
    ui->customPlot1->rescaleAxes();
    ui->customPlot1->replot();
}


void MainWindow::on_clearPlots2PushButton_clicked()
{
    mBucketsMeanPlot.clear();
    mCountsMeanPlot.clear();
    mCountMeanBars->setData(mBucketsMeanPlot, mCountsMeanPlot);
    ui->customPlot2->rescaleAxes();
    ui->customPlot2->replot();
}


void MainWindow::on_readyParse()
{
    if (mParseString.size() == 0) {
        qDebug() << "Empty parse string\n";
        return;
    }


    QString str = mParseString;     // I hope this is a copy.

    for (int i = str.size(); i > 0; i --) {
        if(str.at(str.size()-1) == '\n')
            break;
        str.chop(1);
    }

    int n = str.size();
    mParseString.remove(0,n);
    QStringList sl = str.split("\r",QString::SkipEmptyParts);

    int point_id = 0;
    double time_perf = 0;
    //int num_ticks = 0;
    double time_axi = 0;
    bool parse_ok = false;

    QString prev = QString("\n");
    for (QString s : sl) {
        QStringList dl = s.split(",");
        if (dl.length() == 4) {
            point_id = dl.at(0).toInt(&parse_ok);
            if (!parse_ok) break;
            time_perf = dl.at(1).toDouble(&parse_ok);
            if (!parse_ok) break;
            //num_ticks = dl.at(2).toInt(&parse_ok);
            //if (!parse_ok) break;
            time_axi = dl.at(3).toDouble(&parse_ok);
            if (!parse_ok) break;
            if (prev.size() > 0 && prev.at(prev.size()-1) == '\n') {
                mKeys.append(point_id);
                mData.append(time_axi);
                mPerfData.append(time_perf);
            }
        } // else no parse!
        prev = QString(s);
    }

    if (mData.size() == 0) {
        qDebug() << "no data\n";
        return;
    }
    updatePlot0();
    updatePlot1();
    updatePlot2();






}


void MainWindow::on_CollectDataRadioButton_toggled(bool checked)
{
    qDebug() << "Toggling data collection: " << checked;
}

void MainWindow::updatePlot0()
{
    ui->customPlot0->graph(0)->setData(mKeys,mData);
    ui->customPlot0->graph(1)->setData(mKeys,mPerfData);
    ui->customPlot0->replot();
}

void MainWindow::updatePlot1()
{
    double max = 0;
    double sum = 0;
    for (double d : mData) {
        if (d > max) max = d;
        sum += d;
    }
    double intervals = max / 30;
    //double num = mData.length();
    //num = num / 100;

    double current_interval = 0;
    mBuckets.clear();
    mCounts.clear();
    for (int i = 0; i < 30; i ++) {
        mBuckets.append((i+1)*intervals);
        int count = 0;
        for (double val : mData) {
            if (val >= current_interval &&
                val < current_interval + intervals) {
                count++;
            }
        }
        current_interval += intervals;
        mCounts.append(static_cast<double>(count));
    }
    //qDebug() << mCounts;
    //qDebug() << mBuckets;

    QVector<double> ticks;
    mTickLabels.clear();
    int i = 0;
    for (double b : mBuckets) {
        ticks.append(static_cast<double>(i++));
        mTickLabels.append(QString::number(b));
    }

    mCountBars->setData(ticks,mCounts);
    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    textTicker->setTicks(ticks, mTickLabels);
    ui->customPlot1->xAxis->setTicker(textTicker);
    ui->customPlot1->rescaleAxes();
    ui->customPlot1->replot();
}

void MainWindow::updatePlot2()
{
    double max = 0;
    double sum = 0;
    for (double d : mData) {
        if (d > max) max = d;
        sum += d;
    }

    double mean = sum / mData.size();

    double sigma = 0;
    for (double d : mData) {
        sigma += ((d - mean) * (d - mean));
    }
    sigma = qSqrt(sigma / mData.size());

    //double d1 = sigma;
    //double d2 = 2 * sigma;
    //double d3 = 3 * sigma;

    mBucketsMeanPlot.clear();
    mCountsMeanPlot.clear();
    double s_10 = sigma / 10.0;
    double x = 0;

    for (int i = 0; i < 10; i ++) { // 20 points within one deviation
        x += s_10;
        mBucketsMeanPlot.prepend(mean - x);
        mBucketsMeanPlot.append(mean + x);
    }
    double s_5 = sigma / 5.0;
    for (int i = 0; i < 5; i ++) {
        x += s_5;
        mBucketsMeanPlot.prepend(mean - x);
        mBucketsMeanPlot.append(mean + x);
    }
    double s_2 = sigma / 2.0;
    for (int i = 0; i < 2; i ++) {
        x += s_2;
        mBucketsMeanPlot.prepend(mean - x);
        mBucketsMeanPlot.append(mean + x);
    }
    //mBucketsMeanPlot.prepend(0);
    mBucketsMeanPlot.append(max);

    double previous = 0;
    for (double bucket : mBucketsMeanPlot) {
        int count = 0;
        for (double val : mData) {
            if (val >= previous &&
                val < bucket) {
                count++;
            }
        }
        previous = bucket;
        mCountsMeanPlot.append(static_cast<double>(count));
    }

    QVector<double> ticksAdapted;
    mTickLabelsMeanPlot.clear();

    int i = 0;
    for (double b : mBucketsMeanPlot) {
        ticksAdapted.append(static_cast<double>(i++));
        mTickLabelsMeanPlot.append(QString::number(b));
    }
    mCountMeanBars->setData(ticksAdapted,mCountsMeanPlot);
    QSharedPointer<QCPAxisTickerText> textTicker2(new QCPAxisTickerText);
    textTicker2->setTicks(ticksAdapted, mTickLabelsMeanPlot);
    ui->customPlot2->xAxis->setTicker(textTicker2);
    ui->customPlot2->rescaleAxes();
    ui->customPlot2->replot();

    //qDebug() << "Standard deviation: " << sigma;


    //ui->customPlot1->graph(0)->setData(mBuckets,mCounts);
    //ui->customPlot1->replot();
}
