#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QByteArray>
#include <QFile>
#include <QTextStream>
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
    void fetchVersion(const QString& uid, const QString& mcVersion);

    QString displayNameToUid(const QString& displayName) const;

    Ui::MainWindow *ui;

    QNetworkAccessManager* m_nam;
    MetaManager* m_metaManager;
    QFile m_log;
    QTextStream m_out;
};

#endif