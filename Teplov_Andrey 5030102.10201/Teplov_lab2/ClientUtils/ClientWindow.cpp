#include "ClientWindow.hpp"
#include <QMessageBox>
#include <QDockWidget>
#include <QDateTime>
#include <iostream>
#include "logger.hpp"

ClientWindow::ClientWindow(const std::vector<Book>& books, QWidget* parent)
    : QMainWindow(parent) {
    stackedWidget = new QStackedWidget(this);

    createBookView(books);
    createReadingView();
    createHistoryView();

    setCentralWidget(stackedWidget);
    setWindowTitle("Client Window");
    resize(400, 300);

    // Set started window
    stackedWidget->setCurrentIndex(0);
}

ClientWindow::~ClientWindow() {}

void ClientWindow::createBookView(const std::vector<Book>& books) {
    QWidget* bookView = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(bookView);

    bookList = new QListWidget(this);
    for (const auto& book : books) {
        bookList->addItem(QString::fromStdString(book.name) + " - " + QString::number(book.count) + " copies");
    }
    layout->addWidget(bookList);

    selectButton = new QPushButton("Select Book", this);
    selectButton->setEnabled(false);
    layout->addWidget(selectButton);

    //terminateClientButton = new QPushButton("Terminate Client", this);
    //layout->addWidget(terminateClientButton);

    // Enable button if book is choosed
    connect(bookList, &QListWidget::itemSelectionChanged, [this]() {
        selectButton->setEnabled(bookList->currentItem() != nullptr);
    });
    connect(selectButton, &QPushButton::clicked, this, &ClientWindow::selectBook);
    //connect(terminateClientButton, &QPushButton::clicked, this, &ClientWindow::terminateClient);

    stackedWidget->addWidget(bookView);
}

void ClientWindow::createReadingView() {
    QWidget* readingView = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(readingView);

    readingLabel = new QLabel("Reading book: ", this);
    layout->addWidget(readingLabel);

    cancelReadingButton = new QPushButton("Cancel Reading", this);
    layout->addWidget(cancelReadingButton);

    connect(cancelReadingButton, &QPushButton::clicked, this, &ClientWindow::cancelReading);

    stackedWidget->addWidget(readingView);
}

void ClientWindow::createHistoryView() {
    historyList = new QListWidget(this);
    historyList->setFixedWidth(350);
    auto* widget = new QDockWidget("History", this);
    widget->setWidget(historyList);
    addDockWidget(Qt::LeftDockWidgetArea, widget);
}

void ClientWindow::selectBook() {
    if (bookList->currentItem()) {
        QString bookName = bookList->currentItem()->text().split(" - ").first();
        readingLabel->setText("Reading book: " + bookName);

        emit bookSelected(bookName);
    }
}

void ClientWindow::cancelReading() {
    QString bookName = readingLabel->text().split(": ").last();
    emit bookReturned(bookName);

    // Set started window
    stackedWidget->setCurrentIndex(0);

    addHistory("Cancelled reading: ", bookName, true);
}

void ClientWindow::addHistory(const QString& action, const QString& bookName, bool success) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString status = success ? "SUCCESS" : "FAIL";
    historyList->addItem(QString("[%1] %2 \"%3\": %4").arg(timestamp, action, bookName, status));
}

void ClientWindow::onSuccessTakeBook() {
    // Set reading window
    stackedWidget->setCurrentIndex(1);

    addHistory("TAKE book: ", bookList->currentItem()->text().split(" - ").first(), true);
}

void ClientWindow::onFailedTakeBook() {
    // Cannot display this window....
    // QMessageBox::warning(nullptr, "FAIL", "Failed to take book");
    LoggerClient::get_instance().log(Status::ERROR, "Failed to take book");

    addHistory("TAKE book: ", bookList->currentItem()->text().split(" - ").first(), false);
}

void ClientWindow::terminateClient() {
    QMessageBox::information(this, "Terminate Client", "Client terminated.");
    std::exit(0); // TODO: exit only qapp, but don't whole process
}
