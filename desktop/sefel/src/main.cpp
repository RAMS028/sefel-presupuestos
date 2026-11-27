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
        QMessageBox message(QMessageBox::Critical, "No se pudo iniciar SEFEL", error, QMessageBox::Ok);
        const int detailsAt = error.indexOf("\n\nDetalle técnico");
        if (detailsAt >= 0) {
            message.setText(error.left(detailsAt));
            message.setDetailedText(error.mid(detailsAt + 2));
        }
        message.exec();
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
