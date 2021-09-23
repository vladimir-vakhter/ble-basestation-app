/*
 * This file contains the logics that controls the view of mainwindow.ui
 * mainvindow.ui contains the user interface's widgets
 */

#include "ui_mainwindow.h"
#include "mainwindow.h"

typedef enum {
    CH_HEX = 0, CH_BIN, CH_UNICODE
} output_format_type;

typedef enum {
    DEVICE_ADDRESS = 0,
    DEVICE_NAME,
//    DEVICE_CORE_CONF,
    DEVICE_RSSI
} device_table_column_index;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("BLE Client App");

    mDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent();

    // connect signals and slots
    connect(mDiscoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)), this, SLOT(addDevice(QBluetoothDeviceInfo)));
    connect(mDiscoveryAgent, SIGNAL(deviceUpdated(QBluetoothDeviceInfo, QBluetoothDeviceInfo::Fields)),
            this, SLOT(deviceUpdated(QBluetoothDeviceInfo, QBluetoothDeviceInfo::Fields)));
    connect(mDiscoveryAgent, SIGNAL(finished()), this, SLOT(deviceDiscoveryFinished()));
    connect(mDiscoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)),
            this, SLOT(deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error)));
    connect(mDiscoveryAgent, SIGNAL(canceled()), this, SLOT(deviceDiscoveryCanceled()));

    // a table displaying the infromation about Bluetooth devices
//    ui->devicesTableWidget->setColumnCount(4);
    ui->devicesTableWidget->setColumnCount(3);
    QStringList headerLabels;
//    headerLabels << "Address" << "Name" << "Configuration" << "RSSI, dB";
    headerLabels << "Address" << "Name" << "RSSI, dB";
    ui->devicesTableWidget->setHorizontalHeaderLabels(headerLabels);

    ui->devicesTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->devicesTableWidget->resizeColumnsToContents();

    // start Bluetooth device discovery
    mDiscoveryAgent->start();

    // the comment below is left to show how to stylize groupboxes and may be deleted
    //ui->deviceControlGroupBox->setStyleSheet(QStringLiteral("QGroupBox{border:2px solid gray;border-radius:5px;margin-top: 3ex;background-color: red;}"));

    // update the status of the BLE discovery process
    ui->scanningIndicatorLabel->setStyleSheet("QLabel { background-color : white; color : red; }");
    ui->scanningIndicatorLabel->setText("Scanning...");
}

MainWindow::~MainWindow()
{
    delete mDiscoveryAgent;
    delete mServiceDiscoveryAgent;
    delete mSocket;
    delete mBLEControl;
    delete mBLEService;
    delete ui;
}

void MainWindow::addDevice(QBluetoothDeviceInfo info)
{
    // retrive the information about a Bluetooth device
    QString bluetooth_device_name = info.name();
    QString bluetooth_device_addr = info.address().toString();

//    QString bluetooth_device_configuration = "";
    bool isBle = false;
    QBluetoothDeviceInfo::CoreConfigurations cconf = info.coreConfigurations();
    if (cconf.testFlag(QBluetoothDeviceInfo::LowEnergyCoreConfiguration))            { isBle = true;    /*bluetooth_device_configuration.append("BLE");*/ }
    if (cconf.testFlag(QBluetoothDeviceInfo::UnknownCoreConfiguration))              { isBle = false;   /*bluetooth_device_configuration.append("Unknown");*/ }
    if (cconf.testFlag(QBluetoothDeviceInfo::BaseRateCoreConfiguration))             { isBle = false;   /*bluetooth_device_configuration.append("Standard");*/ }
    if (cconf.testFlag(QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration)) { isBle = true;    /*bluetooth_device_configuration.append("BLE & Standard");*/ }

    QString rssi = QString::number(info.rssi(), 10);

    // add the information about Bluetooth device to a table
    QTableWidgetItem *device_addr_item = new QTableWidgetItem();
    device_addr_item->setData(Qt::UserRole,QVariant::fromValue(info));
    device_addr_item->setText(bluetooth_device_addr);

    QTableWidgetItem *device_name_item = new QTableWidgetItem();
    device_name_item->setText(bluetooth_device_name);

//    QTableWidgetItem *device_configuration_item = new QTableWidgetItem();
//    device_configuration_item->setText(bluetooth_device_configuration);

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
    if (!device_record_found && isBle) {
        int row = ui->devicesTableWidget->rowCount();
        ui->devicesTableWidget->setRowCount(row + 1);
    }

    ui->devicesTableWidget->setItem(device_record_row, DEVICE_ADDRESS,   device_addr_item);
    ui->devicesTableWidget->setColumnHidden(DEVICE_ADDRESS, true);

    ui->devicesTableWidget->setItem(device_record_row, DEVICE_NAME,      device_name_item);

//    ui->devicesTableWidget->setItem(device_record_row, DEVICE_CORE_CONF, device_configuration_item);
//    ui->devicesTableWidget->setColumnHidden(DEVICE_CORE_CONF, true);

    ui->devicesTableWidget->setItem(device_record_row, DEVICE_RSSI,      device_rssi_item);
}

