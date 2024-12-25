#ifndef ELEVATORCONTROLSYSTEM_H
#define ELEVATORCONTROLSYSTEM_H

#include "defs.h"
#include "Elevator.h"

class ElevatorControlSystem : public QObject {
    Q_OBJECT
public:
    //init
    explicit ElevatorControlSystem(const QList<Elevator*>& elevators);
    //ecs actions
    void requestElevator(int startFloor, int destinationFloor);
    Elevator* findNearestElevator(int floor, bool goingUp);
    void handleEmergency(const QString &emergencyType);
    void stopSimulationForAllElevators();
    //used to alert simulation of emergency
    bool emergencyTriggered = false;
    void assignQueuedRequests();

signals:
    //signals
    void elevatorAssigned(int elevatorId, int startFloor, int destinationFloor);

private:
    QList<Elevator*> elevators;
    QMap<Elevator*, QSet<int>> upRequests;
    QMap<Elevator*, QSet<int>> downRequests;
    QList<QPair<int, int>> unassignedRequests;
};

#endif
