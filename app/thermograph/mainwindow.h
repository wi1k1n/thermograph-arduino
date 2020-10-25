#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>

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
    void onPortClicked(const QString &portName);
    void handleError(QSerialPort::SerialPortError);
    void serialReadyRead();

private:
    Ui::MainWindow *ui = nullptr;
    QSerialPort *serial = nullptr;
};

#endif // MAINWINDOW_H
