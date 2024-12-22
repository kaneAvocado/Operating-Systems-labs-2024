#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QListWidget>
#include <QString>
#include <vector>
#include <string>
#include <QWidget>
#include <QTimer>
#include "Book.hpp"

class HostWindow : public QMainWindow {
    Q_OBJECT

public:
    HostWindow(const std::string& hostTitle, const std::vector<Book>& books, QWidget* parent = nullptr);
    virtual ~HostWindow();

    void updateBooks(const std::vector<Book>& books);
    void signalResetTimer();
    void signalStopTimer();
    
    void addHistory(const QString& action, const QString& bookName, bool success);

    pid_t clientPid; // for kill

signals:
    void resetSignalTimer();
    void stopSignalTimer();

private slots:
    void terminateClient();
    void terminateHost();
    void resetTimer();

private:
    QLabel* portLabel;
    QListWidget* bookList;
    QListWidget* historyList;
    QPushButton* terminateClientButton;
    QPushButton* terminateHostButton;

    QTimer* clientTimer;
    QLabel* timerLabel;
};
