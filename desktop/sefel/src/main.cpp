#include "database.h"
#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("SEFEL Desktop");
    QApplication::setOrganizationName("SEFEL");
    QString error;
    if (!Database::connect(&error)) {
        QMessageBox::critical(nullptr, "No se pudo conectar a PostgreSQL",
            "Verifica las variables SEFEL_DB_* y que la base de datos esté creada.\n\n" + error);
        return 1;
    }
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
