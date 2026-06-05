#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QByteArray>
#include <QFile>
#include "meta/MetaManager.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void fetchIndex();
    void fetchPackages();

    Ui::MainWindow *ui;

    QNetworkAccessManager* m_nam;
    QByteArray m_metaData;
    QString m_baseUrl = "https://serverlauncher.github.io/meta-launcher/";
    MetaManager* m_metaManager;
    QFile m_log;
    QTextStream m_out;
};

#endif