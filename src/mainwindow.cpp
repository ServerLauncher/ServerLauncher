#include <QApplication>
#include <QtConcurrent/QtConcurrent>
#include <spdlog/spdlog.h>

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "forge_downloader.hpp"
#include "fabric_downloader.hpp"
#include "neoforge_downloader.hpp"
#include "paper_downloader.hpp"
#include "purpur_downloader.hpp"
#include "spigot_downloader.hpp"

NeoForgeDownloader neoforge_downloader;
ForgeDownloader forge_downloader;
FabricDownloader fabric_downloader;
PaperDownloader paper_downloader;
PurpurDownloader purpur_downloader;
SpigotDownloader spigot_downloader;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

ui->loader_comboBox->addItems({"Fabric", "Forge", "NeoForge", "Paper", "Purpur", "Spigot"});

    QThreadPool::globalInstance()->start([]() {forge_downloader.getListOfMcVer();});
    QThreadPool::globalInstance()->start([]() {neoforge_downloader.getListOfMcVer();});
    QThreadPool::globalInstance()->start([]() {spigot_downloader.getListOfMcVer();});
    QThreadPool::globalInstance()->start([]() {paper_downloader.getListOfMcVer();});
    QThreadPool::globalInstance()->start([]() {purpur_downloader.getListOfMcVer();});
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_loader_comboBox_currentIndexChanged(int index){
    ui->ver_comboBox->clear();
    ui->build_comboBox->clear();
    
    QString loader = ui->loader_comboBox->itemText(index);
    QStringList list;

    if(loader == "Fabric"){
        for(const auto& ver : fabric_downloader.getListOfMcVer().arr){
            list << QString::fromStdString(ver);
        }
    }
    else if(loader == "Forge") {
        for(const auto& ver : forge_downloader.getListOfMcVer().arr){
            list << QString::fromStdString(ver);
        }
    }
    else if(loader == "NeoForge"){
        for(const auto& ver : neoforge_downloader.getListOfMcVer().arr){
            list << QString::fromStdString(ver);
        }
    }
    else if(loader == "Paper") {
        for(const auto& ver : paper_downloader.getListOfMcVer().arr){
            list << QString::fromStdString(ver);
        }
    }
    else if(loader == "Purpur") {
        for(const auto& ver : purpur_downloader.getListOfMcVer().arr){
            list << QString::fromStdString(ver);
        }
    }
    else if(loader == "Spigot"){
        for(const auto& ver : spigot_downloader.getListOfMcVer().arr){
            list << QString::fromStdString(ver);
        }
    }else {
        spdlog::warn("Loader {} not implemented yet", loader.toStdString());
    }
    ui->ver_comboBox->addItems(list);
}

void MainWindow::on_ver_comboBox_currentIndexChanged(int index)
{
    QString version = ui->ver_comboBox->itemText(index);
    QString loader  = ui->loader_comboBox->currentText();

    if (version.isEmpty()) return;

    spdlog::info("Chosen version: {}", version.toStdString());
    ui->build_comboBox->clear();

    const auto& builds = [&]() -> const BuildList& {
        static BuildList empty;
        if (loader == "Fabric")   return fabric_downloader.getListOfBuild(version.toStdString());
        if (loader == "Forge")    return forge_downloader.getListOfBuild(version.toStdString());
        if (loader == "NeoForge") return neoforge_downloader.getListOfBuild(version.toStdString());
        if (loader == "Paper")    return paper_downloader.getListOfBuild(version.toStdString());
        if (loader == "Purpur")   return purpur_downloader.getListOfBuild(version.toStdString());
        if (loader == "Spigot") {
            ui->build_comboBox->addItem("latest");
            ui->build_comboBox->setEnabled(false);
            return empty;
        }
        return empty;
    }();

    if (!builds.arr.empty()) {
        ui->build_comboBox->setEnabled(true);
        for (const auto& build : builds.arr)
            ui->build_comboBox->addItem(QString::fromStdString(build));
    }
}