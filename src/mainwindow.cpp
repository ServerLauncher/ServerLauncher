#include <QApplication>
#include <QtConcurrent/QtConcurrent>

#include "mainwindow.hpp"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->loader_comboBox->addItems({":)", ":D", ":P", ":3", ":O", ":|", ":(", ">:("});
}

MainWindow::~MainWindow()
{
    delete ui;
}