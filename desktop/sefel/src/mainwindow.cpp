#include "mainwindow.h"
#include "database.h"
#include <QComboBox>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFile>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>
#include <QSignalBlocker>
#include <QTextStream>
#include <QStringConverter>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QList>

namespace {
constexpr const char *kColorRed = "#C62828";
constexpr const char *kColorBlack = "#171717";
constexpr const char *kColorGray = "#A8A8A8";
constexpr const char *kColorWhite = "#FFFFFF";

const QString kAppStyleSheet = QString(R"(
    QWidget { background-color: %4; color: %2; }
    QLineEdit, QComboBox, QDoubleSpinBox {
        background-color: %4; color: %2; border: 1px solid %3;
        border-radius: 4px; padding: 4px 6px;
    }
    QLineEdit:focus, QComboBox:focus, QDoubleSpinBox:focus { border: 1px solid %1; }
    QPushButton {
        background-color: %2; color: %4; border: 1px solid %2;
        border-radius: 4px; padding: 6px 12px; font-weight: 600;
    }
    QPushButton:hover { background-color: %1; border-color: %1; }
    QPushButton:disabled { background-color: %3; border-color: %3; color: %4; }
    QTabWidget::pane { border: 1px solid %3; background-color: %4; }
    QTabBar::tab {
        background-color: %3; color: %2; padding: 8px 14px;
        border-top-left-radius: 4px; border-top-right-radius: 4px;
        margin-right: 2px; font-weight: 600;
    }
    QTabBar::tab:selected { background-color: %2; color: %4; }
    QTableWidget {
        background-color: %4; alternate-background-color: #F5F5F5;
        color: %2; gridline-color: %3; border: 1px solid %3;
    }
    QHeaderView::section {
        background-color: %2; color: %4; padding: 6px;
        border: 1px solid %3; font-weight: 600;
    }
)" ).arg(kColorRed, kColorBlack, kColorGray, kColorWhite);

void setAccentButton(QPushButton *button) {
    button->setStyleSheet(QString("background-color:%1;color:%2;border:1px solid %1;")
                              .arg(kColorRed, kColorWhite));
}
} // namespace

static QString databaseErrorGuidance(const QString &state, const QString &technical) {
    if (state == "23505") return "Ya existe un registro con ese código o dato único. Actualiza la búsqueda y usa un valor diferente.";
    if (state == "23503") return "No se puede completar porque el registro está relacionado con otros datos. Revisa las referencias antes de eliminarlo o modificarlo.";
    if (state == "23514" || state == "23502") return "Uno de los datos no cumple las reglas requeridas. Revisa los campos obligatorios, el tipo y los valores permitidos.";
    if (state == "22001") return "Un texto supera la longitud permitida. Acórtalo e inténtalo nuevamente.";
    if (state == "22003" || state == "22P02") return "Un número tiene un formato o valor fuera del rango permitido. Revisa las cantidades, precios y porcentajes.";
    if (state == "42P01") return "Falta una tabla de la aplicación. Ejecuta database/schema.sql sobre la base de datos SEFEL.";
    if (state == "42703") return "Falta una columna esperada por esta versión. Actualiza el esquema ejecutando database/schema.sql.";
    if (state == "42501") return "El usuario de PostgreSQL no tiene permisos para esta operación. Solicita permisos sobre la base SEFEL.";
    if (state == "28P01" || state == "28000") return "PostgreSQL rechazó el usuario o la contraseña configurados.";
    if (state == "3D000") return "La base de datos configurada no existe. Créala y carga database/schema.sql.";
    if (state.startsWith("08") || technical.contains("connection", Qt::CaseInsensitive)
        || technical.contains("server closed", Qt::CaseInsensitive))
        return "Se perdió la conexión con PostgreSQL. Comprueba que el servicio siga iniciado y vuelve a abrir la aplicación.";
    if (state == "40001" || state == "40P01") return "Otro proceso modificó los mismos datos al mismo tiempo. Vuelve a intentar la operación.";
    if (state == "26000") return "La sesión de base de datos quedó desincronizada. Cierra y vuelve a abrir SEFEL; si persiste, revisa la conexión.";
    return "No se pudo completar la operación en PostgreSQL. Revisa los datos y la conexión; usa el detalle técnico si necesitas soporte.";
}

static void showDbError(QWidget *parent, const QSqlQuery &q, const QString &operation = {}) {
    const QSqlError error = q.lastError();
    const QString state = error.nativeErrorCode();
    const QString technical = error.text().isEmpty() ? "PostgreSQL no entregó detalles adicionales." : error.text();
    QMessageBox message(QMessageBox::Critical, "No se pudo completar la operación",
                        operation.isEmpty() ? databaseErrorGuidance(state, technical)
                                            : operation + "\n\n" + databaseErrorGuidance(state, technical),
                        QMessageBox::Ok, parent);
    QString details = "SQLSTATE: " + (state.isEmpty() ? QString("desconocido") : state)
                      + "\nDetalle PostgreSQL: " + technical;
    if (!q.lastQuery().isEmpty()) details += "\n\nConsulta: " + q.lastQuery();
    message.setDetailedText(details);
    message.exec();
}
static QTableWidget *makeTable(QWidget *parent, const QStringList &headers) {
    auto *table = new QTableWidget(parent);
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    return table;
}
static QString codePrefix(const QString &type) {
    if (type == "MATERIAL") return "MAT";
    if (type == "MANO_OBRA") return "MO";
    if (type == "EQUIPO") return "EQP";
    return "HER";
}

