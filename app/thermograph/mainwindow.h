#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QLabel>

#define CMDSTATE_NONE       0
#define CMDSTATE_EEPROM     1
#define CMDSTATE_DATA       2
#define CMDSTATE_LIVE       3

#define USBCMD_PING             0x02
#define USBCMD_SENDDATA         0x09
#define USBCMD_SENDEEPROM       0x0F
#define USBCMD_SENDLIVESTART    0x12
#define USBCMD_SENDLIVESTOP     0x13

#define USBSTATUS_BEGIN     "begin"
#define USBSTATUS_PONG      "pong"
#define USBSTATUS_END       "end"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionAbout_triggered();
    void on_menuConnect_aboutToShow();
    void onPortClicked(QAction *portName);
    void onClosePortClicked();

    void on_btnGetData_clicked(const bool checked);
    void on_btnGetEEPROM_clicked(const bool checked);
    void on_btnLive_clicked(const bool checked);

    void handleError(const QSerialPort::SerialPortError);
    void serialReadyRead();

private:
    Ui::MainWindow *ui = nullptr;
    QSerialPort *serial = nullptr;
    QLabel *lblStatusPort = nullptr;
    QLabel *lblStatusTherm = nullptr;

    uint8_t serialCmdState = CMDSTATE_NONE;
    QString serialData;

    void reloadPorts(const bool autoConnect = false);
    void openSerial(const QString &portName);
    void closeSerial();
    void stopLive();
};

#endif // MAINWINDOW_H
