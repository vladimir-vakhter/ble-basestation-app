/*
 * This file contains the logics that controls the view of mainwindow.ui
 * mainvindow.ui contains the user interface's widgets
 */

#include "mainwindow.h"                            // this is a standard GUI thing
#include "ui_mainwindow.h"                         // this is a standard GUI thing

#include <QLowEnergyAdvertisingData>

typedef enum {
    CH_STRING = 0,
    CH_INT8,
    CH_INT16,
    CH_INT32,
    CH_UINT8,
    CH_UINT16,
    CH_UINT32,
    CH_HEX
} ch_type;

typedef enum {
    DEVICE_ADDRESS = 0,
    DEVICE_NAME,
    DEVICE_CORE_CONF,
    DEVICE_RSSI
} device_table_column_index;

MainWindow::MainWindow(QWidget *parent)             // this is a standard GUI thing
    : QMainWindow(parent)                           // this is a standard GUI thing
    , ui(new Ui::MainWindow)                        // this is a standard GUI thing
{
    ui->setupUi(this);                              // this is a standard GUI thing
    this->setWindowTitle("BLE Central");

   //QBluetoothLocalDevice localDevice;
   //QBluetoothAddress adapterAddress = localDevice.address();

    mDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent();

    connect(mDiscoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)), this, SLOT(addDevice(QBluetoothDeviceInfo)));
    connect(mDiscoveryAgent, SIGNAL(deviceUpdated(QBluetoothDeviceInfo, QBluetoothDeviceInfo::Fields)),
            this, SLOT(deviceUpdated(QBluetoothDeviceInfo, QBluetoothDeviceInfo::Fields)));
    connect(mDiscoveryAgent, SIGNAL(finished()), this, SLOT(deviceDiscoveryFinished()));
    connect(mDiscoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)),
            this, SLOT(deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error)));
    connect(mDiscoveryAgent, SIGNAL(canceled()), this, SLOT(deviceDiscoveryCanceled()));

    // a table displaying the infromation about Bluetooth devices
    ui->devicesTableWidget->setColumnCount(4);
    QStringList headerLabels;
    headerLabels << "Address" << "Name" << "Configuration" << "RSSI, dB";
    ui->devicesTableWidget->setHorizontalHeaderLabels(headerLabels);

    ui->devicesTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->devicesTableWidget->resizeColumnsToContents();

    // start Bluetooth device discovery
    mDiscoveryAgent->start();

    ui->scanningIndicatorLabel->setStyleSheet("QLabel { background-color : white; color : red; }");
    ui->scanningIndicatorLabel->setText("Scanning...");

    ui->bleUartOutputPlainTextEdit->setReadOnly(true);
    ui->bleUartOutputPlainTextEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    mNRF52SerialPort = new QSerialPort(this);

    connect(mNRF52SerialPort, &QSerialPort::readyRead,
            this, &MainWindow::on_NRF52SerialReadyRead);

    connect(ui->bleUartInputLineEdit, &QLineEdit::returnPressed,
            this, &MainWindow::on_bleUartSendPushButton_clicked);
}

MainWindow::~MainWindow()                           // this is a standard GUI thing
{
    delete(mDiscoveryAgent);
    delete ui;                                      // this is a standard GUI thing
}

