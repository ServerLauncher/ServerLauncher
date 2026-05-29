#pragma once
#include "Validator.hpp"
#include <QCryptographicHash>
#include <QFile>

class HashValidator : public Validator {
QString algorithmName() const {
    switch(m_algorithm) {
        case QCryptographicHash::Md5:
            return "MD5";

        case QCryptographicHash::Sha1:
            return "SHA1";

        case QCryptographicHash::Sha224:
            return "SHA224";

        case QCryptographicHash::Sha256:
            return "SHA256";

        case QCryptographicHash::Sha384:
            return "SHA384";

        case QCryptographicHash::Sha512:
            return "SHA512";

        default:
            return "UnknownHash";
    }
}

public:
    HashValidator(QCryptographicHash::Algorithm algorithm, const QString& expectedHash)
        : m_algorithm(algorithm), m_expectedHash(expectedHash), m_hash(algorithm)
    { }

    bool init(QNetworkRequest& request) override {
        Q_UNUSED(request);
        m_hash.reset();
        return true;
    }

    bool write(QByteArray* data) override {
        if(!data) {
            setErrorMessage("No data to hash");
            return false;
        }
        m_hash.addData(*data);
        return true;
    }

    bool abort() override {
        m_hash.reset();
        return true;
    }

    bool validate(QNetworkReply& reply) override {
        Q_UNUSED(reply);

        const QByteArray result = m_hash.result();
        if (result.isEmpty()) {
            setErrorMessage(QString("%1 hash is empty").arg(algorithmName()));
            return false;
        }

        const QString actualHash = QString(result.toHex()).toLower();
        if (actualHash != m_expectedHash) {
            setErrorMessage(
                QString("%1 mismatch. Expected: %2 Actual: %3")
                .arg(algorithmName())
                .arg(m_expectedHash)
                .arg(actualHash));
            return false;
        }
        return true;
    }
private:
    QCryptographicHash::Algorithm m_algorithm;
    QString m_expectedHash;
    QCryptographicHash m_hash;
};