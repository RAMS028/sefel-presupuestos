#pragma once
#include <QDialog>
#include <QMainWindow>
#include <QSqlQueryModel>
#include <QTableWidget>

class QLineEdit;
class QComboBox;
class QDoubleSpinBox;
class QTabWidget;
class QLabel;

class LoginWindow : public QDialog {
    Q_OBJECT
public:
    explicit LoginWindow(QWidget *parent = nullptr);
    int userId() const { return m_userId; }
    QString userName() const { return m_userName; }
    QString role() const { return m_role; }
private slots:
    void authenticate();
private:
    QLineEdit *m_email;
    QLineEdit *m_password;
    int m_userId = 0;
    QString m_userName, m_role;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(int userId, const QString &userName, const QString &role, QWidget *parent = nullptr);
signals:
    void logoutRequested();
private slots:
    void logout();
    void reloadResources();
    void addResource();
    void changePrice();
    void showPriceHistory();
    void addApuLine();
    void calculateBudget();
    void saveBudget();
    void exportBudget();
private:
    void buildResourcesTab();
    void buildPricesTab();
    void buildBudgetTab();
    void refreshCodePreview();
    QString generateResourceCode() const;
    int selectedResourceId() const;
    int m_userId;
    QString m_userName, m_role;
    bool m_canEdit = false;
    QTabWidget *m_tabs;
    QLineEdit *m_resourceFilter, *m_code, *m_description, *m_unit;
    QComboBox *m_type;
    QDoubleSpinBox *m_initialPrice;
    QTableWidget *m_resources, *m_apuLines;
    QComboBox *m_priceResource;
    QDoubleSpinBox *m_newPrice, *m_markup;
    QLineEdit *m_budgetName, *m_apuName, *m_apuUnit;
    QDoubleSpinBox *m_lineQty;
    QComboBox *m_lineResource;
    QLabel *m_subtotalLabel, *m_totalLabel;
    double m_subtotal = 0;
};
