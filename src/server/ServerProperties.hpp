#pragma once

#include <QString>
#include <QMap>
#include <QFile>
#include <QTextStream>
#include <QVariant>

class ServerProperties {
public:
    ServerProperties() = default;

    bool loadFromFile(const QString &filePath) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return false;
        }

        m_props.clear();
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty() || line.startsWith('#') || line.startsWith('!')) 
                continue; // Skip comments and empty lines
            
            int eqIdx = line.indexOf('=');
            if (eqIdx != -1) {
                QString key = line.left(eqIdx).trimmed();
                QString value = line.mid(eqIdx + 1).trimmed();
                m_props.insert(key, value);
            }
        }
        file.close();
        return true;
    }

    bool saveToFile(const QString &filePath) const {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return false;
        }

        QTextStream out(&file);
        out << "#Minecraft server properties\n";
        out << "Updated by Server Launcher :3\n";

        for (auto it = m_props.constBegin(); it != m_props.constEnd(); ++it) {
            out << it.key() << "=" << it.value() << "\n";
        }
        file.close();
        return true;
    }

    QString getValue(const QString &key, const QString &defaultValue = QString()) const {
        return m_props.value(key, defaultValue);
    }

    void setValue(const QString &key, const QString &value) {
        m_props.insert(key, value);
    }

    bool containsKey(const QString &key) const {
        return m_props.contains(key);
    }

    QString get(const QString& key, const QString& defaultValue = "") const {
        return m_props.value(key, defaultValue);
    }

    int getInt(const QString& key, int defaultValue = 0) const {
        bool ok = false;
        int val = m_props.value(key).toInt(&ok);
        return ok ? val : defaultValue;
    }

    bool getBool(const QString& key, bool defaultValue = false) const {
        if (!m_props.contains(key)) return defaultValue;
        return m_props.value(key) == "true";
    }

    void set(const QString& key, const QVariant& value) {
        if (value.typeId() == QMetaType::Bool) {
            m_props[key] = value.toBool() ? "true" : "false";
        } else {
            m_props[key] = value.toString();
        }
    }

    const QMap<QString, QString>& properties() const { return m_props; }

private:
    QMap<QString, QString> m_props;
};