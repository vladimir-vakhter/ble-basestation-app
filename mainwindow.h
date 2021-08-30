#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define DEBUG                                                                       // debug mode

#ifdef DEBUG
#include <QDebug>                                                                   // output stream for debugging information
#endif

#include <QMainWindow>                                                              // this is a standard GUI thing

#include <QBluetoothAddress>                                                        // assigns an address to the Bluetooth device
#include <QBluetoothServiceDiscoveryAgent>                                          // enables us to query for Bluetooth services
#include <QBluetoothServiceInfo>                                                    // enables access to the attributes of a Bluetooth service
#include <QBluetoothLocalDevice>                                                    // enables access to the local Bluetooth device
#include <QBluetoothUuid>                                                           // generates a UUID for each Bluetooth service
#include <QBluetoothSocket>                                                         // enables connection to a Bluetooth device running a bluetooth server
#include <QLowEnergyController>                                                     // provides access to Bluetooth Low Energy Devices
#include <QLowEnergyService>                                                        // represents an individual service on a Bluetooth Low Energy Device
#include <QLowEnergyCharacteristic>                                                 // stores information about a Bluetooth Low Energy service characteristic
#include <QLowEnergyCharacteristicData>                                             // is used to set up GATT service data

#include <QTimer>                                                                   // repetitive and single-shot timers
#include <QSerialPort>                                                              // access to serial ports
#include <QSerialPortInfo>                                                          // information about existing serial ports.
#include <QScrollBar>                                                               // widget provides a vertical or horizontal scroll bar
#include <QDir>                                                                     // provides access to directory structures and their contents
#include <QTreeWidgetItem>                                                          // provides an item for use with a tree view that uses a predefined tree model

QT_BEGIN_NAMESPACE                                                                  // this is a standard GUI thing
namespace Ui { class MainWindow; }                                                  // this is a standard GUI thing
QT_END_NAMESPACE                                                                    // this is a standard GUI thing

class MainWindow : public QMainWindow                                               // this is a standard GUI thing
{                                                                                   // this is a standard GUI thing
    Q_OBJECT                                                                        // this is a standard GUI thing

public:                                                                             // this is a standard GUI thing
    MainWindow(QWidget *parent = nullptr);                                          // this is a standard GUI thing
    ~MainWindow();                                                                  // this is a standard GUI thing

public slots:
    void addDevice(QBluetoothDeviceInfo info);
    void deviceUpdated(const QBluetoothDeviceInfo info,
                       QBluetoothDeviceInfo::Fields fields);

    void deviceDiscoveryFinished();
    inline void deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error) { qDebug() << "Device discovery error: " << error; }
    inline void deviceDiscoveryCanceled() { qDebug() << "Device discovery canceled!"; }

    void addService(QBluetoothServiceInfo info);
    inline void addServiceError(QBluetoothDeviceDiscoveryAgent::Error error) { qDebug() << error; }
    void addServiceDone();

    void socketRead();
    inline void socketConnected() { qDebug() << "socket connect"; }
    inline void socketDisconnected() { qDebug() << "socket disconnect"; }
    inline void socketError() { qDebug() << "socket error"; };

    void bleServiceDiscovered(const QBluetoothUuid &gatt);
    inline void bleServiceDiscoveryFinished() { qDebug() << "bleServiceDiscoveryFinished() has been called"; }

    void bleServiceCharacteristicNotify(const QLowEnergyCharacteristic &info,
                                  const QByteArray &value);
    void bleServiceCharacteristicRead(const QLowEnergyCharacteristic &info,
                                      const QByteArray &value);

private slots:
    void on_servicesPushButton_clicked();
    void on_connectPushButton_clicked();
    void on_bleConnectPushButton_clicked();
    void on_bleDisconnectPushButton_clicked();

    void on_bleCharacteristicReadPushButton_clicked();
    void on_bleCharacteristicWritePushButton_clicked();

    void on_scanPeriodicallyCheckBox_clicked(bool checked);
    void on_bleServicesTreeWidget_currentItemChanged(QTreeWidgetItem *current,
                                                     QTreeWidgetItem *previous);
    void on_listenNotifyPushButton_clicked();

private:                                                                            // this is a standard GUI thing
    Ui::MainWindow *ui;                                                             // this is a standard GUI thing

    QBluetoothDeviceDiscoveryAgent  *mDiscoveryAgent        = nullptr;              // discovers the Bluetooth devices nearby
    QBluetoothServiceDiscoveryAgent *mServiceDiscoveryAgent = nullptr;
    QBluetoothSocket                *mSocket                = nullptr;

    QLowEnergyController            *mBLEControl            = nullptr;
    QLowEnergyService               *mBLEService            = nullptr;

    void format_output(const int& format_selector_index, const QByteArray& value, QString& output);

};                                                                                  // this is a standard GUI thing
#endif // MAINWINDOW_H                                                              // this is a standard GUI thing
