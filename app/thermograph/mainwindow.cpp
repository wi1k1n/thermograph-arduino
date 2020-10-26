#include <QDebug>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QMessageBox>
#include <QLabel>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    serial(new QSerialPort(this)),
    lblStatus(new QLabel)
{
    ui->setupUi(this);
    ui->statusBar->addWidget(lblStatus);
    lblStatus->setText("Disconnected");

    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    connect(ui->actionExit, &QAction::triggered, qApp, QApplication::quit);

    connect(serial, &QSerialPort::readyRead, this, &MainWindow::serialReadyRead);
    connect(serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);

    qDebug() << "Hello from constructor!";
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionAbout_triggered() {
    qDebug() << "Hello About!";
}

void MainWindow::on_menuConnect_aboutToShow() {
//    qDebug() << "Hello from menuPort!";
    const QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();

    ui->menuConnect->clear();
    for (auto iter = portList.begin(); iter != portList.end(); ++iter) {
        QAction *qa = new QAction(iter->portName());
        qa->setCheckable(true);
        connect(qa, &QAction::triggered, this, [=](){onPortClicked(qa);});

        ui->menuConnect->addAction(qa);
    }
    QAction *qaSeparator = new QAction(this);
    qaSeparator->setSeparator(true);
    ui->menuConnect->addAction(qaSeparator);

    QAction *qaClosePort = new QAction("Close port");
    connect(qaClosePort, &QAction::triggered, this, &MainWindow::onClosePortClicked);
    ui->menuConnect->addAction(qaClosePort);
}

void MainWindow::onPortClicked(QAction* act) {
    qDebug() << act->text();
    if (act->text() == serial->portName()) {
        if (serial->isOpen()) {
            qDebug() << "Port: " << serial->portName() << " is already opened!";
            return;
        }
    } else {
        serial->setPortName(act->text());
    }

    if (serial->open(QIODevice::ReadWrite)) {
        qDebug() << "Successfully opened port: " << serial->portName();
        lblStatus->setText(QString("Connected: %1").arg(act->text()));
        foreach (QAction *action, ui->menuConnect->actions()) {
            action->setText(QString("%11").arg(action->text()));
        }

    } else
        qDebug() << "Error opening port: " << serial->portName();

}
void MainWindow::onClosePortClicked() {
    qDebug() << "Close port!";
    if (serial->isOpen())
        serial->close();
    lblStatus->setText("Disconnected");
}

void MainWindow::handleError(const QSerialPort::SerialPortError err) {
    QString errMsg;
    switch (err) {
        case QSerialPort::NoError: return;
        case QSerialPort::DeviceNotFoundError: errMsg = "Attempted to open non-existing device."; break;
        case QSerialPort::PermissionError: errMsg = "Can't open device. Access denied."; break;
        case QSerialPort::OpenError: errMsg = "Error while opening device."; break;
        case QSerialPort::WriteError: errMsg = "Error occured while writing the data."; break;
        case QSerialPort::ReadError: errMsg = "Error occured while reading the data."; break;
        case QSerialPort::ResourceError: errMsg = "Device became unavailable (disconnected?)"; break;
        default: errMsg = "Unknown error."; break;
    }
    QMessageBox::critical(this, tr("Critical Error"), QString("%1\n%2").arg(errMsg, serial->errorString()));
    if (serial->isOpen())
        serial->close();
    lblStatus->setText("Disconnected");
}

void MainWindow::serialReadyRead() {
    const QByteArray data = serial->readAll();

    qDebug() << "Received [" << data.length() << "]: " << data;
    serialData.append(data);

    if (serialData.right(2) == "\r\n") {
        if (serialCmdState == CMDSTATE_DATA) {
            qDebug() << serialData.left(serialData.length() - 2);
            ui->plainTextEdit->document()->setPlainText(serialData.left(serialData.length() - 2).replace(',', '\n').replace(" ", "\n\n"));
            serialCmdState = CMDSTATE_NONE;
        } else if (serialCmdState == CMDSTATE_EEPROM) {
            qDebug() << serialData.left(serialData.length() - 2);
            ui->plainTextEdit->document()->setPlainText(serialData.left(serialData.length() - 2).replace(',', '\n').replace(" ", "\n\n"));
            serialCmdState = CMDSTATE_NONE;
        }
    }
}

void MainWindow::on_btnGetData_clicked(const bool checked) {
    serial->write(QByteArray(1, USBCMD_SENDDATA));
    serialCmdState = CMDSTATE_DATA;
    serialData = "";
}

void MainWindow::on_btnGetEEPROM_clicked(const bool checked) {
    serial->write(QByteArray(1, USBCMD_SENDEEPROM));
    serialCmdState = CMDSTATE_EEPROM;
    serialData = "";
}