void MainWindow::addDevice(QBluetoothDeviceInfo info)
{
//    #ifdef DEBUG
//        qDebug() << "addDevice() has been called";
//    #endif

    // retrive the information about a Bluetooth device
    QString bluetooth_device_name = info.name();
    QString bluetooth_device_addr = info.address().toString();

    QString bluetooth_device_configuration = "";
    QBluetoothDeviceInfo::CoreConfigurations cconf = info.coreConfigurations();
    if (cconf.testFlag(QBluetoothDeviceInfo::LowEnergyCoreConfiguration))            { bluetooth_device_configuration.append("BLE"); }
    if (cconf.testFlag(QBluetoothDeviceInfo::UnknownCoreConfiguration))              { bluetooth_device_configuration.append("Unknown"); }
    if (cconf.testFlag(QBluetoothDeviceInfo::BaseRateCoreConfiguration))             { bluetooth_device_configuration.append("Standard"); }
    if (cconf.testFlag(QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration)) { bluetooth_device_configuration.append("BLE & Standard"); }

    QString rssi = QString::number(info.rssi(), 10);

    // add the information about Bluetooth device to a table
    QTableWidgetItem *device_addr_item = new QTableWidgetItem();
    device_addr_item->setData(Qt::UserRole,QVariant::fromValue(info));
    device_addr_item->setText(bluetooth_device_addr);

    QTableWidgetItem *device_name_item = new QTableWidgetItem();
    device_name_item->setText(bluetooth_device_name);

    QTableWidgetItem *device_configuration_item = new QTableWidgetItem();
    device_configuration_item->setText(bluetooth_device_configuration);

    QTableWidgetItem *device_rssi_item = new QTableWidgetItem();
    device_rssi_item->setText(rssi);

    int  device_record_row = 0;
    bool device_record_found  = false;

    // update an existing row
    for (; device_record_row < (ui->devicesTableWidget->rowCount()); device_record_row++) {
        if (ui->devicesTableWidget->item(device_record_row, 0)->text() == bluetooth_device_addr) {
            device_record_found = true;
            break;
        }
    }

    // add a new row
    if (!device_record_found) {
        int row = ui->devicesTableWidget->rowCount();
        ui->devicesTableWidget->setRowCount(row + 1);
    }

    ui->devicesTableWidget->setItem(device_record_row, DEVICE_ADDRESS,   device_addr_item);
    ui->devicesTableWidget->setItem(device_record_row, DEVICE_NAME,      device_name_item);
    ui->devicesTableWidget->setItem(device_record_row, DEVICE_CORE_CONF, device_configuration_item);
    ui->devicesTableWidget->setItem(device_record_row, DEVICE_RSSI,      device_rssi_item);
}

void MainWindow::deviceUpdated(const QBluetoothDeviceInfo info, QBluetoothDeviceInfo::Fields fields)
{
//    #ifdef DEBUG
//        qDebug() << "deviceUpdated() has been called";
//    #endif

    QString bluetooth_device_addr = info.address().toString();

    int device_record_row = 0;
    bool device_record_found = false;

    for (; device_record_row < ui->devicesTableWidget->rowCount(); device_record_row ++) {
        if (ui->devicesTableWidget->item(device_record_row, 0)->text() == bluetooth_device_addr) {
            device_record_found = true;
            break;
        }
    }

    int mask = 0x0001;
    if (device_record_found && (fields & mask)) {
        QTableWidgetItem *it = new QTableWidgetItem();
        it->setText(QString::number(info.rssi(), 10));
        ui->devicesTableWidget->setItem(device_record_row, DEVICE_RSSI, it);
    }
}

void MainWindow::deviceDiscoveryFinished()
{
    #ifdef DEBUG
        qDebug() << "deviceDiscoveryFinished() has been called";
    #endif

    ui->scanningIndicatorLabel->setStyleSheet("QLabel { background-color : white; color : green; }");
    ui->scanningIndicatorLabel->setText("Done!");

    int msecTimeout = 5000;

    // call a slot after a given timeout
    if (ui->scanPeriodicallyCheckBox->isChecked()) {
        QTimer::singleShot(msecTimeout, this,
                           [this]()
                           {
                                ui->scanningIndicatorLabel->setStyleSheet("QLabel { background-color : white; color : red; }");
                                ui->scanningIndicatorLabel->setText("Scanning...");
                                mDiscoveryAgent->start();
                            }
        );
    }
}

void MainWindow::deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    qDebug() << "Device discovery error: " << error;
}

void MainWindow::deviceDiscoveryCanceled()
{
    qDebug() << "Device discovery canceled!";
}

void MainWindow::addService(QBluetoothServiceInfo info)
{
    #ifdef DEBUG
        qDebug() << "addService() has been called";
    #endif

    QString str = QString("%1 %2").arg(info.serviceName()).arg(info.serviceUuid().toString());

    QListWidgetItem *it = new QListWidgetItem();

    it->setData(Qt::UserRole, QVariant::fromValue(info));
    it->setText(str); //info.serviceName());

    ui->servicesListWidget->addItem(it);

}

void MainWindow::addServiceError(QBluetoothDeviceDiscoveryAgent::Error error)
{
   qDebug() << error;
}

void MainWindow::addServiceDone()
{
    ui->servicesPushButton->setEnabled(true);
}

void MainWindow::socketRead()
{
    char buffer[1024];
    qint64 len;

    qDebug() << "socket read";

    while (mSocket->bytesAvailable()) {
        len = mSocket->read(buffer, 1024);
        qDebug() << len << " : " << QString(buffer);
    }
}

