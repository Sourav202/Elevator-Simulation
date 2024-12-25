#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "defs.h"
#include "Passenger.h"
#include "Elevator.h"
#include "ElevatorControlSystem.h"

namespace Ui {
class MainWindow;
}
//GUI elements for passenger input
struct PassengerInput {
    QSpinBox *startFloorSpinBox;
    QSpinBox *destinationFloorSpinBox;
    QList<QSpinBox*> actionTimeSpins;
    QCheckBox *openDoorsCheckBox;
    QSpinBox *openDoorsTimeSpinBox;
    QCheckBox *closeDoorsCheckBox;
    QSpinBox *closeDoorsTimeSpinBox;
    QList<QCheckBox*> safetyEventChecks;
    QList<QSpinBox*> safetyEventTimes;
    QWidget *rowWidget;
};
//application window for controlling simulation
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    //constructor and destructor
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //logs simulation actions in the GUI
    void logAction(int time, const QString &action);

private slots:
    //control slots
    void startSimulation();
    void simulateStep();
    void endSimulation();
    //add N passengers to simulation
    void addPassenger();
    void removePassenger();
    //emergency GUI response
    void handleFireEmergency();
    void handlePowerOutageEmergency();
    //elevator GUI visuals
    void updateElevatorStatus(int elevatorId, const QString &status);
    void updateElevatorFloor(int elevatorId, int floor);

private:
    Ui::MainWindow *ui;
    //passenger inputs and configs
    QVBoxLayout *passengerLayout;
    QList<Passenger*> passengers;
    QList<PassengerInput> passengerInputs;
    int timeStep;
    bool simulationStarted;
    Elevator::State lastElevatorState;

    Elevator *elevator1;
    Elevator *elevator2;
    ElevatorControlSystem *ecs;
    //utility functions to connect signals (to GUI) and init the simulation
    void connectElevatorSignals();
    void connectUIButtons();
    void initializeUI();
    void initializePassengers();
    bool allPassengersAtDestinations() const;
};

#endif
