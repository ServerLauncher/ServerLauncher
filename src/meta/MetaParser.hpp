#pragma once
#include <QByteArray>
#include "MetaInfo.hpp"

class MetaParser {
public:
    static bool parse(const QByteArray& data, MetaIndex& index, QString& errorMessage);
};