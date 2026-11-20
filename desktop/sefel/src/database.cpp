#include "database.h"
#include <QSqlError>

bool Database::connect(QString *error) {
    auto database = QSqlDatabase::addDatabase("QPSQL");
    database.setHostName(qEnvironmentVariable("SEFEL_DB_HOST", "localhost"));
    bool portOk = false;
    const int port = qEnvironmentVariable("SEFEL_DB_PORT", "5432").toInt(&portOk);
    database.setPort(portOk ? port : 5432);
    database.setDatabaseName(qEnvironmentVariable("SEFEL_DB_NAME", "sefel"));
    database.setUserName(qEnvironmentVariable("SEFEL_DB_USER", "postgres"));
    database.setPassword(qEnvironmentVariable("SEFEL_DB_PASSWORD"));
    if (!database.open()) {
        if (error) *error = database.lastError().text();
        return false;
    }
    return true;
}
QSqlDatabase Database::db() { return QSqlDatabase::database(); }
