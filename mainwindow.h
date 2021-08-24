#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>                              // this is a standard GUI thing

#include <qbluetoothaddress.h>                      //
#include <qbluetoothservicediscoveryagent.h>        //
#include <qbluetoothserviceinfo.h>                  //
#include <qbluetoothlocaldevice.h>                  //
#include <qbluetoothuuid.h>                         //
#include <qbluetoothsocket.h>                       //
#include <qlowenergycontroller.h>                   //
#include <qlowenergyservice.h>                      //
#include <qlowenergycharacteristic.h>               //
#include <qlowenergycharacteristicdata.h>           //

#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QScrollBar>
#include <QDir>
#include <QFileDialog>
#include <QTreeWidgetItem>

#include <QDebug>

QT_BEGIN_NAMESPACE                                  // this is a standard GUI thing
namespace Ui { class MainWindow; }                  // this is a standard GUI thing
QT_END_NAMESPACE                                    // this is a standard GUI thing

class MainWindow : public QMainWindow               // this is a standard GUI thing
{                                                   // this is a standard GUI thing
    Q_OBJECT                                        // this is a standard GUI thing

public:                                             // this is a standard GUI thing
    MainWindow(QWidget *parent = nullptr);          // this is a standard GUI thing
    ~MainWindow();                                  // this is a standard GUI thing



public slots:
    void addDevice(QBluetoothDeviceInfo info);
    void deviceUpdated(const QBluetoothDeviceInfo info,
                       QBluetoothDeviceInfo::Fields fields);
    void deviceDiscoveryFinished();
    void deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error);
    void deviceDiscoveryCanceled();
    void addService(QBluetoothServiceInfo info);
    void addServiceError(QBluetoothDeviceDiscoveryAgent::Error);
    void addServiceDone();
    void socketRead();
    void socketConnected();
    void socketDisconnected();
    void socketError();
    void bleServiceDiscovered(const QBluetoothUuid &gatt);
    void bleServiceDiscoveryFinished();
    void bleServiceCharacteristic(const QLowEnergyCharacteristic &info,
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
    void on_ttyConnectPushButton_clicked();
    void on_NRF52SerialReadyRead();
    void on_consoleSendPushButton_clicked();
    void on_scriptDirBrowsePushButton_clicked();
    void on_bleServicesTreeWidget_currentItemChanged(QTreeWidgetItem *current,
                                                     QTreeWidgetItem *previous);

    void on_listenNotifyPushButton_clicked();

    void on_bleUartConnectPushButton_clicked();

    void on_bleUartSendPushButton_clicked();

private:                                                                        // this is a standard GUI thing
    Ui::MainWindow *ui;                                                         // this is a standard GUI thing

    QBluetoothDeviceDiscoveryAgent *mDiscoveryAgent = nullptr;
    QBluetoothServiceDiscoveryAgent *mServiceDiscoveryAgent = nullptr;
    QBluetoothSocket *mSocket = nullptr;

    QLowEnergyController *mBLEControl = nullptr;
    QLowEnergyService    *mBLEService = nullptr;

    QLowEnergyService    *mBLEUartService = nullptr;

    QSerialPort *mNRF52SerialPort;


};                                                                              // this is a standard GUI thing
#endif // MAINWINDOW_H                                                          // this is a standard GUI thing
