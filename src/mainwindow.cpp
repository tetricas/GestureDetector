#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QThread>
#include <QPixmap>

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
    m_thread->quit();
    if(!m_thread->wait(10))
    {
        m_thread->terminate();
        m_thread->wait();
    }
    delete m_ui;
}

void MainWindow::start()
{
    m_thread = new QThread(this);
    COpenCVProcessor* m_processor = new COpenCVProcessor();

    connect(m_thread, &QThread::started, m_processor, &COpenCVProcessor::run);
    connect(this, &MainWindow::canCaptureBack, m_processor, &COpenCVProcessor::captureBackground, Qt::DirectConnection);
    connect(this, &MainWindow::stop, m_processor, &COpenCVProcessor::stop, Qt::DirectConnection);
    connect(m_processor, &COpenCVProcessor::processedImage, this, &MainWindow::getProcessedImage);

    connect(m_processor, &COpenCVProcessor::finished, m_thread, &QThread::quit);
    connect(m_processor, &COpenCVProcessor::finished, m_processor, &COpenCVProcessor::deleteLater);

    m_processor->moveToThread(m_thread);
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

void MainWindow::getProcessedImage(QImage image)
{
    m_ui->imageLabel->setPixmap(QPixmap::fromImage(image));
}
