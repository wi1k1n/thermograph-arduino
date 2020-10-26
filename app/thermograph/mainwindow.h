#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QLabel>

#define CMDSTATE_NONE       0
#define CMDSTATE_SERVICE    1
#define CMDSTATE_DATA       2

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

    void handleError(const QSerialPort::SerialPortError);
    void serialReadyRead();

private:
    Ui::MainWindow *ui = nullptr;
    QSerialPort *serial = nullptr;
    QLabel *lblStatus = nullptr;

    uint8_t serialCmdState = CMDSTATE_NONE;
    QString serialData;


};

#endif // MAINWINDOW_H
