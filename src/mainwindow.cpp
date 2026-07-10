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

    QDir().mkpath("logs");
    m_log.setFileName("logs/debug.log");
    if (!m_log.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open log file";
    }
    m_out.setDevice(&m_log);

    ui->loader_comboBox->setInsertPolicy(QComboBox::InsertAlphabetically);

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
        if(!pkg) 
            return;

        m_out << "[PACKAGE] name: " << pkg->name
                  << " | versions: " << pkg->versions.size() << "\n";
        m_log.flush();

        if(pkg->name == "Java Runtimes")
            return;
        
        const QString displayName = pkg->name == "Mojang" ? "Vanilla" : pkg-> name;
        if(ui->loader_comboBox->findText(displayName) == -1)
            ui->loader_comboBox->addItem(displayName);
    });

    connect(ui->loader_comboBox, &QComboBox::currentTextChanged, this, [this](const QString& displayName){
        ui->version_comboBox->clear();
        ui->build_comboBox->clear();

        const QString uid = displayNameToUid(displayName);
        if(uid.isEmpty())
            return;

        const MetaPackage* pkg = m_metaManager->package(uid);
        if (!pkg)
            return;

        for (const auto& build : pkg->versions) {
            ui->version_comboBox->addItem(build.mcVersion);
        }
    });

    connect(ui->version_comboBox, &QComboBox::currentTextChanged, this, [this](const QString& mcVersion){
        ui->build_comboBox->clear();
        ui->download_button->setEnabled(false);

        if (mcVersion.isEmpty())
            return;

        const QString displayName = ui->loader_comboBox->currentText();
        const bool isVanilla = (displayName == "Vanilla");

        if (isVanilla) {
            ui->build_comboBox->addItem("N/A");
            ui->build_comboBox->setEnabled(false);
        } else {
            ui->build_comboBox->addItem("Loading...");
            ui->build_comboBox->setEnabled(false);
        }

        const QString uid = displayNameToUid(displayName);
        if (uid.isEmpty())
            return;

        fetchVersion(uid, mcVersion);
    });

    connect(m_metaManager, &MetaManager::packageLoadedFromNetwork, this, [this](const QString& uid){
        m_out << "[OK] Package loaded from network: " << uid << "\n";
        m_log.flush();
    });

    connect(m_metaManager, &MetaManager::versionLoaded, this, [this](const QString& uid, const QString& mcVersion) {
        m_out << "[OK] Version loaded: " << uid << " | " << mcVersion << "\n";

        const MetaVersion* ver = m_metaManager->version(uid, mcVersion);
        if (!ver)
            return;

        m_out << "[VERSION] mcVersion: " << ver->mcVersion
            << " | builds: " << ver->builds.size() << "\n";
        m_log.flush();

        const QString displayName = ui->loader_comboBox->currentText();
        const QString currentUid = displayNameToUid(displayName);
        const QString currentVersion = ui->version_comboBox->currentText();

        if (uid == currentUid && mcVersion == currentVersion) {
            const bool isVanilla = (displayName == "Vanilla");

            if (isVanilla) {
                ui->download_button->setEnabled(!ver->builds.isEmpty());
            } else {
                ui->build_comboBox->clear();
                for (const auto& build : ver->builds) {
                    ui->build_comboBox->addItem(build.build);
                }
                ui->build_comboBox->setEnabled(true);
                ui->download_button->setEnabled(!ver->builds.isEmpty());
            }
        }
    });

    connect(m_metaManager, &MetaManager::loadFailed, this, [this](const QString& error) {
        m_out << "[ERROR] Load failed: " << error << "\n";
        m_log.flush();
    });

    
    connect(m_metaManager, &MetaManager::downloaded, this,
        [this](const QString& uid, const QString& mcVersion, const QString& build, const QString& path) {
        m_out << "[OK] Build downloaded: " << uid << " " << mcVersion << " " << build
              << " -> " << path << "\n";
        m_log.flush();
        ui->download_button->setEnabled(true);
    });

    connect(m_metaManager, &MetaManager::downloadFailed, this,
        [this](const QString& uid, const QString& mcVersion, const QString& build, const QString& error) {
        m_out << "[ERROR] Build download failed: " << uid << " " << mcVersion << " " << build
              << " -> " << error << "\n";
        m_log.flush();
        ui->download_button->setEnabled(true);
    });

    connect(ui->download_button, &QPushButton::clicked, this, [this]() {
        const QString displayName = ui->loader_comboBox->currentText();
        const QString uid = displayNameToUid(displayName);
        const QString mcVersion = ui->version_comboBox->currentText();
        const bool isVanilla = (displayName == "Vanilla");

        const QString build = isVanilla ? QString() : ui->build_comboBox->currentText();

        if (uid.isEmpty() || mcVersion.isEmpty()) {
            m_out << "[WARN] Download requested with incomplete selection\n";
            m_log.flush();
            return;
        }

        if (!isVanilla && (build.isEmpty() || !ui->build_comboBox->isEnabled())) {
            m_out << "[WARN] Download requested with incomplete build selection\n";
            m_log.flush();
            return;
        }

        auto task = m_metaManager->download(uid, mcVersion, build);
        if (!task) {
            return;
        }

        ui->download_button->setEnabled(false);
        task->start();
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

void MainWindow::fetchVersion(const QString& uid, const QString& mcVersion) {
    if (m_metaManager->isVersionLoaded(uid, mcVersion)) {
        m_out << "[INFO] Version already loaded from cache: " << uid << " " << mcVersion << "\n";
        m_log.flush();

        const MetaVersion* ver = m_metaManager->version(uid, mcVersion);
        if (ver) {
            const bool isVanilla = (ui->loader_comboBox->currentText() == "Vanilla");
            if (isVanilla) {
                ui->download_button->setEnabled(!ver->builds.isEmpty());
            } else {
                ui->build_comboBox->clear();
                for (const auto& build : ver->builds) {
                    ui->build_comboBox->addItem(build.build);
                }
                ui->build_comboBox->setEnabled(true);
                ui->download_button->setEnabled(!ver->builds.isEmpty());
            }
        }
        return;
    }

    m_out << "[INFO] Fetching version: " << uid << " " << mcVersion << "\n";
    m_log.flush();

    auto versionTask = m_metaManager->loadVersion(uid, mcVersion);

    connect(versionTask, &Task::completed, this, [this, uid, mcVersion]() {
        m_out << "[OK] Version fetched: " << uid << " " << mcVersion << "\n";
        m_log.flush();
    });
    connect(versionTask, &Task::failed, this, [this](const QString& msg) {
        m_out << "[ERROR] Version fetch failed: " << msg << "\n";
        m_log.flush();
    });

    versionTask->start();
}

QString MainWindow::displayNameToUid(const QString& displayName) const {
    const auto& platforms = m_metaManager->index().platforms;
    for (const auto& platform : platforms) {
        if (platform.name == "Java Runtimes")
            continue;
        const QString name = platform.name == "Mojang" ? "Vanilla" : platform.name;
        if (name == displayName)
            return platform.uid;
    }
    return {};
}

MainWindow::~MainWindow() {
    delete ui;
}