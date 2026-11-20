#pragma once
#include <QSqlDatabase>
#include <QString>

class Database {
public:
    static bool connect(QString *error = nullptr);
    static QSqlDatabase db();
};
