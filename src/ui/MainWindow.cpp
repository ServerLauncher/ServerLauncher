#include "MainWindow.hpp"
#include "ui_mainwindow.h"
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    ui->menuBar->setVisible(false);
#endif
}

MainWindow::~MainWindow() {
    delete ui;
}

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
void MainWindow::keyReleaseEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Alt) {
        ui->menuBar->setVisible(!ui->menuBar->isVisible());
        event->accept();
    } else {
        QMainWindow::keyReleaseEvent(event);
    }
}
#endif