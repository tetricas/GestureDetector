#pragma once

#include <vector>

#include <QMainWindow>
#include <QMouseEvent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void start();

signals:
    void canCaptureBack();
    void canCaptureHand();
    void stop();

private slots:
    void captureClicked();
    void stopClicked();

private:
    Ui::MainWindow *ui;

    QThread* thread;
    bool m_isFirstClick;
};
