#pragma once

#include <QMainWindow>
#include <memory>

class QLineEdit;
class QPushButton;
class QLabel;
class QCheckBox;
class QSpinBox;
class QListWidget;

#include "OpcUaClient.h" 

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    void setupUi();
    void setStatus(const QString& s);

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onBrowseClicked();
    void onListSelectionChanged();
    void onWriteClicked();
    void onAutoRefreshToggled(bool checked);

private:
    std::unique_ptr<OpcUaClient> m_client;

    QLineEdit* m_url;
    QPushButton* m_connect;
    QPushButton* m_disconnect;

    QPushButton* m_browse;
    QCheckBox* m_auto;
    QSpinBox* m_interval;

    QListWidget* m_list;
    QLabel* m_selected;

    QLineEdit* m_currentValue;
    QLabel* m_type;

    QLineEdit* m_newValue;
    QPushButton* m_write;

    QLabel* m_status;
};
