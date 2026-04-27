#pragma once

#include <vector>
#include <queue>

enum class ElevatorDirection
{
    IDLE,
    UP,
    DOWN
};

enum class DoorState
{
    CLOSED,
    OPENING,
    OPEN,
    CLOSING
};

struct ElevatorRequest
{
    int floor;
    ElevatorDirection direction;
    bool fromInside;

    ElevatorRequest(int f, ElevatorDirection d, bool inside = false)
        : floor(f), direction(d), fromInside(inside) {}
};

class Elevator
{
public:
    int id;
    int currentFloor;
    int targetFloor;
    ElevatorDirection direction;
    DoorState doorState;
    std::vector<bool> floorButtons;
    std::queue<int> destinationQueue;

    Elevator(int id, int numFloors);
    void Update();
    void AddDestination(int floor);
    bool HasPendingRequests() const;
};

class ElevatorSimulator
{
public:
    int numElevators;
    int numFloors;
    bool linkedOperation;
    std::vector<Elevator> elevators;
    std::vector<ElevatorRequest> pendingRequests;

    ElevatorSimulator();
    void Initialize(int numElevators, int numFloors, bool linked);
    void Update();
    void RequestElevator(int floor, ElevatorDirection direction);
    void PressElevatorButton(int elevatorId, int floor);
    int FindBestElevator(int floor, ElevatorDirection direction) const;
};
