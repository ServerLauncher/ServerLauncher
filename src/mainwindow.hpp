#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QByteArray>
#include <QFile>

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
    void fetchMeta();

    Ui::MainWindow *ui;

    QNetworkAccessManager* m_nam;
    QByteArray m_metaData;
    QString m_baseUrl = "https://serverlauncher.github.io/meta-launcher/";
    QFile m_log;
    QTextStream m_out;
};

#endif