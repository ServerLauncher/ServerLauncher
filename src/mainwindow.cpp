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
    m_out << "[INFO] URL: " << m_baseUrl << "\n";
    m_log.flush();

    auto request = std::make_shared<NetRequest>();
    request->url = QUrl(m_baseUrl);
    request->sink = std::make_unique<ByteArraySink>(&m_metaData);
    m_out << "[INFO] Request created for: " << request->url.toString() << "\n";
    m_log.flush();

    auto job = new NetJob("Fetch meta", m_nam, this);
    job->addRequest(request);
    m_out << "[INFO] NetJob created with 1 request\n";
    m_log.flush();

    connect(job, &Task::completed, this, [this](){
        m_out << "[OK] Job completed, received " << m_metaData.size() << " bytes\n";
        m_log.flush();

        auto doc = QJsonDocument::fromJson(m_metaData);
        if (doc.isNull()) {
            m_out << "[ERROR] Failed to parse JSON\n";
            m_log.flush();
            return;
        }

        auto platforms = doc.object()["platforms"].toArray();
        m_out << "[INFO] Found " << platforms.size() << " platform(s)\n";
        m_log.flush();

        for (const auto& entry : platforms) {
            auto obj = entry.toObject();
            m_out << "[PLATFORM] uid: " << obj["uid"].toString()
                  << " | name: " << obj["name"].toString()
                  << " | url: " << obj["url"].toString()
                  << " | sha256: " << obj["sha256"].toString().left(16) << "...\n";
            m_log.flush();
        }
    });

    connect(job, &Task::failed, this, [this](const QString& msg){
        m_out << "[ERROR] Job failed: " << msg << "\n";
        m_log.flush();
    });

    connect(job, &Task::aborted, this, [this](){
        m_out << "[WARN] Job aborted\n";
        m_log.flush();
    });

    connect(job, &Task::progress, this, [this](qint64 current, qint64 total, const QString& msg){
        m_out << "[PROGRESS] " << current << "/" << total;
        if (!msg.isEmpty()) m_out << " (" << msg << ")";
        m_out << "\n";
        m_log.flush();
    });

    m_out << "[INFO] Starting job...\n";
    m_log.flush();
    job->start();
    m_out << "[INFO] Job started\n";
    m_log.flush();
}