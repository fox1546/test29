#include "framework.h"
#include "ElevatorSim.h"
#include <algorithm>
#include <climits>

Elevator::Elevator(int id, int numFloors)
    : id(id)
    , currentFloor(1)
    , targetFloor(-1)
    , direction(ElevatorDirection::IDLE)
    , doorState(DoorState::CLOSED)
{
    floorButtons.resize(numFloors + 1, false);
}

void Elevator::Update()
{
    if (doorState != DoorState::CLOSED)
    {
        return;
    }

    if (direction == ElevatorDirection::IDLE)
    {
        if (!destinationQueue.empty())
        {
            targetFloor = destinationQueue.front();
            destinationQueue.pop();

            if (targetFloor > currentFloor)
            {
                direction = ElevatorDirection::UP;
            }
            else if (targetFloor < currentFloor)
            {
                direction = ElevatorDirection::DOWN;
            }
        }
        return;
    }

    if (direction == ElevatorDirection::UP)
    {
        currentFloor++;
        if (currentFloor >= targetFloor)
        {
            currentFloor = targetFloor;
            direction = ElevatorDirection::IDLE;
            targetFloor = -1;
            floorButtons[currentFloor] = false;
        }
    }
    else if (direction == ElevatorDirection::DOWN)
    {
        currentFloor--;
        if (currentFloor <= targetFloor)
        {
            currentFloor = targetFloor;
            direction = ElevatorDirection::IDLE;
            targetFloor = -1;
            floorButtons[currentFloor] = false;
        }
    }
}

void Elevator::AddDestination(int floor)
{
    if (floor < 1 || floor > (int)floorButtons.size() - 1)
        return;

    floorButtons[floor] = true;

    bool exists = false;
    std::queue<int> temp = destinationQueue;
    while (!temp.empty())
    {
        if (temp.front() == floor)
        {
            exists = true;
            break;
        }
        temp.pop();
    }

    if (!exists && floor != currentFloor)
    {
        destinationQueue.push(floor);
    }
}

bool Elevator::HasPendingRequests() const
{
    return direction != ElevatorDirection::IDLE || !destinationQueue.empty();
}

ElevatorSimulator::ElevatorSimulator()
    : numElevators(1)
    , numFloors(10)
    , linkedOperation(false)
{
}

void ElevatorSimulator::Initialize(int elevators, int floors, bool linked)
{
    numElevators = elevators;
    numFloors = floors;
    linkedOperation = linked;

    elevators.clear();
    for (int i = 0; i < numElevators; i++)
    {
        elevators.emplace_back(i, numFloors);
    }

    pendingRequests.clear();
}

void ElevatorSimulator::Update()
{
    for (auto& elevator : elevators)
    {
        elevator.Update();
    }

    for (auto it = pendingRequests.begin(); it != pendingRequests.end();)
    {
        int bestElevator = FindBestElevator(it->floor, it->direction);
        if (bestElevator >= 0)
        {
            elevators[bestElevator].AddDestination(it->floor);
            it = pendingRequests.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void ElevatorSimulator::RequestElevator(int floor, ElevatorDirection direction)
{
    if (floor < 1 || floor > numFloors)
        return;

    if (linkedOperation)
    {
        pendingRequests.emplace_back(floor, direction);
    }
    else
    {
        for (auto& elevator : elevators)
        {
            elevator.AddDestination(floor);
        }
    }
}

void ElevatorSimulator::PressElevatorButton(int elevatorId, int floor)
{
    if (elevatorId < 0 || elevatorId >= (int)elevators.size())
        return;
    if (floor < 1 || floor > numFloors)
        return;

    elevators[elevatorId].AddDestination(floor);
}

int ElevatorSimulator::FindBestElevator(int floor, ElevatorDirection direction) const
{
    int bestElevator = -1;
    int minDistance = INT_MAX;

    for (size_t i = 0; i < elevators.size(); i++)
    {
        const Elevator& e = elevators[i];

        int distance = abs(e.currentFloor - floor);

        if (e.direction == ElevatorDirection::IDLE)
        {
            if (distance < minDistance)
            {
                minDistance = distance;
                bestElevator = (int)i;
            }
        }
        else if (e.direction == direction)
        {
            if ((direction == ElevatorDirection::UP && floor >= e.currentFloor) ||
                (direction == ElevatorDirection::DOWN && floor <= e.currentFloor))
            {
                if (distance < minDistance)
                {
                    minDistance = distance;
                    bestElevator = (int)i;
                }
            }
        }
    }

    if (bestElevator == -1 && !elevators.empty())
    {
        bestElevator = 0;
        minDistance = abs(elevators[0].currentFloor - floor);
        for (size_t i = 1; i < elevators.size(); i++)
        {
            int distance = abs(elevators[i].currentFloor - floor);
            if (distance < minDistance)
            {
                minDistance = distance;
                bestElevator = (int)i;
            }
        }
    }

    return bestElevator;
}