void MainWindow::socketConnected()
{
    qDebug() << "socket connect";
}

void MainWindow::socketDisconnected()
{
    qDebug() << "socket disconnect";
}

void MainWindow::socketError()
{
    qDebug() << "socket error";
}

void MainWindow::bleServiceDiscovered(const QBluetoothUuid &gatt)
{
    #ifdef DEBUG
        qDebug() << "bleServiceDiscovered() has been called";
    #endif

    //QListWidgetItem *it = new QListWidgetItem();
    QTreeWidgetItem *it = new QTreeWidgetItem();

    QLowEnergyService *bleService = mBLEControl->createServiceObject(gatt);

    if (bleService) {
        connect(bleService, &QLowEnergyService::stateChanged,
                this, [this, bleService, it] (QLowEnergyService::ServiceState state) {
            qDebug() << "BLE Service state changed:" << state;
            switch(state) {
            case QLowEnergyService::InvalidService:  {
                QTreeWidgetItem *child = new QTreeWidgetItem();

                child->setText(0,"Invalid Service");
                it->addChild(child);
            } break;
            case QLowEnergyService::RemoteService:
                break;
            case QLowEnergyService::RemoteServiceDiscovering:
                break;
            case QLowEnergyService::RemoteServiceDiscovered:
                for (auto c : bleService->characteristics()) {
                    QTreeWidgetItem *child = new QTreeWidgetItem();

                    child->setData(0,Qt::UserRole, QVariant::fromValue(c));
                    if (!c.name().isEmpty()) {
                        child->setText(0,c.name());
                    } else {
                        child->setText(0,c.uuid().toString());
                    }
                    it->addChild(child);

                }
                break;
            case QLowEnergyService::LocalService: {
                QTreeWidgetItem *child = new QTreeWidgetItem();

                child->setText(0,"Local Service");
                it->addChild(child);
            } break;
            }
        });
        connect(bleService, &QLowEnergyService::characteristicChanged,
                this, &MainWindow::bleServiceCharacteristic);
        connect(bleService, &QLowEnergyService::characteristicRead,
                this, &MainWindow::bleServiceCharacteristicRead);
        //mBLEService->characteristicChanged()
        bleService->discoverDetails();
    } else {
        qDebug() << "Error connecting to BLE Service";
    }

    it->setData(0,Qt::UserRole, QVariant::fromValue(gatt));
    it->setData(1,Qt::UserRole, QVariant::fromValue(bleService));
    it->setText(0,gatt.toString());
    //it->setText(str);


    ui->bleServicesTreeWidget->addTopLevelItem(it);
    //ui->bleServicesListWidget->addItem(it);

}

void MainWindow::bleServiceDiscoveryFinished()
{
    #ifdef DEBUG
        qDebug() << "bleServiceDiscoveryFinished() has been called";
    #endif
}

void MainWindow::bleServiceCharacteristic(const QLowEnergyCharacteristic &info, const QByteArray &value)
{
    #ifdef DEBUG
        qDebug() << "bleServiceCharacteristic() has been called";
    #endif

    QString str = info.name();
    //str.append(": ");
    str.append(QString(value));

    // If value comes from uart service, redirect to uart text output
    if (info.uuid() == QBluetoothUuid(QString("{6e400003-b5a3-f393-e0a9-e50e24dcca9e}"))) {
        // UART RX
        QTextCursor text_cursor = QTextCursor(ui->bleUartOutputPlainTextEdit->document());
        text_cursor.movePosition(QTextCursor::End);
        text_cursor.insertText(str);

        QScrollBar *sb = ui->bleUartOutputPlainTextEdit->verticalScrollBar();
        sb->setValue(sb->maximum());

        //ui->bleUartOutputPlainTextEdit->moveCursor (QTextCursor::End);
        //ui->bleUartOutputPlainTextEdit->appendPlainText(str);

    } else {
        ui->outputPlainTextEdit->appendPlainText(str);
    }
}