LoginWindow::LoginWindow(QWidget *parent) : QDialog(parent) {
    setWindowTitle("SEFEL | Iniciar sesión");
    setMinimumWidth(380);
    setStyleSheet(kAppStyleSheet);
    auto *layout = new QVBoxLayout(this);
    auto *title = new QLabel("Gestión de Presupuestos y Costos");
    title->setStyleSheet(QString("font-size:20px;font-weight:700;color:%1;padding:10px 0;").arg(kColorBlack));
    layout->addWidget(title);
    auto *form = new QFormLayout;
    m_email = new QLineEdit; m_email->setPlaceholderText("usuario@sefel.local");
    m_password = new QLineEdit; m_password->setEchoMode(QLineEdit::Password);
    form->addRow("Correo", m_email); form->addRow("Contraseña", m_password);
    layout->addLayout(form);
    auto *button = new QPushButton("Ingresar");
    setAccentButton(button);
    button->setDefault(true); layout->addWidget(button);
    connect(button, &QPushButton::clicked, this, &LoginWindow::authenticate);
    connect(m_password, &QLineEdit::returnPressed, this, &LoginWindow::authenticate);
}

void LoginWindow::authenticate() {
    QSqlQuery query;
    query.prepare("SELECT u.id_usuario,u.nombre,r.nombre FROM usuario u JOIN rol r USING(id_rol) "
                  "WHERE lower(u.email)=lower(:email) AND u.activo AND u.hash_clave=crypt(:password,u.hash_clave)");
    query.bindValue(":email", m_email->text().trimmed());
    query.bindValue(":password", m_password->text());
    if (!query.exec()) { showDbError(this, query); return; }
    if (!query.next()) {
        QMessageBox::warning(this, "Acceso denegado", "Correo o contraseña incorrectos.");
        m_password->clear(); m_password->setFocus(); return;
    }
    m_userId = query.value(0).toInt(); m_userName = query.value(1).toString(); m_role = query.value(2).toString();
    accept();
}

MainWindow::MainWindow(int userId, const QString &userName, const QString &role, QWidget *parent)
    : QMainWindow(parent), m_userId(userId), m_userName(userName), m_role(role),
      m_canEdit(role == "ENCARGADO_PRESUPUESTOS") {
    setWindowTitle("SEFEL | Presupuestos y costos");
    resize(1120, 720);
    setStyleSheet(kAppStyleSheet);
    auto *central = new QWidget;
    auto *root = new QVBoxLayout(central);
    QString displayRole=role; displayRole.replace('_',' ');
    auto *welcome = new QLabel(QString("%1  ·  %2").arg(userName, displayRole));
    welcome->setStyleSheet(QString("font-size:15px;font-weight:600;color:%1;padding:6px;").arg(kColorBlack));
    auto *sessionBar = new QHBoxLayout;
    auto *logoutButton = new QPushButton("Cerrar sesión");
    sessionBar->addWidget(welcome);
    sessionBar->addStretch();
    sessionBar->addWidget(logoutButton);
    root->addLayout(sessionBar);
    connect(logoutButton, &QPushButton::clicked, this, &MainWindow::logout);
    m_tabs = new QTabWidget; root->addWidget(m_tabs);
    setCentralWidget(central);
    buildResourcesTab(); buildPricesTab(); buildBudgetTab();
    m_tabs->setTabEnabled(1, m_canEdit);
    m_tabs->setTabEnabled(2, m_canEdit);
    if (!m_canEdit) m_tabs->setCurrentIndex(0);
    reloadResources();
    reloadSavedBudgets();
}

