#include "mainwindow.h"
#include "ui_mainwindow.h"
//constructor
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      passengerLayout(nullptr),
      timeStep(0),
      simulationStarted(false),
      elevator1(new Elevator(1, this, nullptr)),
      elevator2(new Elevator(2, this, nullptr)),
      ecs(new ElevatorControlSystem({elevator1, elevator2})),
      lastElevatorState(Elevator::Idle)
{
    //init ecs connect elevators GUI and begin the pre-simulation setupp
    connectElevatorSignals();
    elevator1->setECS(ecs);
    elevator2->setECS(ecs);
    ui->setupUi(this);
    passengerLayout = new QVBoxLayout(ui->passengerWidget);
    ui->logWindow->setVisible(true);
    connectUIButtons();
    initializeUI();
    addPassenger();
}
//deconstructor
MainWindow::~MainWindow() {
    delete ui;
    delete elevator1;
    delete elevator2;
    delete ecs;
}
//connects signall to update elevator status and floor display
void MainWindow::connectElevatorSignals() {
    connect(elevator1, &Elevator::stateChanged, this, &MainWindow::updateElevatorStatus);
    connect(elevator1, &Elevator::floorChanged, this, &MainWindow::updateElevatorFloor);
    connect(elevator2, &Elevator::stateChanged, this, &MainWindow::updateElevatorStatus);
    connect(elevator2, &Elevator::floorChanged, this, &MainWindow::updateElevatorFloor);
}
//connects signal to GUI Buttons
void MainWindow::connectUIButtons() {
    connect(ui->fireButton1, &QPushButton::clicked, elevator1, &Elevator::handleFire);
    connect(ui->powerButton1, &QPushButton::clicked, elevator1, &Elevator::handlePower);
    connect(ui->fireButton2, &QPushButton::clicked, elevator2, &Elevator::handleFire);
    connect(ui->powerButton2, &QPushButton::clicked, elevator2, &Elevator::handlePower);
    connect(ui->start, &QPushButton::clicked, this, &MainWindow::startSimulation);
    connect(ui->endSimulation, &QPushButton::clicked, this, &MainWindow::endSimulation);
    connect(ui->addPassenger, &QPushButton::clicked, this, &MainWindow::addPassenger);
    connect(ui->removePassenger, &QPushButton::clicked, this, &MainWindow::removePassenger);
}
//inits what is visible to the user
void MainWindow::initializeUI() {
    ui->elevator1->setVisible(false);
    ui->elevator2->setVisible(false);
    ui->endSimulation->setVisible(false);
    ui->start->setVisible(true);
    ui->passengerScroll->setVisible(true);
    ui->addPassenger->setVisible(true);
    ui->removePassenger->setVisible(true);
}
//function to add passengerrs and set their predetermined actions
void MainWindow::addPassenger() {
    //create all the passenger input boxes/layouts
    int passengerNumber = passengerInputs.size() + 1;
    QString passengerName = QString("Passenger %1").arg(passengerNumber);
    QLabel *passengerLabel = new QLabel(passengerName);
    QSpinBox *startFloorSpinBox = new QSpinBox();
    startFloorSpinBox->setRange(1, 100);
    startFloorSpinBox->setValue(1);
    QSpinBox *destinationFloorSpinBox = new QSpinBox();
    destinationFloorSpinBox->setRange(1, 100);
    destinationFloorSpinBox->setValue(2);
    QWidget *passengerConfigWidget = new QWidget();
    QVBoxLayout *passengerConfigLayout = new QVBoxLayout(passengerConfigWidget);
    passengerConfigLayout->addWidget(passengerLabel);
    QHBoxLayout *floorConfigLayout = new QHBoxLayout();
    floorConfigLayout->addWidget(new QLabel("Start Floor:"));
    floorConfigLayout->addWidget(startFloorSpinBox);
    floorConfigLayout->addWidget(new QLabel("Destination Floor:"));
    floorConfigLayout->addWidget(destinationFloorSpinBox);
    passengerConfigLayout->addLayout(floorConfigLayout);
    //configure the passenger actions of passenger
    QStringList actions = {"Press Button"};
    QList<QSpinBox*> actionTimeSpins;
    QGroupBox *actionGroupBox = new QGroupBox("Actions");
    QGridLayout *actionLayout = new QGridLayout();
    for (int i = 0; i < actions.size(); ++i) {
        QLabel *actionLabel = new QLabel(actions[i] + " Time:");
        QSpinBox *actionSpinBox = new QSpinBox();
        actionSpinBox->setRange(1, 100);
        actionSpinBox->setValue(i + 1);
        actionLayout->addWidget(actionLabel, i, 0);
        actionLayout->addWidget(actionSpinBox, i, 1);
        actionTimeSpins.append(actionSpinBox);
    }
    actionGroupBox->setLayout(actionLayout);
    passengerConfigLayout->addWidget(actionGroupBox);
    //safety features and door features
    QCheckBox *openDoorsCheckBox = new QCheckBox("Open Doors");
    QSpinBox *openDoorsTimeSpinBox = new QSpinBox();
    openDoorsTimeSpinBox->setRange(1, 100);
    openDoorsTimeSpinBox->setEnabled(false);
    connect(openDoorsCheckBox, &QCheckBox::toggled, openDoorsTimeSpinBox, &QSpinBox::setEnabled);
    QCheckBox *closeDoorsCheckBox = new QCheckBox("Close Doors");
    QSpinBox *closeDoorsTimeSpinBox = new QSpinBox();
    closeDoorsTimeSpinBox->setRange(1, 100);
    closeDoorsTimeSpinBox->setEnabled(false);
    connect(closeDoorsCheckBox, &QCheckBox::toggled, closeDoorsTimeSpinBox, &QSpinBox::setEnabled);
    QHBoxLayout *doorsLayout = new QHBoxLayout();
    doorsLayout->addWidget(openDoorsCheckBox);
    doorsLayout->addWidget(openDoorsTimeSpinBox);
    doorsLayout->addWidget(closeDoorsCheckBox);
    doorsLayout->addWidget(closeDoorsTimeSpinBox);
    passengerConfigLayout->addLayout(doorsLayout);
    QStringList safetyEvents = {"Obstructed", "Overloaded", "Help Alarm"};
    QList<QCheckBox*> safetyEventChecks;
    QList<QSpinBox*> safetyEventTimes;

    for (const QString &event : safetyEvents) {
        QCheckBox *eventCheckBox = new QCheckBox(event);
        QSpinBox *eventTimeSpinBox = new QSpinBox();
        eventTimeSpinBox->setRange(1, 100);
        eventTimeSpinBox->setEnabled(false);
        connect(eventCheckBox, &QCheckBox::toggled, eventTimeSpinBox, &QSpinBox::setEnabled);
        safetyEventChecks.append(eventCheckBox);
        safetyEventTimes.append(eventTimeSpinBox);
        QHBoxLayout *eventLayout = new QHBoxLayout();
        eventLayout->addWidget(eventCheckBox);
        eventLayout->addWidget(eventTimeSpinBox);
        passengerConfigLayout->addLayout(eventLayout);
    }
    //stores the inputs to be later used
    PassengerInput newPassengerInput;
    newPassengerInput.startFloorSpinBox = startFloorSpinBox;
    newPassengerInput.destinationFloorSpinBox = destinationFloorSpinBox;
    newPassengerInput.actionTimeSpins = actionTimeSpins;
    newPassengerInput.openDoorsCheckBox = openDoorsCheckBox;
    newPassengerInput.openDoorsTimeSpinBox = openDoorsTimeSpinBox;
    newPassengerInput.closeDoorsCheckBox = closeDoorsCheckBox;
    newPassengerInput.closeDoorsTimeSpinBox = closeDoorsTimeSpinBox;
    newPassengerInput.safetyEventChecks = safetyEventChecks;
    newPassengerInput.safetyEventTimes = safetyEventTimes;
    newPassengerInput.rowWidget = passengerConfigWidget;

    passengerInputs.append(newPassengerInput);
    passengerLayout->addWidget(passengerConfigWidget);
    int startFloor = startFloorSpinBox->value();
    int destinationFloor = destinationFloorSpinBox->value();

    //list of all the actions
    QList<QPair<QString, int>> actionSequence;
    for (int i = 0; i < safetyEvents.size(); ++i) {
        if (safetyEventChecks[i]->isChecked() && safetyEventTimes[i]->value() > 0) {
            int eventTime = safetyEventTimes[i]->value();
            actionSequence.append(qMakePair(safetyEvents[i], eventTime));
            qDebug() << "Adding action:" << safetyEvents[i] << "with scheduled time:" << eventTime;
        }
    }

    std::sort(actionSequence.begin(), actionSequence.end(), [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
        return a.second < b.second;
    });

    qDebug() << "Passenger Name:" << passengerName;
    qDebug() << "Start Floor:" << startFloorSpinBox->value() << ", Destination Floor:" << destinationFloorSpinBox->value();
    qDebug() << "Actions for " << passengerName << ":";
    for (const auto &action : actionSequence) {
        qDebug() << "    Action:" << action.first << ", Scheduled Time:" << action.second;
    }

    Passenger *passenger = new Passenger(passengerName, startFloor, destinationFloor, actionSequence, ecs);
    passengers.append(passenger);
    logAction(timeStep, "Passenger Added");
    ui->passengerWidget->update();
    ui->passengerScroll->update();
}
//option to remove passengers
void MainWindow::removePassenger() {
    if (passengerInputs.size() > 1) {
        PassengerInput lastPassenger = passengerInputs.takeLast();
        passengerLayout->removeWidget(lastPassenger.rowWidget);
        lastPassenger.rowWidget->deleteLater();
        logAction(timeStep, "Passenger removed");
    } else {
        logAction(timeStep, "Cannot have less than one passenge");
    }
}
//inits the passengers  with all theeir actions
void MainWindow::initializePassengers() {
    qDeleteAll(passengers);
    passengers.clear();
    int passengerNumber = 1;

    for (const PassengerInput &input : passengerInputs) {
        int startFloor = input.startFloorSpinBox->value();
        int destinationFloor = input.destinationFloorSpinBox->value();
        QString passengerName = QString("Passenger %1").arg(passengerNumber++);
        QList<QPair<QString, int>> actionSequence;

        if (!input.actionTimeSpins.isEmpty()) {
            int pressButtonTime = input.actionTimeSpins[0]->value();
            actionSequence.append(qMakePair(QString("Press Button"), pressButtonTime));
        }

        if (input.openDoorsCheckBox->isChecked() && input.openDoorsTimeSpinBox->value() > 0) {
            int openTime = input.openDoorsTimeSpinBox->value();
            actionSequence.append(qMakePair(QString("Open Doors"), openTime));
            qDebug() << "Adding action: Open Doors at time:" << openTime;
        }
        if (input.closeDoorsCheckBox->isChecked() && input.closeDoorsTimeSpinBox->value() > 0) {
            int closeTime = input.closeDoorsTimeSpinBox->value();
            actionSequence.append(qMakePair(QString("Close Doors"), closeTime));
            qDebug() << "Adding action: Close Doors at time:" << closeTime;
        }

        QStringList safetyEvents = {"Obstructed", "Overloaded", "Help Alarm"};
        for (int i = 0; i < safetyEvents.size(); ++i) {
            if (input.safetyEventChecks[i]->isChecked() && input.safetyEventTimes[i]->value() > 0) {
                int eventTime = input.safetyEventTimes[i]->value();
                actionSequence.append(qMakePair(safetyEvents[i], eventTime));
                qDebug() << "Adding action:" << safetyEvents[i] << "at time:" << eventTime;
            }
        }

        qDebug() << "Initialized Passenger:" << passengerName;
        qDebug() << "Start Floor:" << startFloor << ", Destination Floor:" << destinationFloor;
        qDebug() << "Actions for " << passengerName << ":";
        for (const auto &action : actionSequence) {
            qDebug() << "Action:" << action.first << ", Scheduled Time:" << action.second;
        }

        Passenger *passenger = new Passenger(passengerName, startFloor, destinationFloor, actionSequence, ecs);
        passengers.append(passenger);
        connect(passenger, &Passenger::actionCompleted, this, &MainWindow::logAction);
    }
}
//start simulation called when the button is pressed
void MainWindow::startSimulation() {
    if (simulationStarted) return;
    logAction(timeStep, "Simulation started.");
    timeStep = 0;
    initializePassengers();
    simulationStarted = true;
    ui->passengerScroll->setVisible(false);
    ui->addPassenger->setVisible(false);
    ui->removePassenger->setVisible(false);
    ui->endSimulation->setVisible(true);
    ui->start->setVisible(false);
    ui->elevator1->setVisible(true);
    ui->elevator2->setVisible(true);
    ui->endSimulation->setVisible(true);
    ui->statusLabel1->setText("Status: Idle");
    ui->statusLabel2->setText("Status: Idle");
    ui->floorLabel1->setText("Current Floor: 1");
    ui->floorLabel2->setText("Current Floor: 1");
    simulationStarted = true;
    timeStep = 0;
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::simulateStep);
    timer->start(2000);
}
//simulates steps iin thesimulation to make thee GUI diaplay to the user
void MainWindow::simulateStep() {
    if (!simulationStarted && ecs->emergencyTriggered) {
        qDebug() << "Emergency triggered. Stopping simulation.";
        return;
    }

    for (Passenger *passenger : passengers) {
        if (!passenger->hasCompletedJourney()) {
            passenger->executeActionAtTime(timeStep);
        }
    }

    Elevator *currentElevator = (timeStep % 2 == 0) ? elevator1 : elevator2;
    Elevator::State currentState = currentElevator->getState();
    if (currentState == Elevator::HelpAlarm ||
        currentState == Elevator::Obstructed ||
        currentState == Elevator::PowerOutage) {
        logAction(timeStep, QString("Pausing for state: %1").arg(currentElevator->getStateString()));
        QTimer::singleShot(1000, this, &MainWindow::simulateStep);
        return;
    }

    currentElevator->moveToNextStop();
    if (currentElevator->getState() != lastElevatorState) {
        updateElevatorStatus(currentElevator->getID(), currentElevator->getStateString());
        lastElevatorState = currentElevator->getState();
    }

    QStringList passengerNames;
    for (Passenger *passenger : passengers) {
        if (passenger->getAssignedElevator() == currentElevator) {
            passengerNames.append(passenger->getName());
        }
    }

    timeStep++;
    QTimer::singleShot(1000, this, &MainWindow::simulateStep);
}
//ends the simulaation
void MainWindow::endSimulation() {
    if (!simulationStarted) return;
    logAction(timeStep, "Simulation ended.");

    simulationStarted = false;
    ui->elevator1->setVisible(false);
    ui->elevator2->setVisible(false);
    ui->endSimulation->setVisible(false);
    ui->passengerScroll->setVisible(true);
    ui->addPassenger->setVisible(true);
    ui->removePassenger->setVisible(true);
    ui->start->setVisible(true);

    for (const PassengerInput &input : passengerInputs) {
        passengerLayout->removeWidget(input.rowWidget);
        input.rowWidget->deleteLater();
    }
    passengerInputs.clear();
    addPassenger();
    ui->logWindow->clear();
    timeStep = 0;
}
//emergency functions to updaate the GUI
void MainWindow::handleFireEmergency() {
    ecs->emergencyTriggered = true;
    logAction(timeStep, "Fire Alarm triggered. All elevators moving to floor 1.");
    elevator1->updateState(Elevator::FireAlarm);
    elevator2->updateState(Elevator::FireAlarm);
}

