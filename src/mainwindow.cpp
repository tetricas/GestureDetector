#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <vector>
#include <array>

#include <QThread>
#include <QtDebug>

#include "opencv_processor.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_isFirstClick(true)
{
    m_ui->setupUi(this);
    m_ui->captureButton->setText("Remove hand from the frame and press to capture background");

    connect(m_ui->captureButton, &QPushButton::clicked, this, &MainWindow::captureClicked);
    connect(m_ui->stopButton, &QPushButton::clicked, this, &MainWindow::stopClicked);
}

MainWindow::~MainWindow()
{
    emit stop();
    delete m_ui;
}

void MainWindow::start()
{
    m_thread = new QThread(this);
    COpenCVProcessor* m_processor = new COpenCVProcessor(m_thread);

    connect(m_thread, &QThread::started, m_processor, &COpenCVProcessor::run);
    connect(this, &MainWindow::canCaptureBack, m_processor, &COpenCVProcessor::captureBackground);
    connect(this, &MainWindow::stop, m_processor, &COpenCVProcessor::stop);

    connect(m_processor, &COpenCVProcessor::finished, m_thread, &QThread::terminate);
    connect(m_processor, &COpenCVProcessor::finished, m_processor, &COpenCVProcessor::deleteLater);

    //m_processor->moveToThread(m_thread);
    m_thread->start();
}

void MainWindow::captureClicked()
{
    m_ui->captureButton->setText("Place hand inside boxes");
    emit canCaptureBack();
}

void MainWindow::stopClicked()
{
    emit stop();
}
