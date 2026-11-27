#include "database.h"
#include "mainwindow.h"
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlDatabase>
#include <QVBoxLayout>

static bool connectWithUserSettings() {
    while (true) {
        QDialog dialog;
        dialog.setWindowTitle("Conexión a PostgreSQL | SEFEL");
        dialog.setMinimumWidth(440);
        auto *layout = new QVBoxLayout(&dialog);
        auto *description = new QLabel(
            "Ingresa los datos del servidor PostgreSQL. La contraseña se usa solo durante esta ejecución y no se guarda.");
        description->setWordWrap(true);
        layout->addWidget(description);

        auto *form = new QFormLayout;
        auto *host = new QLineEdit(qEnvironmentVariable("SEFEL_DB_HOST", "localhost"));
        auto *port = new QLineEdit(qEnvironmentVariable("SEFEL_DB_PORT", "5432"));
        port->setValidator(new QIntValidator(1, 65535, port));
        auto *database = new QLineEdit(qEnvironmentVariable("SEFEL_DB_NAME", "sefel"));
        auto *user = new QLineEdit(qEnvironmentVariable("SEFEL_DB_USER", "postgres"));
        auto *password = new QLineEdit(qEnvironmentVariable("SEFEL_DB_PASSWORD"));
        password->setEchoMode(QLineEdit::Password);
        form->addRow("Servidor", host);
        form->addRow("Puerto", port);
        form->addRow("Base de datos", database);
        form->addRow("Usuario PostgreSQL", user);
        form->addRow("Contraseña", password);
        layout->addLayout(form);

        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        buttons->button(QDialogButtonBox::Ok)->setText("Conectar");
        buttons->button(QDialogButtonBox::Cancel)->setText("Salir");
        layout->addWidget(buttons);
        QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() != QDialog::Accepted) return false;
        if (host->text().trimmed().isEmpty() || port->text().isEmpty()
            || database->text().trimmed().isEmpty() || user->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "Faltan datos", "Completa servidor, puerto, base de datos y usuario.");
            continue;
        }

        qputenv("SEFEL_DB_HOST", host->text().trimmed().toUtf8());
        qputenv("SEFEL_DB_PORT", port->text().toUtf8());
        qputenv("SEFEL_DB_NAME", database->text().trimmed().toUtf8());
        qputenv("SEFEL_DB_USER", user->text().trimmed().toUtf8());
        qputenv("SEFEL_DB_PASSWORD", password->text().toUtf8());

        QString error;
        if (Database::connect(&error)) return true;

        QMessageBox message(QMessageBox::Critical, "No se pudo conectar a PostgreSQL",
                            "Revisa los datos ingresados. Puedes corregirlos e intentar nuevamente.",
                            QMessageBox::Ok, &dialog);
        const int detailsAt = error.indexOf("\n\nDetalle técnico");
        if (detailsAt >= 0) {
            message.setDetailedText(error.mid(detailsAt + 2));
        } else {
            message.setDetailedText(error);
        }
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
    if (!connectWithUserSettings()) return 0;

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
