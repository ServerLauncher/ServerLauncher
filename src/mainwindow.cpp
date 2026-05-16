#include <QApplication>
#include <QtConcurrent/QtConcurrent>
#include <spdlog/spdlog.h>

#include "mainwindow.hpp"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->loader_comboBox->addItems({"Fabric", "Forge", "NeoForge", "Paper", "Purpur", "Spigot"});
}

MainWindow::~MainWindow()
{
    delete ui;
}