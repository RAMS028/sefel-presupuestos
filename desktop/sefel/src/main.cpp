#include "database.h"
#include "mainwindow.h"
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QSettings>
#include <QSqlDatabase>

struct DatabaseConfig {
    QString host;
    QString port;
    QString name;
    QString user;
};

static bool loadDatabaseConfig(DatabaseConfig *config) {
    QDir directory(QCoreApplication::applicationDirPath());
    QString configPath;
    for (int level = 0; level < 6; ++level) {
        const QString candidate = directory.filePath("configbd.txt");
        if (QFileInfo::exists(candidate)) {
            configPath = candidate;
            break;
        }
        if (!directory.cdUp()) break;
    }
    if (configPath.isEmpty()) {
        QMessageBox::critical(nullptr, "Falta configbd.txt",
            "No se encontró configbd.txt junto al programa o en las carpetas superiores.\n\n"
            "Crea el archivo con estas líneas:\n"
            "host=localhost\nport=5432\ndatabase=sefel\nuser=postgres");
        return false;
    }

    QSettings settings(configPath, QSettings::IniFormat);
    settings.setFallbacksEnabled(false);
    config->host = settings.value("host").toString().trimmed();
    config->port = settings.value("port").toString().trimmed();
    config->name = settings.value("database").toString().trimmed();
    config->user = settings.value("user").toString().trimmed();
    bool portOk = false;
    const int portNumber = config->port.toInt(&portOk);
    if (settings.status() != QSettings::NoError || config->host.isEmpty()
        || !portOk || portNumber < 1 || portNumber > 65535
        || config->name.isEmpty() || config->user.isEmpty()) {
        QMessageBox::critical(nullptr, "configbd.txt no válido",
            QString("Revisa el archivo:\n%1\n\nDebe tener servidor, puerto, base de datos y usuario. El puerto debe estar entre 1 y 65535.")
                .arg(configPath));
        return false;
    }
    return true;
}

static bool connectToDatabase(const DatabaseConfig &config) {
    qputenv("SEFEL_DB_HOST", config.host.toUtf8());
    qputenv("SEFEL_DB_PORT", config.port.toUtf8());
    qputenv("SEFEL_DB_NAME", config.name.toUtf8());
    qputenv("SEFEL_DB_USER", config.user.toUtf8());

    while (true) {
        bool accepted = false;
        const QString password = QInputDialog::getText(
            nullptr, "Conexión a PostgreSQL",
            QString("Contraseña para %1 en la base %2:").arg(config.user, config.name),
            QLineEdit::Password, QString(), &accepted);
        if (!accepted) return false;
        qputenv("SEFEL_DB_PASSWORD", password.toUtf8());

        QString error;
        if (Database::connect(&error)) return true;

        QMessageBox message(QMessageBox::Critical, "No se pudo conectar a PostgreSQL",
                            "Revisa la contraseña y los datos de configbd.txt. La contraseña no se guarda.",
                            QMessageBox::Ok);
        const int detailsAt = error.indexOf("\n\nDetalle técnico");
        message.setDetailedText(detailsAt >= 0 ? error.mid(detailsAt + 2) : error);
        message.exec();
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("SEFEL Desktop");
    QApplication::setOrganizationName("SEFEL");

    if (!QSqlDatabase::isDriverAvailable("QPSQL")) {
        QString error;
        Database::connect(&error);
        QMessageBox::critical(nullptr, "Controlador PostgreSQL no disponible", error);
        return 1;
    }

    DatabaseConfig config;
    if (!loadDatabaseConfig(&config) || !connectToDatabase(config)) return 0;

    constexpr int logoutExitCode = 42;
    while (true) {
        LoginWindow login;
        if (login.exec() != QDialog::Accepted) return 0;
        MainWindow window(login.userId(), login.userName(), login.role());
        QObject::connect(&window, &MainWindow::logoutRequested, &app,
                         [&app] { app.exit(logoutExitCode); });
        window.show();
        const int result = app.exec();
        if (result != logoutExitCode) return result;
    }
}