void MainWindow::bleServiceCharacteristicRead(const QLowEnergyCharacteristic &info, const QByteArray &value)
{
    #ifdef DEBUG
        qDebug() << "bleServiceCharacteristicRead() has been called";
    #endif

    (void) info;

    int index = ui->bleCharacteristicReadTypeComboBox->currentIndex();
    QString str;

    switch(index) {
    case CH_STRING:  //String
        str = QString(value);
        break;
    case CH_INT8:  // int8;
        str = QString("%1").number(value.at(0));
        break;
    case CH_INT16:
        break;
    case CH_INT32:
        break;
    case CH_UINT8:
        break;
    case CH_UINT16:
        break;
    case CH_UINT32:
        break;
    case CH_HEX:
        for (auto c: value)  {
            str.append(QString("0x%1").number(c,16));
        }
    }

    ui->outputPlainTextEdit->appendPlainText(str);
}

void MainWindow::on_servicesPushButton_clicked()
{
    #ifdef DEBUG
        qDebug() << "on_servicesPushButton_clicked() has been called";
    #endif

    ui->servicesPushButton->setEnabled(false);
    ui->servicesListWidget->clear();

    //QListWidgetItem *it = ui->devicesListWidget->currentItem();
    QTableWidgetItem *it = ui->devicesTableWidget->currentItem();

    QBluetoothDeviceInfo info = it->data(Qt::UserRole).value<QBluetoothDeviceInfo>();

    qDebug() << info.name();
    qDebug() << info.address();


    mServiceDiscoveryAgent = new QBluetoothServiceDiscoveryAgent();
    mServiceDiscoveryAgent->setRemoteAddress(info.address());

    if (mServiceDiscoveryAgent->error()) {
        qDebug() << "wrong device address";
    }

    connect(mServiceDiscoveryAgent, SIGNAL(serviceDiscovered(QBluetoothServiceInfo)),
            this, SLOT(addService(QBluetoothServiceInfo)));
    connect(mServiceDiscoveryAgent, SIGNAL(finished()),
            this, SLOT(addServiceDone()));
//    connect(mServiceDiscoveryAgent, QOverload<QBluetoothServiceDiscoveryAgent::Error>::of(&QBluetoothServiceDiscoveryAgent::error),
//        [=](QBluetoothServiceDiscoveryAgent::Error error){ qDebug() << error; ui->servicesPushButton->setEnabled(true); });

    mServiceDiscoveryAgent->start();
}

