#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <vector>
#include <array>

#include <QThread>
#include <QtDebug>

#include "opencv_processor.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_isFirstClick(true)
{
    ui->setupUi(this);
    ui->captureButton->setText("Remove hand from the frame and press to capture background");
    connect(ui->captureButton, &QPushButton::clicked, this, &MainWindow::captureClicked);
    connect(ui->stopButton, &QPushButton::clicked, this, &MainWindow::stopClicked);
}

MainWindow::~MainWindow()
{
    emit stop();
    delete ui;
}

void MainWindow::start()
{
    COpenCVProcessor* worker = new COpenCVProcessor();
//    thread = new QThread;
//    worker->moveToThread(thread);

    //connect(thread, &QThread::started, worker, &COpenCVProcessor::run);
    connect(this, &MainWindow::canCaptureBack, worker, &COpenCVProcessor::captureBackground);
    connect(this, &MainWindow::stop, worker, &COpenCVProcessor::stop);

    //connect(worker, &COpenCVProcessor::finished, thread, &QThread::quit);
    connect(worker, &COpenCVProcessor::finished, worker, &COpenCVProcessor::deleteLater);
    //connect(thread, &QThread::finished, worker, &QThread::deleteLater);

    //thread->start();
    worker->run();
}

void MainWindow::captureClicked()
{
    ui->captureButton->setText("Place hand inside boxes");
    emit canCaptureBack();
}

void MainWindow::stopClicked()
{
    emit stop();
}
