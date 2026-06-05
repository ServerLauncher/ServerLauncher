#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "meta/MetaManager.hpp"
#include "tasks/ConcurrentTask.hpp"
#include <QDebug>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_nam(new QNetworkAccessManager(this))
    , m_metaManager(new MetaManager(
          QDir::currentPath() + "/cache",
          "https://serverlauncher.github.io/meta-launcher/",
          m_nam,
          this))
{
    ui->setupUi(this);
    ui->loader_comboBox->addItems({":)", ":D", ":P", ":3", ":O", ":|", ":(", ">:("});

    QDir().mkpath("logs");
    m_log.setFileName("logs/debug.log");
    if (!m_log.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open log file";
    }
    m_out.setDevice(&m_log);

    connect(m_metaManager, &MetaManager::indexLoaded, this, [this]() {
        m_out << "[OK] Meta index loaded\n";

        const auto& index = m_metaManager->index();
        m_out << "[INFO] Found " << index.platforms.size() << " platform(s)\n";

        for (const auto& platform : index.platforms) {
            m_out << "[PLATFORM] uid: " << platform.uid
                  << " | name: " << platform.name
                  << " | url: " << platform.url
                  << " | sha256: " << platform.sha256.left(16) << "...\n";
        }
        m_log.flush();
    });

    connect(m_metaManager, &MetaManager::indexLoadedFromNetwork, this, [this]() {
        fetchPackages();
    });

    connect(m_metaManager, &MetaManager::packageLoaded, this, [this](const QString& uid) {
        m_out << "[OK] Package loaded: " << uid << "\n";
        const MetaPackage* pkg = m_metaManager->package(uid);
        if (pkg) {
            m_out << "[PACKAGE] name: " << pkg->name
                  << " | versions: " << pkg->versions.size() << "\n";
        }
        m_log.flush();
    });

    connect(m_metaManager, &MetaManager::loadFailed, this, [this](const QString& error) {
        m_out << "[ERROR] Load failed: " << error << "\n";
        m_log.flush();
    });

    fetchIndex();
}

void MainWindow::fetchIndex() {
    m_out << "[INFO] fetchMeta called\n";
    m_out << "[INFO] URL: https://serverlauncher.github.io/meta-launcher/\n";
    m_log.flush();

    m_metaManager->init();

    if (m_metaManager->isIndexLoaded()) {
        m_out << "[INFO] Index loaded from disk cache\n";
        m_log.flush();
    }

    auto concurrent = new ConcurrentTask("Fetch meta", 1, this);
    auto indexTask = m_metaManager->loadIndex();
    m_out << "[INFO] LoadMetaTask created\n";
    m_log.flush();

    concurrent->addTask(indexTask);

    connect(concurrent, &Task::completed, this, [this]() {
        m_out << "[OK] Job completed, received "
              << m_metaManager->indexCache()->lastDataSize() << " bytes\n";
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

void MainWindow::fetchPackages() {
    const auto& platforms = m_metaManager->index().platforms;
    if (platforms.isEmpty()) {
        m_out << "[WARN] No platforms to fetch\n";
        m_log.flush();
        return;
    }

    m_out << "[INFO] Fetching " << platforms.size() << " package(s)...\n";
    m_log.flush();

    auto concurrent = new ConcurrentTask("Fetch Packages", 3, this);

    for(const auto& platform : platforms) {
        m_out << "[INFO] Adding package task: " << platform.uid << "\n";
        auto packageTask = m_metaManager->loadPackage(platform.uid);
        concurrent->addTask(packageTask);
    }

    connect(concurrent, &Task::completed, this, [this]() {
        m_out << "[OK] All packages fetched\n";
        m_log.flush();
    });
    connect(concurrent, &Task::failed, this, [this](const QString& msg) {
        m_out << "[ERROR] Packages job failed: " << msg << "\n";
        m_log.flush();
    });
    connect(concurrent, &Task::aborted, this, [this]() {
        m_out << "[WARN] Packages job aborted\n";
        m_log.flush();
    });
    connect(concurrent, &Task::progress, this, [this](qint64 current, qint64 total, const QString& msg) {
        m_out << "[PROGRESS] packages " << current << "/" << total;
        if (!msg.isEmpty()) m_out << " (" << msg << ")";
        m_out << "\n";
        m_log.flush();
    });

    m_out << "[INFO] Starting packages job...\n";
    m_log.flush();
    concurrent->start();
}

MainWindow::~MainWindow() {
    delete ui;
}