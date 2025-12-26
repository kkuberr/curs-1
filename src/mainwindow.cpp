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

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    m_client = std::make_unique<OpcUaClient>();
    setupUi();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi()
{
    auto* central = new QWidget(this);
    auto* mainLay = new QVBoxLayout(central);

    auto* topLay = new QHBoxLayout();
    m_url = new QLineEdit("opc.tcp://localhost:4840");
    m_connect = new QPushButton("Подключиться");
    m_disconnect = new QPushButton("Отключиться");
    m_disconnect->setEnabled(false);

    topLay->addWidget(new QLabel("URL сервера:"));
    topLay->addWidget(m_url);
    topLay->addWidget(m_connect);
    topLay->addWidget(m_disconnect);

    auto* browseLay = new QHBoxLayout();
    m_browse = new QPushButton("Обойти Objects");
    m_auto = new QCheckBox("Автообновление");
    m_auto->setChecked(false); 
    m_interval = new QSpinBox();
    m_interval->setRange(1, 3600);
    m_interval->setValue(2);

    browseLay->addWidget(m_browse);
    browseLay->addWidget(m_auto);
    browseLay->addWidget(new QLabel("Обновление (с):"));
    browseLay->addWidget(m_interval);

    m_list = new QListWidget();
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);

    auto* infoLay = new QHBoxLayout();
    m_selected = new QLabel("-");
    infoLay->addWidget(new QLabel("NodeId:"));
    infoLay->addWidget(m_selected);

    auto* valueLay = new QHBoxLayout();
    m_currentValue = new QLineEdit();
    m_currentValue->setReadOnly(true);
    m_type = new QLabel("-");

    valueLay->addWidget(new QLabel("Текущее значение:"));
    valueLay->addWidget(m_currentValue);
    valueLay->addWidget(new QLabel("Тип:"));
    valueLay->addWidget(m_type);

    auto* writeLay = new QHBoxLayout();
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
    setWindowTitle("OPC UA Client (QT)");

    connect(m_connect, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(m_disconnect, &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);
    connect(m_browse, &QPushButton::clicked, this, &MainWindow::onBrowseClicked);
    connect(m_list, &QListWidget::itemSelectionChanged,
            this, &MainWindow::onListSelectionChanged);
    connect(m_write, &QPushButton::clicked, this, &MainWindow::onWriteClicked);
    connect(m_auto, &QCheckBox::toggled, this, &MainWindow::onAutoRefreshToggled);

    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        if (m_client->isConnected() && m_auto->isChecked()) {
            if (!m_list->selectedItems().isEmpty()) {
                onListSelectionChanged();
            }
        }
    });

    connect(m_interval, QOverload<int>::of(&QSpinBox::valueChanged), [timer](int val) {
        timer->setInterval(val * 1000);
    });

    timer->start(m_interval->value() * 1000);
}

void MainWindow::setStatus(const QString& s)
{
    m_status->setText(s);
}

void MainWindow::onConnectClicked()
{
    setStatus("Подключение...");
    if (m_client->connect(m_url->text().toStdString())) {
        setStatus("Подключено");
        m_connect->setEnabled(false);
        m_disconnect->setEnabled(true);
    } else {
        setStatus("Ошибка подключения");
    }
}

void MainWindow::onDisconnectClicked()
{
    m_client->disconnect();
    m_list->clear();
    m_selected->setText("-");
    m_currentValue->clear();
    m_type->setText("-");
    setStatus("Отключено");
    m_connect->setEnabled(true);
    m_disconnect->setEnabled(false);
}

void MainWindow::onBrowseClicked()
{
    if (!m_client->isConnected()) {
        setStatus("Нет подключения");
        return;
    }

    m_list->clear();
    auto items = m_client->browse_objects();

    for (const auto& it : items) {
        auto* item = new QListWidgetItem(QString::fromStdString(it.displayPath));
        item->setData(Qt::UserRole, QString::fromStdString(it.nodeId));
        m_list->addItem(item);
    }

    setStatus(QString("Найдено узлов: %1").arg(m_list->count()));
    m_write->setEnabled(m_list->count() > 0);
}

void MainWindow::onListSelectionChanged()
{
    auto sel = m_list->selectedItems();
    if (sel.isEmpty()) return;

    auto* item = sel.first();
    QString nodeId = item->data(Qt::UserRole).toString();

    auto val = m_client->read_value(nodeId.toStdString());

    m_selected->setText(nodeId);
    m_currentValue->setText(QString::fromStdString(val.value));
    m_type->setText(QString::fromStdString(val.type));
}

void MainWindow::onWriteClicked()
{
    auto sel = m_list->selectedItems();
    if (sel.isEmpty()) {
        setStatus("Узел не выбран");
        return;
    }

    auto* item = sel.first();
    QString nodeId = item->data(Qt::UserRole).toString();

    if (m_client->write_value(nodeId.toStdString(),
                              m_newValue->text().toStdString())) {
        onListSelectionChanged(); 
        setStatus("Запись успешна");
    } else {
        setStatus("Ошибка записи");
    }
}

void MainWindow::onAutoRefreshToggled(bool checked)
{
    if (checked) {
        setStatus("Автообновление включено");
    } else {
        setStatus("Автообновление выключено");
    }
}