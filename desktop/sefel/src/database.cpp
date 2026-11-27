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
    const QString host = qEnvironmentVariable("SEFEL_DB_HOST", "localhost").trimmed();
    bool portOk = false;
    const QString portText = qEnvironmentVariable("SEFEL_DB_PORT", "5432").trimmed();
    const int port = portText.toInt(&portOk);
    if (!portOk || port < 1 || port > 65535) {
        if (error) *error = "El puerto configurado en SEFEL_DB_PORT no es válido. Usa un número entre 1 y 65535 (normalmente 5432).";
        return false;
    }
    const QString databaseName = qEnvironmentVariable("SEFEL_DB_NAME", "sefel").trimmed();
    const QString userName = qEnvironmentVariable("SEFEL_DB_USER", "postgres").trimmed();
    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection)) {
        QSqlDatabase previous = QSqlDatabase::database();
        previous.close();
        previous = QSqlDatabase();
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    }
    auto database = QSqlDatabase::addDatabase("QPSQL");
    database.setHostName(host);
    database.setPort(port);
    database.setDatabaseName(databaseName);
    database.setUserName(userName);
    database.setPassword(qEnvironmentVariable("SEFEL_DB_PASSWORD"));
    if (!database.open()) {
        const QSqlError sqlError = database.lastError();
        const QString code = sqlError.nativeErrorCode();
        const QString technical = sqlError.text();
        const QString technicalLower = technical.toLower();
        QString guidance;
        if (code == "28P01" || code == "28000"
            || technicalLower.contains("no password supplied")
            || technicalLower.contains("password authentication failed")
            || technicalLower.contains("authentication failed")) {
            guidance = "No se recibió la contraseña o PostgreSQL rechazó las credenciales. Revisa SEFEL_DB_USER y SEFEL_DB_PASSWORD.";
        } else if (code == "3D000") {
            guidance = QString("No existe la base de datos '%1'. Créala en PostgreSQL y carga database/schema.sql.").arg(databaseName);
        } else if (code.startsWith("08") || technicalLower.contains("could not connect")
                   || technicalLower.contains("connection refused")
                   || technicalLower.contains("timeout expired")) {
            guidance = QString("No se pudo contactar PostgreSQL en %1:%2. Comprueba que el servicio esté iniciado, el host y puerto sean correctos y el firewall permita la conexión.").arg(host).arg(port);
        } else if (technicalLower.contains("no pg_hba.conf entry")) {
            guidance = "El servidor PostgreSQL no permite conexiones para este usuario, equipo o método de autenticación. Revisa pg_hba.conf.";
        } else if (technicalLower.contains("no route to host") || technicalLower.contains("could not translate host name")) {
            guidance = QString("No se pudo localizar el servidor '%1'. Revisa SEFEL_DB_HOST y la conexión de red.").arg(host);
        } else {
            guidance = "No se pudo abrir la conexión con PostgreSQL. Revisa el servicio, la configuración SEFEL_DB_* y los permisos del usuario.";
        }
        if (error) *error = guidance + "\n\nDetalle técnico (SQLSTATE " + (code.isEmpty() ? QString("desconocido") : code) + "): " + technical;
        database.close();
        return false;
    }
    return true;
}
QSqlDatabase Database::db() { return QSqlDatabase::database(); }