void MainWindow::buildResourcesTab() {
    auto *page = new QWidget; auto *layout = new QVBoxLayout(page);
    auto *filterRow = new QHBoxLayout;
    m_resourceFilter = new QLineEdit; m_resourceFilter->setPlaceholderText("Buscar por código o descripción");
    auto *search = new QPushButton("Buscar");
    filterRow->addWidget(m_resourceFilter,1); filterRow->addWidget(search);
    layout->addLayout(filterRow);
    m_resources = makeTable(page,{"ID","Código","Descripción","Unidad","Tipo","Precio vigente"});
    layout->addWidget(m_resources,1);
    auto *form = new QFormLayout;
    auto *row = new QHBoxLayout;
    m_code = new QLineEdit; m_code->setReadOnly(true); m_description = new QLineEdit; m_unit = new QLineEdit;
    m_type = new QComboBox; m_type->addItems({"MATERIAL","MANO_OBRA","EQUIPO","HERRAMIENTA"});
    m_initialPrice = new QDoubleSpinBox; m_initialPrice->setRange(0.10,999999999); m_initialPrice->setDecimals(2); m_initialPrice->setPrefix("$ "); m_initialPrice->setValue(0.10);
    row->addWidget(new QLabel("Código")); row->addWidget(m_code);
    row->addWidget(new QLabel("Descripción")); row->addWidget(m_description,2);
    row->addWidget(new QLabel("Unidad")); row->addWidget(m_unit);
    row->addWidget(m_type); row->addWidget(new QLabel("Precio vigente")); row->addWidget(m_initialPrice);
    form->addRow("Nuevo recurso", row); layout->addLayout(form);
    auto *actions = new QHBoxLayout;
    auto *add = new QPushButton("Registrar recurso");
    auto *remove = new QPushButton("Retirar recurso del catálogo");
    actions->addStretch(); actions->addWidget(remove); actions->addWidget(add); layout->addLayout(actions);
    setAccentButton(add);
    add->setEnabled(m_canEdit); remove->setEnabled(m_canEdit); m_code->setEnabled(m_canEdit); m_description->setEnabled(m_canEdit); m_unit->setEnabled(m_canEdit); m_type->setEnabled(m_canEdit); m_initialPrice->setEnabled(m_canEdit);
    connect(search,&QPushButton::clicked,this,&MainWindow::reloadResources);
    connect(m_resourceFilter,&QLineEdit::returnPressed,this,&MainWindow::reloadResources);
    connect(add,&QPushButton::clicked,this,&MainWindow::addResource);
    connect(remove,&QPushButton::clicked,this,&MainWindow::deleteResource);
    connect(m_type, &QComboBox::currentTextChanged, this, [this] { refreshCodePreview(); });
    connect(m_resources,&QTableWidget::itemSelectionChanged,this,[this]{
        if (selectedResourceId() > 0) {
            for (int i=0;i<m_priceResource->count();++i)
                if (m_priceResource->itemData(i).toInt()==selectedResourceId()) m_priceResource->setCurrentIndex(i);
        }
    });
    m_tabs->addTab(page,"Recursos · HU-01/02");
    refreshCodePreview();
}

void MainWindow::buildPricesTab() {
    auto *page = new QWidget; auto *layout = new QVBoxLayout(page);
    auto *intro = new QLabel("Actualiza el precio vigente. Cada cambio se registra en el historial de forma permanente.");
    intro->setWordWrap(true); layout->addWidget(intro);
    auto *form = new QFormLayout;
    m_priceResource = new QComboBox;
    m_newPrice = new QDoubleSpinBox; m_newPrice->setRange(0.10,999999999); m_newPrice->setDecimals(2); m_newPrice->setPrefix("$ "); m_newPrice->setValue(0.10);
    form->addRow("Recurso",m_priceResource); form->addRow("Nuevo precio",m_newPrice); layout->addLayout(form);
    auto *buttons = new QHBoxLayout;
    auto *update = new QPushButton("Guardar precio"); auto *history = new QPushButton("Ver historial");
    setAccentButton(update); setAccentButton(history);
    buttons->addWidget(update); buttons->addWidget(history); buttons->addStretch(); layout->addLayout(buttons);
    connect(update,&QPushButton::clicked,this,&MainWindow::changePrice);
    connect(history,&QPushButton::clicked,this,&MainWindow::showPriceHistory);
    connect(m_priceResource, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (index < 0) return;
        QSqlQuery q;
        q.prepare("SELECT precio_vigente FROM recurso WHERE id_recurso=:id AND activo");
        q.bindValue(":id",m_priceResource->itemData(index).toInt());
        if (!q.exec()) { showDbError(this,q,"No se pudo cargar el precio vigente."); return; }
        if (q.next()) m_newPrice->setValue(q.value(0).toDouble());
        else QMessageBox::information(this,"Recurso no disponible","El recurso fue retirado o ya no existe. Actualiza el catálogo.");
        q.finish();
    });
    m_tabs->addTab(page,"Precios e historial · HU-03/04");
}

