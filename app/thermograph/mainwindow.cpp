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

    reloadPorts();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionAbout_triggered() {
    QMessageBox::about(this, tr("Thermograph App"), tr("This is an app for thermograph project. Find more on https://github.com/wi1k1n/thermograph-arduino"));
}

void MainWindow::on_menuConnect_aboutToShow() {
    reloadPorts();
}
void MainWindow::reloadPorts() {
    // get list of available ports
    const QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();

    // show warning if on COM ports detected
    if (portList.length() == 0) {
        QMessageBox::warning(this, tr("No thermograph detected!"), tr("Please check if you have connected thermograph to your PC!"));
        ui->menuConnect->clear();
        return;
    }

    // compose qmenu if there are some
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

    QAction *qaClosePort = new QAction(tr("Close port"));
    connect(qaClosePort, &QAction::triggered, this, &MainWindow::onClosePortClicked);
    ui->menuConnect->addAction(qaClosePort);

    // try to connect if there is only one available port
    if (portList.length() == 1) {
        openSerial(portList.first().portName());
    }
}

void MainWindow::onPortClicked(QAction* act) {
//    qDebug() << act->text();
    if (act->text() == serial->portName()) {
        if (serial->isOpen()) {
            qDebug() << "Port: " << serial->portName() << " is already opened!";
            return;
        }
    }
    openSerial(act->text());
}
void MainWindow::onClosePortClicked() {
    qDebug() << "Close port!";
    closeSerial();
}

void MainWindow::openSerial(const QString &portName) {
    if (serial->portName() != portName)
        serial->setPortName(portName);

    if (serial->open(QIODevice::ReadWrite)) {
        qDebug() << "Successfully opened port: " << serial->portName();
        lblStatus->setText(QString(tr("Connected: %1")).arg(serial->portName()));
    } else {
        qDebug() << "Error opening port: " << serial->portName();
    }
}
void MainWindow::closeSerial() {
    if (serial->isOpen())
        serial->close();
    lblStatus->setText(tr("Disconnected"));
}

void MainWindow::handleError(const QSerialPort::SerialPortError err) {
    QString errMsg;
    switch (err) {
        case QSerialPort::NoError: return;
        case QSerialPort::DeviceNotFoundError: errMsg = tr("Attempted to open non-existing device."); break;
        case QSerialPort::PermissionError: errMsg = tr("Can't open device. Access denied."); break;
        case QSerialPort::OpenError: errMsg = tr("Error while opening device."); break;
        case QSerialPort::WriteError: errMsg = tr("Error occured while writing the data."); break;
        case QSerialPort::ReadError: errMsg = tr("Error occured while reading the data."); break;
        case QSerialPort::ResourceError: errMsg = tr("Device became unavailable (disconnected?)"); break;
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
        qDebug() << serialData.left(serialData.length() - 2);
        if (serialCmdState == CMDSTATE_DATA) {
            ui->plainTextEdit->document()->setPlainText(serialData.left(serialData.length() - 2).replace(',', '\n').replace(" ", "\n\n"));
            serialCmdState = CMDSTATE_NONE;
        } else if (serialCmdState == CMDSTATE_EEPROM) {
            ui->plainTextEdit->document()->setPlainText(serialData.left(serialData.length() - 2).replace(',', '\n').replace(" ", "\n\n"));
            serialCmdState = CMDSTATE_NONE;
        } else if (serialCmdState == CMDSTATE_LIVE) {
            ui->plainTextEdit->moveCursor (QTextCursor::End);
            ui->plainTextEdit->insertPlainText(serialData);
            serialData = "";
        }
    }
}

void MainWindow::on_btnGetData_clicked(const bool) {
    stopLive();
    serial->write(QByteArray(1, USBCMD_SENDDATA));
    serialCmdState = CMDSTATE_DATA;
    serialData = "";
}

void MainWindow::on_btnGetEEPROM_clicked(const bool) {
    stopLive();
    serial->write(QByteArray(1, USBCMD_SENDEEPROM));
    serialCmdState = CMDSTATE_EEPROM;
    serialData = "";
}

void MainWindow::on_btnLive_clicked(const bool) {
    ui->btnLive->setChecked(ui->btnLive->isChecked());
    if (ui->btnLive->isChecked()) {
        serial->write(QByteArray(1, USBCMD_SENDLIVESTART));
        serialCmdState = CMDSTATE_LIVE;
        serialData = "";
        ui->plainTextEdit->document()->setPlainText("");
    }
    // if turning live off
    else stopLive();
}

void MainWindow::stopLive() {
    serial->write(QByteArray(1, USBCMD_SENDLIVESTOP));
    serialCmdState = CMDSTATE_NONE;
    ui->btnLive->setChecked(false);
}
