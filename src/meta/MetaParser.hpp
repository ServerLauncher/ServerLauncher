#pragma once
#include <QByteArray>
#include "MetaInfo.hpp"

class MetaParser {
public:
    static bool parseIndex(const QByteArray& data, MetaIndex& index, QString& errorMessage);
    static bool parsePackage(const QByteArray& data, MetaPackage& package, QString& errorMessage);
};