void MainWindow::buildBudgetTab() {
    auto *page = new QWidget; auto *layout = new QVBoxLayout(page);
    auto *form = new QFormLayout;
    m_budgetName = new QLineEdit; m_budgetName->setPlaceholderText("Ej. Radier bodega");
    m_apuName = new QLineEdit; m_apuName->setPlaceholderText("Ej. Hormigón H25");
    m_apuUnit = new QLineEdit("m3");
    m_lineResource = new QComboBox;
    m_lineQty = new QDoubleSpinBox; m_lineQty->setRange(0.0001,9999999); m_lineQty->setDecimals(4); m_lineQty->setValue(1);
    form->addRow("Nombre presupuesto",m_budgetName);
    form->addRow("Actividad APU",m_apuName);
    form->addRow("Unidad APU",m_apuUnit);
    auto *addLineRow = new QHBoxLayout; addLineRow->addWidget(m_lineResource,2); addLineRow->addWidget(new QLabel("Cantidad por unidad")); addLineRow->addWidget(m_lineQty);
    form->addRow("Recurso APU",addLineRow); layout->addLayout(form);
    auto *addLine = new QPushButton("Agregar recurso al APU"); layout->addWidget(addLine,0,Qt::AlignRight);
    connect(addLine,&QPushButton::clicked,this,&MainWindow::addApuLine);
    m_apuLines=makeTable(page,{"Recurso","Unidad","Cantidad APU","Precio vigente","Costo línea","ID"});
    layout->addWidget(m_apuLines,1);
    auto *lineActions = new QHBoxLayout;
    auto *removeLine = new QPushButton("Quitar recurso seleccionado del APU");
    removeLine->setEnabled(m_canEdit);
    lineActions->addStretch(); lineActions->addWidget(removeLine); layout->addLayout(lineActions);
    connect(removeLine, &QPushButton::clicked, this, &MainWindow::removeApuLine);
    auto *controls = new QHBoxLayout;
    m_markup = new QDoubleSpinBox; m_markup->setRange(0,999); m_markup->setDecimals(2); m_markup->setValue(20); m_markup->setSuffix(" %");
    auto *budgetQty = new QDoubleSpinBox; budgetQty->setObjectName("budgetQty"); budgetQty->setRange(0.0001,99999999); budgetQty->setDecimals(4); budgetQty->setValue(1);
    auto *calc = new QPushButton("Calcular"); auto *save = new QPushButton("Guardar presupuesto"); auto *exportBtn = new QPushButton("Exportar CSV");
    setAccentButton(calc); setAccentButton(save); setAccentButton(exportBtn);
    controls->addWidget(new QLabel("Cantidad presupuestada")); controls->addWidget(budgetQty); controls->addWidget(new QLabel("Utilidad")); controls->addWidget(m_markup); controls->addWidget(calc); controls->addWidget(save); controls->addWidget(exportBtn);
    layout->addLayout(controls);
    auto *totals = new QHBoxLayout; m_subtotalLabel=new QLabel("Costo unitario neto: $0.00"); m_totalLabel=new QLabel("Total presupuesto: $0.00");
    m_subtotalLabel->setStyleSheet(QString("font-weight:600;color:%1;").arg(kColorBlack));
    m_totalLabel->setStyleSheet(QString("font-size:17px;font-weight:700;color:%1;").arg(kColorBlack)); totals->addWidget(m_subtotalLabel); totals->addStretch(); totals->addWidget(m_totalLabel); layout->addLayout(totals);
    connect(calc,&QPushButton::clicked,this,&MainWindow::calculateBudget);
    connect(save,&QPushButton::clicked,this,&MainWindow::saveBudget);
    connect(exportBtn,&QPushButton::clicked,this,&MainWindow::exportBudget);
    auto *savedLabel = new QLabel("Presupuestos guardados");
    savedLabel->setStyleSheet(QString("font-size:15px;font-weight:700;color:%1;padding-top:8px;").arg(kColorBlack));
    layout->addWidget(savedLabel);
    m_savedBudgets = makeTable(page,{"ID","Presupuesto","APU","Fecha","Total"});
    m_savedBudgets->setMaximumHeight(180);
    m_savedBudgets->setColumnHidden(0,true);
    layout->addWidget(m_savedBudgets);
    auto *deleteBudget = new QPushButton("Eliminar presupuesto y su APU");
    setAccentButton(deleteBudget);
    deleteBudget->setEnabled(m_canEdit);
    layout->addWidget(deleteBudget,0,Qt::AlignRight);
    connect(deleteBudget,&QPushButton::clicked,this,&MainWindow::deleteSavedBudget);
    m_tabs->addTab(page,"APU y presupuesto · HU-05/06");
}

void MainWindow::reloadResources() {
    QSqlQuery q;
    if (!q.prepare("SELECT id_recurso,codigo,descripcion,unidad,tipo,precio_vigente FROM recurso WHERE activo AND (:f='' OR codigo ILIKE :pattern OR descripcion ILIKE :pattern) ORDER BY descripcion")) {
        showDbError(this,q);
        return;
    }
    const QString f=m_resourceFilter->text().trimmed(); q.bindValue(":f",f); q.bindValue(":pattern","%"+f+"%");
    if (!q.exec()) { showDbError(this,q); return; }
    m_resources->setRowCount(0);
    while(q.next()) { int r=m_resources->rowCount(); m_resources->insertRow(r); for(int c=0;c<6;++c) m_resources->setItem(r,c,new QTableWidgetItem(c==5?QString::number(q.value(c).toDouble(),'f',2):q.value(c).toString())); }
    const int previous=m_priceResource->currentData().toInt();
    {
        const QSignalBlocker blockPriceSelection(m_priceResource);
        m_priceResource->clear(); m_lineResource->clear();
        QSqlQuery list;
        if (!list.exec("SELECT id_recurso,concat(codigo,' · ',descripcion,' ($',precio_vigente,')') FROM recurso WHERE activo ORDER BY descripcion")) {
            showDbError(this,list,"No se pudo cargar la lista de recursos para precios y APU.");
            return;
        }
        while(list.next()) { m_priceResource->addItem(list.value(1).toString(),list.value(0)); m_lineResource->addItem(list.value(1).toString(),list.value(0)); }
        list.finish();
        for(int i=0;i<m_priceResource->count();++i) if(m_priceResource->itemData(i).toInt()==previous) m_priceResource->setCurrentIndex(i);
    }
    const int selected = m_priceResource->currentData().toInt();
    if (selected > 0) {
        QSqlQuery currentPrice;
        currentPrice.prepare("SELECT precio_vigente FROM recurso WHERE id_recurso=:id AND activo");
        currentPrice.bindValue(":id",selected);
        if (!currentPrice.exec()) showDbError(this,currentPrice,"No se pudo consultar el precio del recurso seleccionado.");
        else if (currentPrice.next()) m_newPrice->setValue(currentPrice.value(0).toDouble());
        currentPrice.finish();
    }
}

