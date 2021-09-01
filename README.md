# ble-basestation-app
This application is used to:
1. Discover/scan for Bluetooth devices (both standard and low-energy/BLE ones).
2. Display the address, the name, the configuration (Standard/BLE), and the RSSI[dB] for each discovered device.
3. Connect to/disconnect from the discovered device.
4. Display the Bluetooth services and characteristics of the connected device.
5. Listen to the Notify characteristics.
6. Read and write the characteristics with the corresponding properties/permissions.
7. Save the content of the Read and Notify characteristics into a csv file.

## Tests
The application was tested with the STM development board "P-NUCLEO-WB55" (the STM32WB55 MCU) flashed with the proprietary P2P application example [[AN5289](https://www.st.com/resource/en/application_note/dm00598033-building-wireless-applications-with-stm32wb-series-microcontrollers-stmicroelectronics.pdf)].

## Technology
1. The application is based on Qt 5.15.2 (GCC 7.3.1 20180303 (Red Hat 7.3.1-5), 64 bit).
2. The application uses standard Qt widgets (not QML). 
3. The application was built in Qt Creator 5.0.0-rc1 (4.84.0) on Ubuntu 20.04.2 LTS.

## Acknowledgements
[Joel Svensson](https://github.com/svenssonjoel) for their project of [qscanner](https://github.com/svenssonjoel/BLE_S/tree/master/qscanner).
