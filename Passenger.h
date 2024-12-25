#ifndef PASSENGER_H
#define PASSENGER_H

#include "defs.h"
#include "ElevatorControlSystem.h"

class Passenger : public QObject {
    Q_OBJECT
public:
    Passenger(const QString &name, int startFloor, int destinationFloor, const QList<QPair<QString, int>>& actionSequence, ElevatorControlSystem *ecs);

    //scheduled action based on time step
    void executeActionAtTime(int timeStep);

    //getters
    int getStartFloor() const;
    int getDestinationFloor() const;
    Elevator* getAssignedElevator() const;
    QString getName() const;

    //elevator and passenger interaction
    bool hasElevatorAssigned() const;
    void setAssignedElevator(Elevator *elevator);
    bool hasCompletedJourney() const;
    void resetActions();
    //actions for passenger movement and status
    void executeAction(const QString& action, int timeStep);
    void pressButton();
    void enterElevator();
    void exitElevator();

signals:
    //signals
    void actionCompleted(int time, QString action);
    void passengerEnteredElevator(const QString &passengerName, int elevatorId);
    void passengerExitedElevator(const QString &name, int elevatorId);

private:
    QString name;
    int startFloor;
    int destinationFloor;
    Elevator *assignedElevator;
    ElevatorControlSystem* ecs;
    bool journeyCompleted;
    QTimer* actionTimer;
    int actionDelay;
    //actions with timing
    QList<QPair<QString, int>> action;
    int currActionIndex;
};

#endif
