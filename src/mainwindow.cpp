#include <QApplication>
#include <QtConcurrent/QtConcurrent>

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "net/NetJob.hpp"
#include "net/NetRequest.hpp"
#include "net/ByteArraySink.hpp"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_nam(new QNetworkAccessManager(this))
    , m_metaManager(new MetaManager("cache", m_baseUrl + "index.json", m_nam, this))
{
    ui->setupUi(this);

    ui->loader_comboBox->addItems({":)", ":D", ":P", ":3", ":O", ":|", ":(", ">:("});

    QDir().mkpath("logs");

    m_log.setFileName("logs/debug.log");
    if (!m_log.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open log file";
    }
    m_out.setDevice(&m_log);

    fetchMeta();
}

void MainWindow::fetchMeta() {
    m_out << "[INFO] fetchMeta called\n";
    m_out << "[INFO] URL: https://serverlauncher.github.io/meta-launcher/index.json\n";
    m_log.flush();

    m_metaManager->init();

    if (m_metaManager->isLoaded()) {
        m_out << "[INFO] Loaded from disk cache\n";
        m_log.flush();
    }

    auto concurrent = new ConcurrentTask("Fetch meta", 1, this);
    auto loadTask = m_metaManager->load();
    m_out << "[INFO] LoadMetaTask created\n";
    m_log.flush();

    concurrent->addTask(loadTask);

    connect(concurrent, &Task::completed, this, [this]() {
        m_out << "[OK] Job completed, received "
              << m_metaManager->cache()->lastDataSize() << " bytes\n";
        m_log.flush();
    });
    connect(concurrent, &Task::failed, this, [this](const QString& msg) {
        m_out << "[ERROR] Job failed: " << msg << "\n";
        m_log.flush();
    });
    connect(concurrent, &Task::aborted, this, [this]() {
        m_out << "[WARN] Job aborted\n";
        m_log.flush();
    });
    connect(concurrent, &Task::progress, this, [this](qint64 current, qint64 total, const QString& msg) {
        m_out << "[PROGRESS] " << current << "/" << total;
        if (!msg.isEmpty()) m_out << " (" << msg << ")";
        m_out << "\n";
        m_log.flush();
    });

    m_out << "[INFO] Starting job...\n";
    m_log.flush();
    concurrent->start();
    m_out << "[INFO] Job started\n";
    m_log.flush();
}

MainWindow::~MainWindow() {
    delete ui;
}