void MainWindow::on_connectPushButton_clicked()
{
    #ifdef DEBUG
        qDebug() << "on_connectPushButton_clicked() has been called";
    #endif

    QBluetoothServiceInfo remoteService;

    QListWidgetItem *it = ui->servicesListWidget->currentItem();

    if (!it) return;

    QBluetoothServiceInfo info = it->data(Qt::UserRole).value<QBluetoothServiceInfo>();

    qDebug () << info.serviceName();
    qDebug () << info.serviceUuid();
    qDebug () << info.serviceDescription();

    //info.setServiceUuid(QBluetoothUuid((quint16)0x2A19));
    qDebug () << info.serviceUuid();

    if (mSocket) {
        qDebug() << "socket exists, deleting";
        disconnect(mSocket, SIGNAL(readyRead()), this, SLOT(socketRead()));
        disconnect(mSocket, SIGNAL(connected()), this, SLOT(socketConnected()));
        disconnect(mSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
        //disconnect(mSocket, SIGNAL(error ()), this, SLOT(socketError()));
        mSocket->disconnectFromService();
        delete(mSocket);
    }

    // Connect to service
    mSocket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);

    qDebug() << "Create socket";
    mSocket->connectToService(info);
    qDebug() << "ConnectToService done";

    connect(mSocket, SIGNAL(readyRead()), this, SLOT(socketRead()));
    connect(mSocket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(mSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    //connect(mSocket, SIGNAL(error()), this, SLOT(socketError()));
}


void MainWindow::on_bleConnectPushButton_clicked()
{
    #ifdef DEBUG
        qDebug() << "on_bleConnectPushButton_clicked() has been called";
    #endif

    //QBluetoothDeviceInfo dev = ui->devicesListWidget->currentItem()->data(Qt::UserRole).value<QBluetoothDeviceInfo>();
    int row = ui->devicesTableWidget->currentRow();
    QTableWidgetItem *it = ui->devicesTableWidget->item(row, 0);

    if (!it) {
        qDebug() << "No device selected!";
        return;
    }

    QBluetoothDeviceInfo dev = it->data(Qt::UserRole).value<QBluetoothDeviceInfo>();
    mBLEControl = QLowEnergyController::createCentral(dev, this);

    connect(mBLEControl, &QLowEnergyController::serviceDiscovered,
            this, &MainWindow::bleServiceDiscovered);

    connect(mBLEControl, &QLowEnergyController::discoveryFinished,
            this, &MainWindow::bleServiceDiscoveryFinished);

    connect(mBLEControl, &QLowEnergyController::connected, this, [this]() {
        (void)this; qDebug() << "connected to BLE device!";
        mBLEControl->discoverServices();
    });

    connect(mBLEControl, &QLowEnergyController::disconnected, this, [this]() {
        (void) this; qDebug() << "Disconnected from BLE device!";
    });

    mBLEControl->connectToDevice();

}

void MainWindow::on_bleDisconnectPushButton_clicked()
{
    #ifdef DEBUG
        qDebug() << "on_bleDisconnectPushButton_clicked() has been called";
    #endif

    mBLEControl->disconnectFromDevice();

    //ui->bleServicesListWidget->clear();
    ui->bleServicesTreeWidget->clear();
    //ui->bleCharacteristicsListWidget->clear();
}

void MainWindow::on_bleCharacteristicReadPushButton_clicked()
{
    #ifdef DEBUG
        qDebug() << "on_bleCharacteristicReadPushButton_clicked() has been called";
    #endif

    QTreeWidgetItem *it = ui->bleServicesTreeWidget->currentItem();

    if (!it) return;

    if (it->data(0, Qt::UserRole).canConvert<QLowEnergyCharacteristic>()) {
        QLowEnergyCharacteristic ch = it->data(0,Qt::UserRole).value<QLowEnergyCharacteristic>();
        qDebug() << "Should be ok to convert to characteristic";

        // the top-level parent holds the service
        // TODO: it is not absolute guaranteed that top-level parent holds the service.
        //       Needs a more robust way of finding the enclosing service.
        QTreeWidgetItem *p = it->parent();

        while (p->parent() != nullptr) {
            p = p->parent();
        }

        if (p->data(1, Qt::UserRole).canConvert<QLowEnergyService*>()) {
            QLowEnergyService *s = p->data(1, Qt::UserRole).value<QLowEnergyService*>();
            qDebug() << "Should be ok to convert to a service..";
            s->readCharacteristic(ch);
        }
    }
}

void MainWindow::on_bleCharacteristicWritePushButton_clicked()
{
    #ifdef DEBUG
        qDebug() << "on_bleCharacteristicWritePushButton_clicked() has been called";
        qDebug() << "not implemented";
    #endif
}

void MainWindow::on_scanPeriodicallyCheckBox_clicked(bool checked)
{
    #ifdef DEBUG
        qDebug() << "on_scanPeriodicallyCheckBox_clicked() has been called";
    #endif

    int msecTimeout = 5000;

    if (!mDiscoveryAgent->isActive() && checked) {
        QTimer::singleShot(msecTimeout, this,
                           [this]()
                           {
                                ui->scanningIndicatorLabel->setStyleSheet("QLabel { background-color : white; color : red; }");
                                ui->scanningIndicatorLabel->setText("Scanning...");
                                mDiscoveryAgent->start();
                            }
        );
    }
}

void MainWindow::on_NRF52SerialReadyRead()
{
    #ifdef DEBUG
        qDebug() << "on_NRF52SerialReadyRead() has been called";
    #endif

    QByteArray data = mNRF52SerialPort->readAll();
    QString str = QString(data);
    //ui->consoleOutputTextEdit->setc
}

void MainWindow::on_bleServicesTreeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    #ifdef DEBUG
        qDebug() << "on_bleServicesTreeWidget_currentItemChanged() has been called";
    #endif

    (void) previous;
    (void) current;
}

void MainWindow::on_listenNotifyPushButton_clicked()
{
    #ifdef DEBUG
        qDebug() << "on_listenNotifyPushButton_clicked() has been called";
    #endif

    QTreeWidgetItem *it = ui->bleServicesTreeWidget->currentItem();

    if (!it) return;

    if (!(it->data(0, Qt::UserRole).canConvert<QLowEnergyCharacteristic>()))  return;

    QLowEnergyCharacteristic ch = it->data(0,Qt::UserRole).value<QLowEnergyCharacteristic>();
    qDebug() << "Should be ok to convert to characteristic";

    QTreeWidgetItem *p = it;

    while (p->parent() != nullptr) {
        p = p->parent();
    }

    if (!(p->data(1, Qt::UserRole).canConvert<QLowEnergyService*>())) return;

    QLowEnergyService *s = p->data(1, Qt::UserRole).value<QLowEnergyService*>();
    qDebug() << "Should be ok to convert to a service..";

    const QLowEnergyCharacteristic txChar = s->characteristic(QBluetoothUuid(ch.uuid()));

    if (!txChar.isValid()){
        qDebug() << "BLE Tx characteristic not found";
        return;
    }

     QLowEnergyDescriptor desc = txChar.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);

    if (desc.isValid()) {
        // enable notification
        s->writeDescriptor(desc, QByteArray::fromHex("0100"));
    } else {
        qDebug() << "Tx Characteristc descriptor is invalid!";
    }

}

