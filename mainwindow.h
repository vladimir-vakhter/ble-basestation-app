#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>                                            // output stream for debugging information
#include <QMainWindow>                                       // GUI
#include <QTimer>                                            // repetitive and single-shot timers (used in BLE discovery / scanning)
#include <QTreeWidgetItem>                                   // provides an item for use with a tree view that uses a predefined tree model
#include <QFileDialog>                                       // provides a dialog that allow users to select files or directories
#include <QMessageBox>                                       // provides a modal dialog to interact with the user
#include <QMutex>                                            // provides access serialization between threads

#include <QBluetoothAddress>                                 // assigns an address to the Bluetooth device
#include <QBluetoothServiceDiscoveryAgent>                   // enables us to query for Bluetooth services
//#include <QBluetoothServiceInfo>                             // enables access to the attributes of a Bluetooth service
#include <QBluetoothLocalDevice>                             // enables access to the local Bluetooth device
#include <QBluetoothUuid>                                    // generates a UUID for each Bluetooth service
#include <QBluetoothSocket>                                  // enables connection to a Bluetooth device running a bluetooth server
#include <QLowEnergyController>                              // provides access to Bluetooth Low Energy Devices
#include <QLowEnergyService>                                 // represents an individual service on a Bluetooth Low Energy Device
#include <QLowEnergyCharacteristic>                          // stores information about a Bluetooth Low Energy service characteristic
#include <QLowEnergyCharacteristicData>                      // is used to set up GATT service data

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void addDevice(QBluetoothDeviceInfo info);
    void deviceUpdated(const QBluetoothDeviceInfo info, QBluetoothDeviceInfo::Fields fields);

    void deviceDiscoveryFinished();
    inline void deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error) { qDebug() << "Device discovery error: " << error; }
    inline void deviceDiscoveryCanceled() { qDebug() << "Device discovery canceled!"; }

    void socketRead();
    inline void socketConnected() { qDebug() << "socket connect"; }
    inline void socketDisconnected() { qDebug() << "socket disconnect"; }
    inline void socketError() { qDebug() << "socket error"; };

    void bleServiceDiscovered(const QBluetoothUuid &gatt);
    inline void bleServiceDiscoveryFinished() { qDebug() << "bleServiceDiscoveryFinished() has been called"; }

    void bleServiceCharacteristicNotify(const QLowEnergyCharacteristic &info, const QByteArray &value);
    void bleServiceCharacteristicRead(const QLowEnergyCharacteristic &info, const QByteArray &value);

private slots:
    void on_bleConnectPushButton_clicked();
    void on_bleDisconnectPushButton_clicked();

    void on_bleCharacteristicReadPushButton_clicked();
    void on_bleCharacteristicWritePushButton_clicked();

    void on_scanPeriodicallyCheckBox_clicked(bool checked);
    void on_bleServicesTreeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void on_listenNotifyPushButton_clicked();

    void on_clearOutputPushButton_clicked();

    void on_browsePushButton_clicked();

private:
    Ui::MainWindow *ui;

    QBluetoothDeviceDiscoveryAgent  *mDiscoveryAgent        = nullptr;
    QBluetoothServiceDiscoveryAgent *mServiceDiscoveryAgent = nullptr;
    QBluetoothSocket                *mSocket                = nullptr;

    QLowEnergyController            *mBLEControl            = nullptr;
    QLowEnergyService               *mBLEService            = nullptr;

    QString                         csvFilePath             = "";          // path to the file that stores the values of READ and NOTIFY characteristics
    QMutex                          mutex;                                 // "guards" shared resources in case READ and NOTIFY appear simultaneously

    void bleServiceCharacteristicReadNotify(const QString &header, const QByteArray &value);
    void format_output(const int& format_selector_index, const QByteArray& rawBytesValue, QString& formattedOutput, QString& format);
    void append_to_csv(const QString& header, const QString& data);
};
#endif // MAINWINDOW_H
