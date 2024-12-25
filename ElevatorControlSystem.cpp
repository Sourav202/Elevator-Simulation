#include "ElevatorControlSystem.h"
//constructor
ElevatorControlSystem::ElevatorControlSystem(const QList<Elevator*>& elevators)
    : elevators(elevators), emergencyTriggered(false) {

    //queue and assign unassigned requests based on a timer
    QTimer* queue = new QTimer(this);
    connect(queue, &QTimer::timeout, this, &ElevatorControlSystem::assignQueuedRequests);
    queue->start(1000);
}

//find nearest elevatorr and assign send it
Elevator* ElevatorControlSystem::findNearestElevator(int floor, bool goingUp) {
    Elevator* closestElevator = nullptr;
    int closestDistance = 99999;

    for (Elevator* elevator : elevators) {
        int distance = std::abs(elevator->getCurrentFloor() - floor);
        bool correctDirection = elevator->isIdle() ||
                                (goingUp && elevator->getDirection() == "Up" && elevator->getCurrentFloor() <= floor) ||
                                (!goingUp && elevator->getDirection() == "Down" && elevator->getCurrentFloor() >= floor);

        if (correctDirection && distance < closestDistance) {
            closestElevator = elevator;
            closestDistance = distance;
        }
    }

    if (closestElevator) {
        qDebug() << "Assigned Elevator:" << closestElevator->getID() << "for floor" << floor;
    } else {
        qDebug() << "No available elevator found..." << floor;
    }

    return closestElevator;
}
//a queue to make sure all passenger requests geet fufilled
void ElevatorControlSystem::assignQueuedRequests() {
    auto passengerRequests = unassignedRequests.begin();

    while (passengerRequests != unassignedRequests.end()) {
        int startFloor = passengerRequests->first;
        int destinationFloor = passengerRequests->second;
        bool goingUp = destinationFloor > startFloor;

        Elevator* nearestElevator = findNearestElevator(startFloor, goingUp);

        if (nearestElevator) {
            nearestElevator->addStop(startFloor);
            nearestElevator->addStop(destinationFloor);
            emit elevatorAssigned(nearestElevator->getID(), startFloor, destinationFloor);
            passengerRequests = unassignedRequests.erase(passengerRequests);
        } else {
            ++passengerRequests;
        }
    }
}

//tells an elevator to floor and handle a passenger's request
void ElevatorControlSystem::requestElevator(int startFloor, int destinationFloor) {
    qDebug() << "Requesting elevator for floor:" << startFloor;

    bool goingUp = destinationFloor > startFloor;
    Elevator* nearestElevator = findNearestElevator(startFloor, goingUp);

    if (nearestElevator) {
        nearestElevator->addStop(startFloor);
        nearestElevator->addStop(destinationFloor);
        emit elevatorAssigned(nearestElevator->getID(), startFloor, destinationFloor);
    } else {
        //add to queue if no elevator is available
        qDebug() << "No elevator available. Queuing request from floor" << startFloor << "to floor" << destinationFloor;
        unassignedRequests.append(qMakePair(startFloor, destinationFloor));
    }
}


//handles all emergencies for all elevators
void ElevatorControlSystem::handleEmergency(const QString &emergencyType) {
    emergencyTriggered = true;
    qDebug() << "Emergency :" << emergencyType << ". Moving all elevators to a safe floor!";

    for (Elevator* elevator : elevators) {
        if (emergencyType == "FireAlarm") {
            elevator->updateState(Elevator::FireAlarm);
        } else if (emergencyType == "PowerOutage") {
            elevator->updateState(Elevator::PowerOutage);
        }
       //safety protocol for this system is defined as going to first floor and opening doors for passengers to exit
        elevator->moveToFloor(1);
        elevator->openDoors();
    }
}