void MainWindow::on_bleUartConnectPushButton_clicked()
{
    #ifdef DEBUG
        qDebug() << "on_bleUartConnectPushButton_clicked() has been called";
    #endif

    QTreeWidgetItem *it = ui->bleServicesTreeWidget->currentItem();

    if (!it) return;

    // the root service here should be of ble uart type.
    QTreeWidgetItem *p = it;

    while (p->parent() != nullptr) {
        p = p->parent();
    }

    if (!(p->data(1, Qt::UserRole).canConvert<QLowEnergyService*>())) {
        qDebug() << "root of tree is not a service!";
        return;
    }
    QLowEnergyService *s = p->data(1, Qt::UserRole).value<QLowEnergyService*>();

    mBLEUartService = s;

    QList<QLowEnergyCharacteristic> chs =  s->characteristics();

    if (chs.size() != 2) {
        qDebug() << "Ble uart service should have exactly 2 characteristics";
        return;
    }

    QLowEnergyCharacteristic rx;
    if (chs[0].uuid() == QBluetoothUuid(QString("{6e400003-b5a3-f393-e0a9-e50e24dcca9e}"))) {
        // 0 is RX
        rx = s->characteristic(QBluetoothUuid(chs[0].uuid()));
    } else if (chs[1].uuid() == QBluetoothUuid(QString("{6e400003-b5a3-f393-e0a9-e50e24dcca9e}"))) {
        // 1 is RX
        rx = s->characteristic(QBluetoothUuid(chs[1].uuid()));
    } else  {
        qDebug() << chs[0].uuid().toString();
        qDebug() << chs[1].uuid().toString();
        qDebug() << "Probably not a proper uart!";
        return;
    }

    QLowEnergyDescriptor desc = rx.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);

    if (desc.isValid()) {
        // enable notification
        s->writeDescriptor(desc, QByteArray::fromHex("0100"));
    } else {
        qDebug() << "Rx Characteristc descriptor is invalid!";
    }
}

void MainWindow::on_bleUartSendPushButton_clicked()
{
    #ifdef DEBUG
        qDebug() << "on_bleUartSendPushButton_clicked() has been called";
    #endif

    if (!mBLEUartService) {
        qDebug() << "No BLE uart connected";
    } else {

        QList<QLowEnergyCharacteristic> chs = mBLEUartService->characteristics();
        QLowEnergyCharacteristic tx;

        if (chs[0].uuid() == QBluetoothUuid(QString("{6e400002-b5a3-f393-e0a9-e50e24dcca9e}"))) {
            // 0 is RX
            tx = mBLEUartService->characteristic(QBluetoothUuid(chs[0].uuid()));
        } else if (chs[1].uuid() == QBluetoothUuid(QString("{6e400002-b5a3-f393-e0a9-e50e24dcca9e}"))) {
            // 1 is RX
            tx = mBLEUartService->characteristic(QBluetoothUuid(chs[1].uuid()));
        } else  {
            qDebug() << chs[0].uuid().toString();
            qDebug() << chs[1].uuid().toString();
            qDebug() << "Probably not a proper uart!";
            return;
        }

        if (tx.isValid()) {
            QByteArray ba = ui->bleUartInputLineEdit->text().append("\n").toLocal8Bit();
            while (ba.size() > 0) {
                qDebug() << "ba = " << QString(ba);
                mBLEUartService->writeCharacteristic(tx, ba.mid(0,20), QLowEnergyService::WriteWithoutResponse);
                ba = ba.mid(20,-1);
            }
        }
    }
    ui->bleUartInputLineEdit->clear();
}