QString MainWindow::generateResourceCode() {
    const QString prefix = codePrefix(m_type->currentText());
    QSqlQuery q;
    q.prepare("SELECT COALESCE(MAX(substring(codigo from '[0-9]+$')::integer),0)+1 "
              "FROM recurso WHERE codigo LIKE :pattern");
    q.bindValue(":pattern", prefix + "-%");
    if (!q.exec()) { showDbError(this,q,"No se pudo generar el siguiente código de recurso."); return {}; }
    if (!q.next()) { QMessageBox::critical(this,"No se pudo generar el código","PostgreSQL no devolvió un código. Intenta nuevamente."); return {}; }
    return prefix + "-" + QString("%1").arg(q.value(0).toInt(), 4, 10, QChar('0'));
}

void MainWindow::refreshCodePreview() {
    const QString code = generateResourceCode();
    m_code->setText(code.isEmpty() ? "(se asignará al guardar)" : code);
}

void MainWindow::logout() {
    hide();
    emit logoutRequested();
}

int MainWindow::selectedResourceId() const {
    const int row=m_resources->currentRow(); if(row<0 || !m_resources->item(row,0)) return 0;
    return m_resources->item(row,0)->text().toInt();
}

void MainWindow::addResource() {
    if(m_description->text().trimmed().isEmpty()||m_unit->text().trimmed().isEmpty()) { QMessageBox::warning(this,"Datos requeridos","Completa descripción y unidad."); return; }
    auto db = QSqlDatabase::database();
    if (!db.transaction()) { QMessageBox::critical(this,"No se pudo iniciar el registro","No se pudo iniciar una transacción con PostgreSQL. Comprueba la conexión y los permisos de escritura.\n\nDetalle: " + db.lastError().text()); return; }
    QSqlQuery lock;
    if (!lock.exec("LOCK TABLE recurso IN EXCLUSIVE MODE")) { db.rollback(); showDbError(this,lock); return; }
    const QString code = generateResourceCode();
    if (code.isEmpty()) { db.rollback(); return; } // generateResourceCode ya mostró la causa.
    const double price=m_initialPrice->value();
    QSqlQuery q; q.prepare("INSERT INTO recurso(codigo,descripcion,unidad,tipo,precio_vigente) VALUES(:c,:d,:u,:t,:p) RETURNING id_recurso");
    q.bindValue(":c",code); q.bindValue(":d",m_description->text().trimmed()); q.bindValue(":u",m_unit->text().trimmed()); q.bindValue(":t",m_type->currentText()); q.bindValue(":p",price);
    if(!q.exec()||!q.next()) { QSqlDatabase::database().rollback(); showDbError(this,q); return; }
    const int id=q.value(0).toInt();
    q.finish();
    QSqlQuery h; h.prepare("INSERT INTO historial_precio(id_recurso,id_usuario,precio_anterior,precio_nuevo,origen) VALUES(:r,:u,NULL,:p,'MANUAL')"); h.bindValue(":r",id); h.bindValue(":u",m_userId); h.bindValue(":p",price);
    if(!h.exec()) { QSqlDatabase::database().rollback(); showDbError(this,h); return; }
    if (!db.commit()) { const QString detail=db.lastError().text(); db.rollback(); QMessageBox::critical(this,"No se pudo guardar el recurso","PostgreSQL no confirmó el registro. No se guardaron los cambios.\n\nDetalle: " + detail); return; }
    m_description->clear();m_unit->clear();m_initialPrice->setValue(0);reloadResources();refreshCodePreview();
    QMessageBox::information(this,"Recurso registrado",QString("Recurso %1 guardado correctamente.").arg(code));
}

void MainWindow::deleteResource() {
    const int id = selectedResourceId();
    if (id == 0) {
        QMessageBox::information(this,"Selecciona un recurso","Selecciona una fila del catálogo antes de retirarla.");
        return;
    }
    const int row = m_resources->currentRow();
    const QString code = m_resources->item(row,1)->text();
    const auto answer = QMessageBox::question(this,"Retirar recurso",
        QString("¿Retirar %1 del catálogo activo? Su historial y los presupuestos existentes se conservarán.").arg(code),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) return;

    QSqlQuery q;
    q.prepare("UPDATE recurso SET activo=FALSE WHERE id_recurso=:id AND activo=TRUE");
    q.bindValue(":id",id);
    if (!q.exec()) { showDbError(this,q); return; }
    if (q.numRowsAffected() != 1) {
        QMessageBox::information(this,"Recurso ya retirado","El recurso ya no está en el catálogo activo.");
        reloadResources();
        return;
    }
    reloadResources();
    QMessageBox::information(this,"Recurso retirado","Se ocultó del catálogo activo. Su historial y referencias se conservaron.");
}

