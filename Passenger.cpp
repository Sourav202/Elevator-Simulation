#include "Passenger.h"
#include "ElevatorControlSystem.h"
#include "Elevator.h"
//constructor
Passenger::Passenger(const QString &name, int startFloor, int destinationFloor, const QList<QPair<QString, int>> &action, ElevatorControlSystem *ecs)
    : name(name), startFloor(startFloor), destinationFloor(destinationFloor),
      assignedElevator(nullptr), ecs(ecs), journeyCompleted(false),
      action(action), currActionIndex(0), actionDelay(2000) {

    actionTimer = new QTimer(this);
}

//execute a passenger actions at a scheduled time
void Passenger::executeActionAtTime(int timeStep) {
    if (currActionIndex >= action.size() || journeyCompleted) return;

    QPair<QString, int> currentAction = action[currActionIndex];
    qDebug() << "Checking action:" << currentAction.first
             << "at Time Step:" << timeStep
             << "Scheduled at Time:" << currentAction.second;

    if (currentAction.second == timeStep) {
        qDebug() << "Executing action:" << currentAction.first << "at Time Step:" << timeStep;
        executeAction(currentAction.first, timeStep);
        currActionIndex++;
    }
}
//executes a specific action and logs the action
void Passenger::executeAction(const QString& action, int timeStep) {
    QString logMessage;

    if (action == "Press Button") {
        pressButton();
        QString direction = (destinationFloor > startFloor) ? "Up" : "Down";
        logMessage = QString("Passenger presses button on floor %1, lighting up the button, requesting to go %2").arg(startFloor).arg(direction);

    } else if (action == "Obstructed" && assignedElevator) {
        assignedElevator->handleObstructed();
        logMessage = "Elevator is obstructed.";

    } else if (action == "Overloaded" && assignedElevator) {
        assignedElevator->handleOverloaded();
        logMessage = "Elevator is overloaded.";

    } else if (action == "Help" && assignedElevator) {
        assignedElevator->handleHelp();
        logMessage = "Help alarm activated.";

    } else if (action == "Open Doors" && assignedElevator) {
        assignedElevator->openDoors();
        logMessage = "Passenger opens elevator doors.";

    } else if (action == "Close Doors" && assignedElevator) {
        assignedElevator->closeDoors();
        logMessage = "Passenger closes elevator doors.";
    }

    if (!logMessage.isEmpty()) {
        emit actionCompleted(timeStep, logMessage);
        qDebug() << logMessage;
    }
}
//passenger behaviours
void Passenger::pressButton() {
    qDebug() << "Passenger on floor" << startFloor << "requests elevator";
    ecs->requestElevator(startFloor, destinationFloor);
}

void Passenger::enterElevator() {
    if (assignedElevator) {
        qDebug() << "Passenger enters elevator at floor" << startFloor;
        emit passengerEnteredElevator(name, assignedElevator->getID());
    }
}

void Passenger::exitElevator() {
    if (assignedElevator) {
        qDebug() << "Passenger exited the elevator at floor" << destinationFloor;
        emit passengerExitedElevator(name, assignedElevator->getID());
        assignedElevator = nullptr;
        journeyCompleted = true;
    } else {
        qDebug() << "err: No elevator assigned.";
    }
}
//utiility functions
bool Passenger::hasCompletedJourney() const {
    return journeyCompleted;
}

int Passenger::getStartFloor() const {
    return startFloor;
}

int Passenger::getDestinationFloor() const {
    return destinationFloor;
}

Elevator* Passenger::getAssignedElevator() const {
    return assignedElevator;
}

bool Passenger::hasElevatorAssigned() const { return
            assignedElevator != nullptr;
}

void Passenger::setAssignedElevator(Elevator *elevator) {
    assignedElevator = elevator;
}

QString Passenger::getName() const {
    return name;
}
