#include "database.h"
#include <QSqlError>

bool Database::connect(QString *error) {
    if (!QSqlDatabase::isDriverAvailable("QPSQL")) {
        if (error) {
            *error = "No está instalado o no se encuentra el controlador QPSQL de Qt. "
                     "Instala/despliega el plugin sqldrivers/qsqlpsql.dll y las bibliotecas de PostgreSQL compatibles.\n\n"
                     "Detalle técnico: Qt no encuentra el controlador QPSQL.";
        }
        return false;
    }
    auto database = QSqlDatabase::addDatabase("QPSQL");
    const QString host = qEnvironmentVariable("SEFEL_DB_HOST", "localhost").trimmed();
    bool portOk = false;
    const QString portText = qEnvironmentVariable("SEFEL_DB_PORT", "5432").trimmed();
    const int port = portText.toInt(&portOk);
    if (!portOk || port < 1 || port > 65535) {
        if (error) *error = "El puerto configurado en SEFEL_DB_PORT no es válido. Usa un número entre 1 y 65535 (normalmente 5432).";
        database.close();
        return false;
    }
    const QString databaseName = qEnvironmentVariable("SEFEL_DB_NAME", "sefel").trimmed();
    const QString userName = qEnvironmentVariable("SEFEL_DB_USER", "postgres").trimmed();
    database.setHostName(host);
    database.setPort(port);
    database.setDatabaseName(databaseName);
    database.setUserName(userName);
    database.setPassword(qEnvironmentVariable("SEFEL_DB_PASSWORD"));
    if (!database.open()) {
        const QSqlError sqlError = database.lastError();
        const QString code = sqlError.nativeErrorCode();
        QString guidance;
        if (code == "28P01" || code == "28000") {
            guidance = "PostgreSQL rechazó las credenciales. Revisa SEFEL_DB_USER y SEFEL_DB_PASSWORD.";
        } else if (code == "3D000") {
            guidance = QString("No existe la base de datos '%1'. Créala en PostgreSQL y carga database/schema.sql.").arg(databaseName);
        } else if (code.startsWith("08") || sqlError.text().contains("could not connect", Qt::CaseInsensitive)
                   || sqlError.text().contains("connection refused", Qt::CaseInsensitive)) {
            guidance = QString("No se pudo contactar PostgreSQL en %1:%2. Comprueba que el servicio esté iniciado, el host y puerto sean correctos y el firewall permita la conexión.").arg(host).arg(port);
        } else {
            guidance = "No se pudo abrir la conexión con PostgreSQL. Revisa el servicio, la configuración SEFEL_DB_* y los permisos del usuario.";
        }
        if (error) *error = guidance + "\n\nDetalle técnico (SQLSTATE " + (code.isEmpty() ? QString("desconocido") : code) + "): " + sqlError.text();
        database.close();
        return false;
    }
    return true;
}
QSqlDatabase Database::db() { return QSqlDatabase::database(); }
