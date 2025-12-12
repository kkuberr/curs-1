#include "mainwindow.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QTimer>
#include <QGroupBox>

#include "opcua_client.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    m_client = std::make_unique<OpcUaClient>();
    setupUi();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi()
{
    auto central = new QWidget(this);
    auto mainLay = new QVBoxLayout(central);

    auto topLay = new QHBoxLayout();
    m_url = new QLineEdit("opc.tcp://localhost:4840");
    m_connect = new QPushButton("Подключиться");
    m_disconnect = new QPushButton("Отключиться");
    m_disconnect->setEnabled(false);
    topLay->addWidget(new QLabel("URL сервера:"));
    topLay->addWidget(m_url);
    topLay->addWidget(m_connect);
    topLay->addWidget(m_disconnect);

    auto browseLay = new QHBoxLayout();
    m_browse = new QPushButton("Обойти Objects");
    m_auto = new QCheckBox("Автообновление");
    m_auto->setChecked(true);
    m_interval = new QSpinBox();
    m_interval->setRange(1, 3600);
    m_interval->setValue(2);
    browseLay->addWidget(m_browse);
    browseLay->addWidget(m_auto);
    browseLay->addWidget(new QLabel("Обновление (с):"));
    browseLay->addWidget(m_interval);

    m_list = new QListWidget();
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);

    auto infoLay = new QHBoxLayout();
    m_selected = new QLabel("-");
    infoLay->addWidget(new QLabel("Выбран:"));
    infoLay->addWidget(m_selected);

    auto valueLay = new QHBoxLayout();
    m_currentValue = new QLineEdit();
    m_currentValue->setReadOnly(true);
    m_type = new QLabel("-");
    valueLay->addWidget(new QLabel("Текущее значение:"));
    valueLay->addWidget(m_currentValue);
    valueLay->addWidget(new QLabel("Тип:"));
    valueLay->addWidget(m_type);

    auto writeLay = new QHBoxLayout();
    m_newValue = new QLineEdit();
    m_write = new QPushButton("Записать");
    m_write->setEnabled(false);
    writeLay->addWidget(new QLabel("Новое значение:"));
    writeLay->addWidget(m_newValue);
    writeLay->addWidget(m_write);

    m_status = new QLabel("Готово");

    mainLay->addLayout(topLay);
    mainLay->addLayout(browseLay);
    mainLay->addWidget(m_list);
    mainLay->addLayout(infoLay);
    mainLay->addLayout(valueLay);
    mainLay->addLayout(writeLay);
    mainLay->addWidget(m_status);

    setCentralWidget(central);
    setWindowTitle("OPC UA Клиент (черновик)");

    // connections
    connect(m_connect, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(m_disconnect, &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);
    connect(m_browse, &QPushButton::clicked, this, &MainWindow::onBrowseClicked);
    connect(m_list, &QListWidget::itemSelectionChanged, this, &MainWindow::onListSelectionChanged);
    connect(m_write, &QPushButton::clicked, this, &MainWindow::onWriteClicked);
    connect(m_auto, &QCheckBox::toggled, this, &MainWindow::onAutoRefreshToggled);

    // timer for auto-refresh
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        if (m_client && m_client->isConnected() && m_auto->isChecked()) {
            // refresh displayed values
            // in skeleton mode this will invoke browse again to update values
            onBrowseClicked();
        }
    });
    timer->start(m_interval->value() * 1000);
}

void MainWindow::setStatus(const QString& s)
{
    m_status->setText(s);
}

void MainWindow::onConnectClicked()
{
    const QString url = m_url->text();
    setStatus("Подключение...");
    bool ok = m_client->connect(url.toStdString());
    if (ok) {
        setStatus("Подключено к " + url);
        m_connect->setEnabled(false);
        m_disconnect->setEnabled(true);
    } else {
        setStatus("Ошибка подключения");
    }
}

void MainWindow::onDisconnectClicked()
{
    m_client->disconnect();
    setStatus("Отключено");
    m_connect->setEnabled(true);
    m_disconnect->setEnabled(false);
}

void MainWindow::onBrowseClicked()
{
    if (!m_client->isConnected()) {
        setStatus("Не подключено");
        return;
    }
    setStatus("Обход...");
    auto items = m_client->browse_objects();
    m_list->clear();
    for (const auto& it : items) {
        // display: "<parent> / <name> | <nodeid> = <value>"
        m_list->addItem(QString::fromStdString(it.display));
    }
    setStatus(QString("Найдено %1 узлов").arg(m_list->count()));
    m_write->setEnabled(m_list->count() > 0);
}

void MainWindow::onListSelectionChanged()
{
    auto sel = m_list->selectedItems();
    if (sel.isEmpty()) return;
    QString txt = sel.first()->text();
    // in skeleton mode we store the node id in the client-side vector
    auto node = m_client->find_node_by_display(txt.toStdString());
    if (!node.empty()) {
        auto val = m_client->read_value(node);
        m_selected->setText(QString::fromStdString(node));
        m_currentValue->setText(QString::fromStdString(val.value));
        m_type->setText(QString::fromStdString(val.type));
    }
}

void MainWindow::onWriteClicked()
{
    auto sel = m_list->selectedItems();
    if (sel.isEmpty()) {
        setStatus("Переменная не выбрана");
        return;
    }
    // Получаем указатель на выделенный элемент (это критично для обновления)
    QListWidgetItem* item = sel.first(); 
    
    QString txt = item->text();
    auto node = m_client->find_node_by_display(txt.toStdString());
    if (node.empty()) {
        setStatus("Узел не найден");
        return;
    }
    const QString newv = m_newValue->text();
    bool ok = m_client->write_value(node, newv.toStdString());
    
    if (ok) {
        
        // 1. Находим позицию знака "=" в текущей строке, чтобы отделить Node ID от значения.
        int eq_pos = txt.lastIndexOf('=');
        
        if (eq_pos != -1) {
            // 2. Сформировать новую строку: старая часть до '=' + " " + новое значение
            QString new_display = txt.left(eq_pos + 1) + " " + newv;
            
            // 3. Обновить текст элемента в списке!
            item->setText(new_display);
            
            // 4. Обновить поля "Текущее значение" и "Тип"
            //    (поскольку текст элемента обновился, onListSelectionChanged прочитает новое значение)
            onListSelectionChanged(); 
        }
        
        setStatus("Запись успешна");
    } else {
        setStatus("Ошибка записи");
    }
}

void MainWindow::onAutoRefreshToggled(bool checked)
{
    Q_UNUSED(checked);
    // timer lambda checks m_auto->isChecked, nothing else needed here
}