void MainWindow::handlePowerOutageEmergency() {
    ecs->emergencyTriggered = true;
    logAction(timeStep, "Power Outage triggered. All elevators moving to floor 1.");
}
//update the status of the elevator (GUI)
void MainWindow::updateElevatorStatus(int elevatorId, const QString &status) {
    qDebug() << "Updating status for Elevator" << elevatorId << ": " << status;
    if (elevatorId == 1) {
        ui->statusLabel1->setText("Status: " + status);
    } else if (elevatorId == 2) {
        ui->statusLabel2->setText("Status: " + status);
    }
}
//update the elevator floor (GUI)
void MainWindow::updateElevatorFloor(int elevatorId, int floor) {
    qDebug() << "Update Elevator" << elevatorId << "Floor to:" << floor;
    if (elevatorId == 1) {
        ui->floorLabel1->setText("Current Floor: " + QString::number(floor));
    } else if (elevatorId == 2) {
        ui->floorLabel2->setText("Current Floor: " + QString::number(floor));
    }
}
//log actions to the log window on the GUI
void MainWindow::logAction(int timeStep, const QString &action) {
    QString logEntry = QString("Time Step %1: %2").arg(timeStep).arg(action);
    ui->logWindow->append(logEntry);
    qDebug() << logEntry;
}