void MainWindow::changePrice() {
    const int id=m_priceResource->currentData().toInt();
    if(!id){QMessageBox::information(this,"Selecciona un recurso","No hay recursos activos disponibles para actualizar. Registra o reactiva un recurso primero.");return;}
    auto db = QSqlDatabase::database();
    if (!db.transaction()) { QMessageBox::critical(this,"No se pudo actualizar el precio","No se pudo iniciar una transacción con PostgreSQL. Comprueba la conexión y los permisos.\n\nDetalle: " + db.lastError().text()); return; }
    QSqlQuery q; q.prepare("SELECT precio_vigente FROM recurso WHERE id_recurso=:id FOR UPDATE"); q.bindValue(":id",id);
    if(!q.exec()) {db.rollback();showDbError(this,q,"No se pudo bloquear y consultar el precio actual.");return;}
    if(!q.next()) {db.rollback();QMessageBox::information(this,"Recurso no disponible","El recurso ya no existe o fue retirado del catálogo. Actualiza la lista e inténtalo otra vez.");return;}
    const double old=q.value(0).toDouble(), now=m_newPrice->value();
    q.finish();
    QSqlQuery update; update.prepare("UPDATE recurso SET precio_vigente=:p WHERE id_recurso=:id"); update.bindValue(":p",now);update.bindValue(":id",id);
    if(!update.exec()) {QSqlDatabase::database().rollback();showDbError(this,update);return;}
    update.finish();
    QSqlQuery hist; hist.prepare("INSERT INTO historial_precio(id_recurso,id_usuario,precio_anterior,precio_nuevo,origen) VALUES(:r,:u,:o,:n,'MANUAL')"); hist.bindValue(":r",id);hist.bindValue(":u",m_userId);hist.bindValue(":o",old);hist.bindValue(":n",now);
    if(!hist.exec()) {QSqlDatabase::database().rollback();showDbError(this,hist);return;}
    hist.finish();
    if (!db.commit()) { const QString detail=db.lastError().text(); db.rollback(); QMessageBox::critical(this,"No se pudo actualizar el precio","PostgreSQL no confirmó el cambio. El precio y su historial no se guardaron.\n\nDetalle: " + detail); return; }
    reloadResources();QMessageBox::information(this,"Precio actualizado","El cambio quedó registrado en el historial.");
}

void MainWindow::showPriceHistory() {
    const int id=m_priceResource->currentData().toInt(); if(!id)return;
    QSqlQuery q; q.prepare("SELECT h.fecha,u.nombre,COALESCE(h.precio_anterior::text,'—'),h.precio_nuevo,h.origen FROM historial_precio h JOIN usuario u USING(id_usuario) WHERE id_recurso=:id ORDER BY fecha DESC");q.bindValue(":id",id);
    if(!q.exec()){showDbError(this,q);return;}
    auto *dialog=new QDialog(this);dialog->setWindowTitle("Historial de precios");dialog->resize(760,400);dialog->setStyleSheet(kAppStyleSheet);auto *lay=new QVBoxLayout(dialog);
    auto *table=makeTable(dialog,{"Fecha","Usuario","Precio anterior","Precio nuevo","Origen"});
    while(q.next()){int r=table->rowCount();table->insertRow(r);for(int c=0;c<5;++c)table->setItem(r,c,new QTableWidgetItem(q.value(c).toString()));}
    lay->addWidget(table);dialog->exec();dialog->deleteLater();
}

void MainWindow::addApuLine() {
    const int id=m_lineResource->currentData().toInt();
    if(!id){QMessageBox::information(this,"No hay recursos disponibles","Registra un recurso activo antes de agregar líneas al APU.");return;}
    QSqlQuery q;q.prepare("SELECT descripcion,unidad,precio_vigente FROM recurso WHERE id_recurso=:id");q.bindValue(":id",id);
    if(!q.exec()){showDbError(this,q,"No se pudo consultar el recurso para agregarlo al APU.");return;}
    if(!q.next()){QMessageBox::warning(this,"Recurso no disponible","El recurso ya no existe o fue retirado del catálogo. Actualiza la lista e inténtalo otra vez.");return;}
    for(int r=0;r<m_apuLines->rowCount();++r)if(m_apuLines->item(r,5)&&m_apuLines->item(r,5)->text().toInt()==id){QMessageBox::warning(this,"Recurso duplicado","Cada recurso se agrega una sola vez al APU.");return;}
    const int row=m_apuLines->rowCount();m_apuLines->insertRow(row);
    m_apuLines->setItem(row,0,new QTableWidgetItem(q.value(0).toString()));m_apuLines->setItem(row,1,new QTableWidgetItem(q.value(1).toString()));
    m_apuLines->setItem(row,2,new QTableWidgetItem(QString::number(m_lineQty->value(),'f',4)));
    m_apuLines->setItem(row,3,new QTableWidgetItem(QString::number(q.value(2).toDouble(),'f',2)));
    m_apuLines->setItem(row,4,new QTableWidgetItem(QString::number(q.value(2).toDouble()*m_lineQty->value(),'f',2)));
    m_apuLines->setItem(row,5,new QTableWidgetItem(QString::number(id)));m_apuLines->setColumnHidden(5,true);calculateBudget();
}

void MainWindow::removeApuLine() {
    const int row = m_apuLines->currentRow();
    if (row < 0) {
        QMessageBox::information(this,"Selecciona una línea","Selecciona un recurso del APU antes de quitarlo.");
        return;
    }
    m_apuLines->removeRow(row);
    calculateBudget();
}

