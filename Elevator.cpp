#include "defs.h"
#include "Elevator.h"
#include "mainwindow.h"
#include "Passenger.h"
//constructor
Elevator::Elevator(int id, MainWindow *mainWindow, ElevatorControlSystem* ecs)
    : id(id), startFloor(1), targetFloor(-1), idle(true), state(Idle), mainWindow(mainWindow), ecs(ecs) {}
//move to specified floor
void Elevator::moveToFloor(int floor) {
    qDebug() << "Elevator" << id << "moving to floor:" << floor << ", from floor" << startFloor;

    if (ecs->emergencyTriggered) {
        qDebug() << "Emergency triggered-Elevator" << id << "is idle";
        return;
    }

    if (startFloor != floor) {
        startFloor = floor;
        emit floorChanged(id, startFloor);
        openDoors();
    }
}
//add floor stop
void Elevator::addStop(int floor) {
    qDebug() << "Elevator" << id << "adding stop at floor:" << floor;

    if (floor > startFloor) {
        upStops.insert(floor);
        if (state == Idle) updateState(MovingUp);
    } else if (floor < startFloor) {
        downStops.insert(floor);
        if (state == Idle) updateState(MovingDown);
    }
}
//move to next floor in list (check for emergencies)
void Elevator::moveToNextStop() {
    if (state == FireAlarm || state == Obstructed || state == PowerOutage) return;

    int nextFloor = -1;
    if (state == MovingUp && !upStops.isEmpty()) {
        nextFloor = *upStops.begin();
        upStops.remove(nextFloor);
    } else if (state == MovingDown && !downStops.isEmpty()) {
        nextFloor = *downStops.begin();
        downStops.remove(nextFloor);
    }

    if (nextFloor != -1) moveToFloor(nextFloor);
    else updateDirection();
}

void Elevator::setECS(ElevatorControlSystem* ecs) {
    this->ecs = ecs;
}
//update direction elevator is travelling (changes state)
void Elevator::updateDirection() {
    if (!upStops.isEmpty()) updateState(MovingUp);
    else if (!downStops.isEmpty()) updateState(MovingDown);
    else updateState(Idle);
}
//open doors (changes state)
void Elevator::openDoors() {
    if (ecs->emergencyTriggered) {
        qDebug() << "Elevator" << id << "doors opening due to emergency";
        emit doorsOpened(id);
        return;
    }
    qDebug() << "Elevator rings bell";
    qDebug() << "Elevator" << id << "doors opening at floor" << startFloor;
    updateState(Open);
    emit doorsOpened(id);
}
//close doors
void Elevator::closeDoors() {
    qDebug() << "Elevator rings bell";
    qDebug() << "Elevator" << id << "doors closing";
    updateState(Idle);
    emit doorsClosed(id);
}
//keep track of passengers
void Elevator::pickUpPassenger(Passenger *passenger) {
    passengers.append(passenger);
    closeDoors();
}

void Elevator::dropOffPassenger(Passenger *passenger) {
    passengers.removeOne(passenger);
    closeDoors();
}
//safety & emergency protocols
void Elevator::handleObstructed() {
    qDebug() << "Elevator" << id << "is obstructed!";
    updateState(Obstructed);
    QTimer::singleShot(3000, this, [this]() {
        if (state == Obstructed) {
            updateState(Idle);
            moveToNextStop();
        }
    });
}

void Elevator::handleOverloaded() {
    qDebug() << "Elevator" << id << "is overloaded!";
    updateState(Overloaded);
    QTimer::singleShot(3000, this, [this]() {
        if (state == Overloaded) {
            updateState(Idle);
            moveToNextStop();
        }
    });
}

void Elevator::handleHelp() {
    qDebug() << "Help alarm activated for Elevator" << id;
    updateState(HelpAlarm);
    emit helpButtonPressed(id);
    QTimer::singleShot(5000, this, [this]() {
        if (state == HelpAlarm) {
            updateState(Idle);
            moveToNextStop();
        }
    });
}

void Elevator::handleFire() {
    updateState(FireAlarm);
    handleEmergency();
}

void Elevator::handlePower() {
    updateState(PowerOutage);
    handleEmergency();
}

void Elevator::handleEmergency() {
    if (startFloor != 1) moveToFloor(1);
    else openDoors();
}
//clear stops for emergency
void Elevator::clearStops() {
    upStops.clear();
    downStops.clear();
}
//utility functions
int Elevator::getID() const {
    return id;
}

int Elevator::getCurrentFloor() const {
    return startFloor;
}

bool Elevator::isIdle() const {
    return state == Idle;
}

Elevator::State Elevator::getState() const {
    return state;
}

QString Elevator::getDirection() const {
    return state == MovingUp ? "Up" : (state == MovingDown ? "Down" : "Idle");
}

QString Elevator::getStateString() const {
    switch (state) {
        case Idle: return "Idle";
        case MovingUp: return "Moving Up";
        case MovingDown: return "Moving Down";
        case Obstructed: return "Obstructed";
        case FireAlarm: return "Fire Alarm";
        case HelpAlarm: return "Help Alarm";
        case PowerOutage: return "Power Outage";
        case Open: return "Open";
        default: return "Unknown";
    }
}

void Elevator::updateState(State newState) {
    if (state != newState) {
        state = newState;
        emit stateChanged(id, getStateString());
    }
}