void MainWindow::deviceUpdated(const QBluetoothDeviceInfo info, QBluetoothDeviceInfo::Fields fields)
{
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

void MainWindow::socketRead()
{
    char buffer[1024];
    qint64 len;

    while (mSocket->bytesAvailable()) {
        len = mSocket->read(buffer, 1024);
        qDebug() << len << " : " << QString(buffer);
    }
}

void MainWindow::bleServiceDiscovered(const QBluetoothUuid &gatt)
{
    QTreeWidgetItem *it = new QTreeWidgetItem();
    QLowEnergyService *bleService = mBLEControl->createServiceObject(gatt);

    if (bleService) {
        connect(bleService, &QLowEnergyService::stateChanged, this,
                [bleService, it] (QLowEnergyService::ServiceState state)
                {
                    #ifdef DEBUG
                        qDebug() << "BLE Service state changed:" << state;
                    #endif
                    switch(state) {
                        case QLowEnergyService::InvalidService:
                        {
                            QTreeWidgetItem *child = new QTreeWidgetItem();
                            child->setText(0, "Invalid Service");
                            it->addChild(child);
                            break;
                        }
                        case QLowEnergyService::RemoteService:
                            break;
                        case QLowEnergyService::RemoteServiceDiscovering:
                            break;
                        case QLowEnergyService::RemoteServiceDiscovered:
                        {
                            for (const auto &c : bleService->characteristics()) {
                                int column = 0;

                                // the human-readable name of the characteristic
                                QTreeWidgetItem *charChild = new QTreeWidgetItem();
                                QString charName= "Name: {" + (c.name().isEmpty() ?  "Unknown}; " : c.name() + "}; ");
                                QString charUuid = "UUID: " + c.uuid().toString() + "; ";
                                QString charProps = "Props: ";

                                switch (c.properties().toInt()) {
                                    case 0x00:
                                        charProps += "Unknown";
                                        break;
                                    case 0x01:
                                        charProps += "Broadcasting";
                                        break;
                                    case 0x02:
                                        charProps += "Read";
                                        break;
                                    case 0x04:
                                        charProps += "WriteNoResponse";
                                        break;
                                    case 0x08:
                                        charProps += "Write";
                                        break;
                                    case 0x10:
                                        charProps += "Notify";
                                        break;
                                    case 0x20:
                                        charProps += "Indicate";
                                        break;
                                    case 0x40:
                                        charProps += "WriteSigned";
                                        break;
                                    case 0x80:
                                        charProps += "ExtendedProperty";
                                        break;
                                    case 0x06:
                                        charProps += "Read, WriteNoResponse";
                                    break;
                                    default:
                                        charProps += "Define combination!";
                                        break;
                                }

                                QString charDescription = charName + charUuid + charProps;

                                charChild->setData(column, Qt::UserRole, QVariant::fromValue(c));
                                charChild->setText(column, charDescription);
                                it->addChild(charChild);
                            }
                            break;
                        }
                        case QLowEnergyService::LocalService:
                        {
                            QTreeWidgetItem *child = new QTreeWidgetItem();
                            child->setText(0,"Local Service");
                            it->addChild(child);
                            break;
                        }
                    }
                }
        );
        connect(bleService, &QLowEnergyService::characteristicChanged, this, &MainWindow::bleServiceCharacteristicNotify);
        connect(bleService, &QLowEnergyService::characteristicRead, this, &MainWindow::bleServiceCharacteristicRead);
        bleService->discoverDetails();
    } else {
        qDebug() << "Error connecting to BLE Service";
    }

    it->setData(0, Qt::UserRole, QVariant::fromValue(gatt));
    it->setData(1, Qt::UserRole, QVariant::fromValue(bleService));
    it->setText(0, gatt.toString());

    ui->bleServicesTreeWidget->addTopLevelItem(it);
}

void MainWindow::format_output(const int& format_selector_index, const QByteArray& rawBytesValue, QString& formattedOutput, QString& format)
{
    if (format_selector_index == CH_UNICODE) {
        format = "Unicode";
        formattedOutput = QString(rawBytesValue);
    } else {
        int base = 0;
        int numDigits = 0;
        QChar paddingSymbol = '0';

        if (format_selector_index == CH_BIN)  {
            format = "bin";
            base = 2;
            numDigits = 8;
        } else if (format_selector_index == CH_HEX) {
            format = "hex";
            base = 16;
        }

        for (int i = 0; i < rawBytesValue.size(); i++) {
            formattedOutput += QString::number(rawBytesValue.at(i), base).rightJustified(numDigits, paddingSymbol);
        }
    }
}

void MainWindow::bleServiceCharacteristicReadNotify(const QString& header, const QByteArray &value)
{
    // if other threads try to call lock() on this mutex, they will block until this thread calls unlock()
    mutex.lock();

    // format value
    int output_format_selector_index = ui->bleCharacteristicReadTypeComboBox->currentIndex();
    QString output;
    QString format;
    format_output(output_format_selector_index, value, output, format);

    // save if checkbox is checked
    if((header == "Notify" && ui->notifyCheckBox->isChecked()) ||
       (header == "Read" && ui->readCheckBox->isChecked()))
    {
        append_to_csv(header, output);
    }

    // display
    output = header + ": " + output;
    ui->outputPlainTextEdit->appendPlainText(output);

    mutex.unlock();
}

void MainWindow::bleServiceCharacteristicNotify(const QLowEnergyCharacteristic &info, const QByteArray &value)
{
    QString header = "Notify";
    bleServiceCharacteristicReadNotify(header, value);
}

void MainWindow::bleServiceCharacteristicRead(const QLowEnergyCharacteristic& info, const QByteArray& value)
{
    QString header = "Read";
    bleServiceCharacteristicReadNotify(header, value);
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
    mBLEControl->disconnectFromDevice();
    ui->bleServicesTreeWidget->clear();
}

void MainWindow::on_bleCharacteristicReadPushButton_clicked()
{
    // if no items selected, give a hint to the user and return
    QTreeWidgetItem *it = ui->bleServicesTreeWidget->currentItem();
    if (!it) {
        ui->statusbar->clearMessage();
        ui->statusbar->showMessage("First, connect to a BLE device and select a READ characteristic!");
        return;
    }

    // if a characteristic selected
    if (it->data(0, Qt::UserRole).canConvert<QLowEnergyCharacteristic>()) {
        QLowEnergyCharacteristic ch = it->data(0,Qt::UserRole).value<QLowEnergyCharacteristic>();

        // the top-level parent holds the service
        QTreeWidgetItem *p = it->parent();
        while (p->parent() != nullptr) { p = p->parent(); }

        if (p->data(1, Qt::UserRole).canConvert<QLowEnergyService*>()) {
            QLowEnergyService *s = p->data(1, Qt::UserRole).value<QLowEnergyService*>();
            // read the characteristic
            s->readCharacteristic(ch);

            // TODO: If the operation is successful, the characteristicRead() signal is emitted.
            // TODO: Otherwise the CharacteristicReadError is set.
            // A characteristic can only be read if the service is in the ServiceDiscovered state and belongs to the service.
            // If one of these conditions is not true the QLowEnergyService::OperationError is set.
        }
    } else {
        ui->statusbar->clearMessage();
        ui->statusbar->showMessage("Not a characteristic! Select a characteristic!");
        return;
    }
}

void MainWindow::on_bleCharacteristicWritePushButton_clicked()
{
    // if no items selected, give a hint to the user and return
    QTreeWidgetItem *it = ui->bleServicesTreeWidget->currentItem();
    if (!it) {
        ui->statusbar->clearMessage();
        ui->statusbar->showMessage("First, connect to a BLE device and select a WRITE characteristic!");
        return;
    }

    // if a characteristic selected
    if (it->data(0, Qt::UserRole).canConvert<QLowEnergyCharacteristic>()) {
        QLowEnergyCharacteristic ch = it->data(0, Qt::UserRole).value<QLowEnergyCharacteristic>();

        if (!(ch.properties() & QLowEnergyCharacteristic::PropertyType::WriteNoResponse)) {
            if(!(ch.properties() & QLowEnergyCharacteristic::PropertyType::Write)){
                if(!(ch.properties() & QLowEnergyCharacteristic::PropertyType::WriteSigned)){
                    ui->statusbar->clearMessage();
                    ui->statusbar->showMessage("The characteristic's descriptor is invalid!");
                    return;
                }
            }
        }

        // the top-level parent holds the service
        QTreeWidgetItem *p = it->parent();
        while (p->parent() != nullptr) { p = p->parent(); }

        if (p->data(1, Qt::UserRole).canConvert<QLowEnergyService*>()) {
            QLowEnergyService *s = p->data(1, Qt::UserRole).value<QLowEnergyService*>();

            // read out and parse the input value
            QString value = ui->bleCharacteristicWriteLineEdit->text();

            bool parseOK;
            int parseBase = 16;
            value.toUInt(&parseOK, parseBase);

            if(parseOK) {
                // transmit the UTF-8 representation of input value or show a hint how to fix the error
                s->writeCharacteristic(ch, QByteArray::fromHex(value.toUtf8()), QLowEnergyService::WriteWithoutResponse);
                ui->statusbar->clearMessage();
                ui->statusbar->showMessage("0x" + value + " has been written!");
            } else {
                ui->statusbar->clearMessage();
                ui->statusbar->showMessage("Convertion to HEX failed: only characters '0-9' and 'a-f' are allowed! Fix the value and try to write again!");
            }
        } else {
            ui->statusbar->clearMessage();
            ui->statusbar->showMessage("Cannot find the enclosing service!");
            return;
        }
    } else {
        ui->statusbar->clearMessage();
        ui->statusbar->showMessage("Not a characteristic! Select a characteristic!");
        return;
    }
}

void MainWindow::on_listenNotifyPushButton_clicked()
{
    QTreeWidgetItem *it = ui->bleServicesTreeWidget->currentItem();
    if (!it) {
        ui->statusbar->clearMessage();
        ui->statusbar->showMessage("First, connect to a BLE device and select a NOTIFY characteristic!");
        return;
    }

    if (!(it->data(0, Qt::UserRole).canConvert<QLowEnergyCharacteristic>())) {
        ui->statusbar->clearMessage();
        ui->statusbar->showMessage("Not a characteristic! Select a characteristic!");
        return;
    }

    QLowEnergyCharacteristic characteristic = it->data(0, Qt::UserRole).value<QLowEnergyCharacteristic>();

    if (characteristic.properties().toInt() != QLowEnergyCharacteristic::Notify) {
        ui->statusbar->clearMessage();
        ui->statusbar->showMessage("The characteristic's descriptor is invalid!");
        return;
    }

    QTreeWidgetItem *p = it;
    while (p->parent() != nullptr) { p = p->parent(); }

    if (!(p->data(1, Qt::UserRole).canConvert<QLowEnergyService*>())) {
        ui->statusbar->clearMessage();
        ui->statusbar->showMessage("Cannot find the enclosing service!");
        return;
    };

    QLowEnergyService *s = p->data(1, Qt::UserRole).value<QLowEnergyService*>();

    QLowEnergyDescriptor charDescriptor = characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);

    if (charDescriptor.isValid()) {
        // enable notification
        ui->statusbar->clearMessage();
        ui->statusbar->showMessage("Listen for the selected notifications!");
        s->writeDescriptor(charDescriptor, QByteArray::fromHex("0100"));
    } else {
        ui->statusbar->clearMessage();
        ui->statusbar->showMessage("The characteristic's descriptor is invalid!");
    }
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

void MainWindow::on_bleServicesTreeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    #ifdef DEBUG
        qDebug() << "on_bleServicesTreeWidget_currentItemChanged() has been called";
    #endif

    (void) previous;
    (void) current;
}

void MainWindow::on_clearOutputPushButton_clicked()
{
    ui->outputPlainTextEdit->clear();
}

void MainWindow::on_browsePushButton_clicked()
{
    csvFilePath = QFileDialog::getOpenFileName(this, tr("Save Characteristics"),
                                                    "", tr("CSV File (*.csv);;All Files (*)"));

    if (csvFilePath.isEmpty()) return;
    ui->saveLineEdit->setText(csvFilePath);
}

void MainWindow::append_to_csv(const QString& header, const QString& data)
{
    // attempt to open csv-file to append it
    QFile file(csvFilePath);

    if(!file.open(QIODevice::Append)) {
        QMessageBox::information(this, tr("Unable to open file to append"),
                                 file.errorString());
        return;
    }

    // append data to file
    QTextStream stream(&file);

    // stream the data to the file as a pair "header-data"
    stream << header << ",";                                // comma is used to move to the next column
    stream << data << "\n";                                 // '\n' is used to move to the next row

    stream.flush();
    file.close();
}