void MainWindow::calculateBudget() {
    m_subtotal=0;for(int r=0;r<m_apuLines->rowCount();++r)m_subtotal+=m_apuLines->item(r,4)->text().toDouble();
    auto *qty=findChild<QDoubleSpinBox*>("budgetQty");const double quantity=qty?qty->value():1;
    const double total=m_subtotal*quantity*(1.0+m_markup->value()/100.0);
    m_subtotalLabel->setText(QString("Costo unitario neto: $%1").arg(m_subtotal,0,'f',2));
    m_totalLabel->setText(QString("Total presupuesto: $%1").arg(total,0,'f',2));
}

void MainWindow::saveBudget() {
    calculateBudget();
    if(m_budgetName->text().trimmed().isEmpty()||m_apuName->text().trimmed().isEmpty()||m_apuLines->rowCount()==0){QMessageBox::warning(this,"Datos requeridos","Indica el presupuesto, la actividad APU y agrega al menos un recurso.");return;}
    auto *qty=findChild<QDoubleSpinBox*>("budgetQty");const double quantity=qty->value();const double total=m_subtotal*quantity*(1+m_markup->value()/100.0);
    auto db = QSqlDatabase::database();
    if (!db.transaction()) { QMessageBox::critical(this,"No se pudo guardar el presupuesto","No se pudo iniciar una transacción con PostgreSQL. Comprueba la conexión y los permisos de escritura.\n\nDetalle: " + db.lastError().text()); return; }
    QSqlQuery apu;apu.prepare("INSERT INTO apu(nombre,unidad) VALUES(:n,:u) RETURNING id_apu");apu.bindValue(":n",m_apuName->text().trimmed());apu.bindValue(":u",m_apuUnit->text().trimmed());
    if(!apu.exec()){db.rollback();showDbError(this,apu,"No se pudo crear la actividad APU.");return;}
    if(!apu.next()){db.rollback();QMessageBox::critical(this,"No se creó el APU","PostgreSQL no devolvió el identificador del APU. Intenta guardar nuevamente.");return;}
    const int apuId=apu.value(0).toInt();
    apu.finish();
    for(int r=0;r<m_apuLines->rowCount();++r){QSqlQuery line;line.prepare("INSERT INTO apu_detalle(id_apu,id_recurso,cantidad) VALUES(:a,:r,:q)");line.bindValue(":a",apuId);line.bindValue(":r",m_apuLines->item(r,5)->text().toInt());line.bindValue(":q",m_apuLines->item(r,2)->text().toDouble());if(!line.exec()){QSqlDatabase::database().rollback();showDbError(this,line);return;}line.finish();}
    QSqlQuery budget;budget.prepare("INSERT INTO presupuesto(nombre,utilidad_pct,total,id_usuario) VALUES(:n,:u,:t,:id) RETURNING id_presupuesto");budget.bindValue(":n",m_budgetName->text().trimmed());budget.bindValue(":u",m_markup->value());budget.bindValue(":t",total);budget.bindValue(":id",m_userId);
    if(!budget.exec()){db.rollback();showDbError(this,budget,"No se pudo crear el presupuesto.");return;}
    if(!budget.next()){db.rollback();QMessageBox::critical(this,"No se creó el presupuesto","PostgreSQL no devolvió el identificador del presupuesto. Intenta guardar nuevamente.");return;}
    const int budgetId=budget.value(0).toInt();budget.finish();
    QSqlQuery part;part.prepare("INSERT INTO partida(id_presupuesto,id_apu,descripcion,unidad,cantidad,costo_unitario_neto,costo_presupuestado) VALUES(:p,:a,:d,:u,:q,:c,:t)");part.bindValue(":p",budgetId);part.bindValue(":a",apuId);part.bindValue(":d",m_apuName->text().trimmed());part.bindValue(":u",m_apuUnit->text().trimmed());part.bindValue(":q",quantity);part.bindValue(":c",m_subtotal);part.bindValue(":t",total);
    if(!part.exec()){QSqlDatabase::database().rollback();showDbError(this,part);return;}part.finish();
    if (!db.commit()) { const QString detail=db.lastError().text(); db.rollback(); QMessageBox::critical(this,"No se pudo guardar el presupuesto","PostgreSQL no confirmó el presupuesto. No se guardaron los cambios.\n\nDetalle: " + detail); return; }
    reloadSavedBudgets();
    QMessageBox::information(this,"Presupuesto guardado",QString("Presupuesto #%1 guardado correctamente.").arg(budgetId));
}

void MainWindow::reloadSavedBudgets() {
    if (!m_savedBudgets) return;
    QSqlQuery q;
    if (!q.exec("SELECT b.id_presupuesto,b.nombre,COALESCE(a.nombre,'-'),b.fecha::text,b.total "
                "FROM presupuesto b LEFT JOIN LATERAL ("
                " SELECT ap.nombre FROM partida p JOIN apu ap USING(id_apu) "
                " WHERE p.id_presupuesto=b.id_presupuesto ORDER BY p.id_partida LIMIT 1"
                ") a ON TRUE ORDER BY b.fecha DESC,b.id_presupuesto DESC")) {
        showDbError(this,q);
        return;
    }
    m_savedBudgets->setRowCount(0);
    while (q.next()) {
        const int row = m_savedBudgets->rowCount();
        m_savedBudgets->insertRow(row);
        for (int column=0; column<5; ++column) {
            QString value = q.value(column).toString();
            if (column == 4) value = QString("$ %1").arg(q.value(column).toDouble(),0,'f',2);
            m_savedBudgets->setItem(row,column,new QTableWidgetItem(value));
        }
    }
}

