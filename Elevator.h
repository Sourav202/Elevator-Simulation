#ifndef ELEVATOR_H
#define ELEVATOR_H
#include "defs.h"

class MainWindow;
class Passenger;
class ElevatorControlSystem;

class Elevator : public QObject {
    Q_OBJECT
public:
    //elevator states
    enum State { Idle, MovingUp, MovingDown, Obstructed, Overloaded, FireAlarm, HelpAlarm, PowerOutage, Open };
    //init
    explicit Elevator(int id, MainWindow *mainWindow, ElevatorControlSystem* ecs);
    void setECS(ElevatorControlSystem* ecs);
    //elevator actions
    void moveToFloor(int floor);
    void addStop(int floor);
    void moveToNextStop();
    void pickUpPassenger(Passenger *passenger);
    void dropOffPassenger(Passenger *passenger);
    void openDoors();
    void closeDoors();
    void resetToIdleState();
    void updateDirection();
    QString getStateString() const;

    //elevator emergency actions
    void clearStops();
    void handleFire();
    void handleHelp();
    void handlePower();
    void handleEmergency();
    void handleObstructed();
    void handleOverloaded();

    //util functions
    void updateState(State newState);
    int getID() const;
    int getCurrentFloor() const;
    bool isIdle() const;
    State getState() const;
    QString getDirection() const;

signals:
    //signals
    void arrivedAtFloor(int elevatorId, int floor);
    void doorsOpened(int elevatorId);
    void doorsClosed(int elevatorId);
    void fireAlarmTriggered(int elevatorId);
    void helpButtonPressed(int elevatorId);
    void powerOutageHandled(int elevatorId);
    void stateChanged(int elevatorId, const QString &state);
    void floorChanged(int elevatorId, int floor);

private:
    int id;
    int startFloor;
    int targetFloor;
    bool idle;
    State state;
    MainWindow *mainWindow;
    ElevatorControlSystem* ecs;
    QList<Passenger*> passengers;

    QSet<int> upStops;
    QSet<int> downStops;
};

#endif
