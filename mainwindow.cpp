#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , networkManager(new QNetworkAccessManager(this)) // Initialize the network manager
{
    ui->setupUi(this);

    // Connect buttons to their respective slots
    connect(ui->FetchAllFlights, &QPushButton::clicked, this, &MainWindow::fetchAllFlights);
    connect(ui->fetchFlightInfo, &QPushButton::clicked, this, &MainWindow::fetchFlightInfo);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Fetch all flights and populate the table
void MainWindow::fetchAllFlights()
{
    QUrl url("http://192.168.1.2:3000/flights");
    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleAllFlightsReply(reply);
    });
}

// Fetch specific flight info based on the flight register
void MainWindow::fetchFlightInfo()
{
    QString flightRegister = ui->FlightRegister->text();
    if (flightRegister.isEmpty()) {
        ui->labelStatus->setText("Enter a flight register.");
        return;
    }

    QUrl url(QString("http://192.168.1.2:3000/flight/%1/info").arg(flightRegister));
    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleFlightInfoReply(reply);
    });
}

// Handle the response from /flights
void MainWindow::handleAllFlightsReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray response = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
        QJsonObject jsonObj = jsonDoc.object();

        QJsonArray flights = jsonObj.value("flights").toArray();

        // Populate tableWidget
        ui->tableWidgetFlights->setRowCount(flights.size());
        ui->tableWidgetFlights->setColumnCount(1);
        ui->tableWidgetFlights->setHorizontalHeaderLabels(QStringList() << "Flight Registers");

        for (int i = 0; i < flights.size(); ++i) {
            QTableWidgetItem *item = new QTableWidgetItem(flights[i].toString());
            ui->tableWidgetFlights->setItem(i, 0, item);
        }

        ui->labelStatus->setText("Fetched all flights successfully.");
    } else {
        ui->labelStatus->setText("Error fetching flights: " + reply->errorString());
    }

    reply->deleteLater();
}

void MainWindow::handleFlightInfoReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        // Parse the JSON response
        QByteArray response = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
        QJsonObject jsonObj = jsonDoc.object();

        // Extract flight details
        QString flightRegister = jsonObj.value("flightRegister").toString();
        QString flightNumber = jsonObj.value("flightNumber").toString();
        QString latitude = jsonObj.value("lat").toString();
        QString longitude = jsonObj.value("long").toString();
        QString speed = jsonObj.value("Speed").toString();
        QString altitude = jsonObj.value("Altitude").toString();

        // Set up table widget to display the data
        QTableWidget *table = ui->tableWidgetFlightInfo;
        table->clear(); // Clear previous data
        table->setRowCount(6); // Set 6 rows for 6 details
        table->setColumnCount(2); // Set 2 columns (key and value)
        table->setHorizontalHeaderLabels({"Field", "Value"});

        // Populate the table with flight details
        table->setItem(0, 0, new QTableWidgetItem("Flight Register"));
        table->setItem(0, 1, new QTableWidgetItem(flightRegister));

        table->setItem(1, 0, new QTableWidgetItem("Flight Number"));
        table->setItem(1, 1, new QTableWidgetItem(flightNumber));

        table->setItem(2, 0, new QTableWidgetItem("Latitude"));
        table->setItem(2, 1, new QTableWidgetItem(latitude));

        table->setItem(3, 0, new QTableWidgetItem("Longitude"));
        table->setItem(3, 1, new QTableWidgetItem(longitude));

        table->setItem(4, 0, new QTableWidgetItem("Speed"));
        table->setItem(4, 1, new QTableWidgetItem(speed));

        table->setItem(5, 0, new QTableWidgetItem("Altitude"));
        table->setItem(5, 1, new QTableWidgetItem(altitude));

        // Adjust the table's appearance
        table->resizeColumnsToContents();
        table->resizeRowsToContents();

        // Update status label
        ui->labelStatus->setText("Fetched flight info successfully.");
    } else {
        // Display error in status label
        ui->labelStatus->setText("Error fetching flight info: " + reply->errorString());
    }

    reply->deleteLater(); // Clean up the reply object
}