void MainWindow::deleteSavedBudget() {
    const int row = m_savedBudgets->currentRow();
    if (row < 0 || !m_savedBudgets->item(row,0)) {
        QMessageBox::information(this,"Selecciona un presupuesto","Selecciona un presupuesto guardado para eliminarlo.");
        return;
    }
    const int budgetId = m_savedBudgets->item(row,0)->text().toInt();
    const QString budgetName = m_savedBudgets->item(row,1)->text();
    const auto answer = QMessageBox::question(this,"Eliminar presupuesto y APU",
        QString("¿Eliminar '%1' y sus partidas? Si el APU no está compartido con otro presupuesto, también se eliminará. Esta acción no se puede deshacer.").arg(budgetName),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) return;

    auto db = QSqlDatabase::database();
    if (!db.transaction()) { QMessageBox::critical(this,"No se pudo eliminar el presupuesto","No se pudo iniciar una transacción con PostgreSQL. Comprueba la conexión y los permisos de escritura.\n\nDetalle: " + db.lastError().text()); return; }
    QList<int> apuIds;
    QSqlQuery findApus;
    findApus.prepare("SELECT DISTINCT id_apu FROM partida WHERE id_presupuesto=:budget AND id_apu IS NOT NULL");
    findApus.bindValue(":budget",budgetId);
    if (!findApus.exec()) { db.rollback(); showDbError(this,findApus); return; }
    while (findApus.next()) apuIds.append(findApus.value(0).toInt());
    findApus.finish();

    QSqlQuery removeBudget;
    removeBudget.prepare("DELETE FROM presupuesto WHERE id_presupuesto=:budget");
    removeBudget.bindValue(":budget",budgetId);
    if (!removeBudget.exec()) { db.rollback(); showDbError(this,removeBudget); return; }
    removeBudget.finish();
    for (const int apuId : apuIds) {
        QSqlQuery removeApu;
        removeApu.prepare("DELETE FROM apu WHERE id_apu=:apu AND NOT EXISTS (SELECT 1 FROM partida WHERE id_apu=:ref)");
        removeApu.bindValue(":apu",apuId);
        removeApu.bindValue(":ref",apuId);
        if (!removeApu.exec()) { db.rollback(); showDbError(this,removeApu); return; }
    }
    if (!db.commit()) { const QString detail=db.lastError().text(); db.rollback(); QMessageBox::critical(this,"No se pudo eliminar el presupuesto","PostgreSQL no confirmó la eliminación; se cancelaron los cambios.\n\nDetalle: " + detail); return; }
    reloadSavedBudgets();
    QMessageBox::information(this,"Eliminación completada","Se eliminó el presupuesto. Los recursos e historiales asociados se conservaron.");
}

void MainWindow::exportBudget() {
    calculateBudget();if(m_apuLines->rowCount()==0){QMessageBox::information(this,"Sin datos","Agrega recursos al APU antes de exportar.");return;}
    const QString path=QFileDialog::getSaveFileName(this,"Exportar presupuesto",m_budgetName->text().trimmed()+".csv","CSV (*.csv)");if(path.isEmpty())return;
    QFile file(path);if(!file.open(QIODevice::WriteOnly|QIODevice::Text)){QMessageBox::critical(this,"No se pudo exportar el archivo","No se pudo crear o escribir el CSV. Comprueba que la carpeta exista, que tengas permisos y que el archivo no esté abierto en otro programa.\n\nDetalle: " + file.errorString());return;}
    QTextStream out(&file);out.setEncoding(QStringConverter::Utf8);out<<"Presupuesto;"<<m_budgetName->text()<<"\nActividad;"<<m_apuName->text()<<"\nRecurso;Unidad;Cantidad APU;Precio vigente;Costo línea\n";
    for(int r=0;r<m_apuLines->rowCount();++r)out<<m_apuLines->item(r,0)->text()<<';'<<m_apuLines->item(r,1)->text()<<';'<<m_apuLines->item(r,2)->text()<<';'<<m_apuLines->item(r,3)->text()<<';'<<m_apuLines->item(r,4)->text()<<"\n";
    auto *qty=findChild<QDoubleSpinBox*>("budgetQty");out<<"Costo unitario neto;;; ;"<<QString::number(m_subtotal,'f',2)<<"\nCantidad presupuestada;"<<qty->value()<<"\nUtilidad (%);"<<m_markup->value()<<"\nTotal presupuesto;;; ;"<<m_totalLabel->text().section('$',-1)<<"\n";
    file.close();
    if (file.error() != QFileDevice::NoError) { QMessageBox::critical(this,"Exportación incompleta","Ocurrió un error mientras se escribía el CSV. Comprueba el espacio disponible y los permisos de la carpeta.\n\nDetalle: " + file.errorString()); return; }
    QMessageBox::information(this,"Exportación lista","Presupuesto exportado como CSV compatible con Excel.");
}
