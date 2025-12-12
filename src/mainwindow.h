#pragma once

#include <QMainWindow>
#include <memory>

class QListWidget;
class QLineEdit;
class QLabel;
class QPushButton;
class QCheckBox;
class QSpinBox;

class OpcUaClient;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onBrowseClicked();
    void onListSelectionChanged();
    void onWriteClicked();
    void onAutoRefreshToggled(bool checked);

private:
    void setupUi();
    void setStatus(const QString& s);

    QListWidget* m_list = nullptr;
    QLineEdit* m_url = nullptr;
    QPushButton* m_connect = nullptr;
    QPushButton* m_disconnect = nullptr;
    QPushButton* m_browse = nullptr;
    QLabel* m_selected = nullptr;
    QLineEdit* m_currentValue = nullptr;
    QLabel* m_type = nullptr;
    QLineEdit* m_newValue = nullptr;
    QPushButton* m_write = nullptr;
    QCheckBox* m_auto = nullptr;
    QSpinBox* m_interval = nullptr;

    QLabel* m_status = nullptr;

    std::unique_ptr<OpcUaClient> m_client;
};
