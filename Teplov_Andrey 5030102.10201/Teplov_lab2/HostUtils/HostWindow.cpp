#include "HostWindow.hpp"
#include "logger.hpp"
#include <QMessageBox>
#include <QApplication>
#include <QDockWidget>
#include <QDateTime>
#include <cstdlib>
#include <signal.h>

static int secondsLeft = 5;

HostWindow::HostWindow(const std::string& hostTitle, const std::vector<Book>& books, QWidget* parent)
    : QMainWindow(parent) {

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    portLabel = new QLabel(QString::fromStdString(hostTitle), this);
    layout->addWidget(portLabel);

    bookList = new QListWidget(this);
    for (const auto& book : books) {
        bookList->addItem(QString::fromStdString(book.name) + " - " + QString::number(book.count) + " copies");
    }
    layout->addWidget(bookList);

    terminateClientButton = new QPushButton("Terminate Client", this);
    layout->addWidget(terminateClientButton);

    terminateHostButton = new QPushButton("Terminate Host", this);
    layout->addWidget(terminateHostButton);

    timerLabel = new QLabel("Time left: 5 seconds", this);
    layout->addWidget(timerLabel);

    clientTimer = new QTimer(this);
    clientTimer->setInterval(1000);
    connect(clientTimer, &QTimer::timeout, this, [this]() {
        if (!clientTimer->isActive()) return;

        if (--secondsLeft <= 0) {
            terminateClient();
        } else {
            timerLabel->setText(QString("Time left: %1 seconds").arg(secondsLeft));
        }
    });

    connect(terminateClientButton, &QPushButton::clicked, this, &HostWindow::terminateClient);
    connect(terminateHostButton, &QPushButton::clicked, this, &HostWindow::terminateHost);
    connect(this, &HostWindow::resetSignalTimer, this, &HostWindow::resetTimer);
    connect(this, &HostWindow::stopSignalTimer, this, [this]() {
        timerLabel->setText("Client reading");
        clientTimer->stop();
    });

    historyList = new QListWidget(this);
    QDockWidget* historyDock = new QDockWidget("History", this);
    historyDock->setWidget(historyList);
    addDockWidget(Qt::RightDockWidgetArea, historyDock);

    setCentralWidget(centralWidget);
    setWindowTitle("Host Window");
    resize(600, 450);

    clientTimer->start();
}

HostWindow::~HostWindow() {}

void HostWindow::updateBooks(const std::vector<Book>& books) {
    bookList->clear();
    for (const auto& book : books) {
        bookList->addItem(QString::fromStdString(book.name) + " - " + QString::number(book.count) + " copies");
    }
}

void HostWindow::addHistory(const QString& action, const QString& bookName, bool success) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString status = success ? "SUCCESS" : "FAIL";
    historyList->addItem(QString("[%1] Client ID: 1, %2 \"%3\": %4").arg(timestamp, action, bookName, status));
}

void HostWindow::terminateClient() {
    clientTimer->stop();
    timerLabel->setText("Client terminated");
    QMessageBox::information(this, "Terminate Client", "Client terminated.");
    LoggerHost::get_instance().log(Status::INFO, "Terminate Client");
    kill(clientPid, SIGKILL);
}

void HostWindow::terminateHost() {
    QMessageBox::information(this, "Terminate Host", "Host terminated.");
    LoggerHost::get_instance().log(Status::INFO, "Terminate Host");
    kill(clientPid, SIGKILL); // and kill client too
    std::exit(0);
}

void HostWindow::resetTimer() {
    secondsLeft = 5;
    timerLabel->setText("Time left: 5 seconds");
    clientTimer->start();
}

void HostWindow::signalResetTimer() {
    emit resetSignalTimer();
}

void HostWindow::signalStopTimer() {
    emit stopSignalTimer();
}
