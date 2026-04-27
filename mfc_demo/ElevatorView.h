#pragma once

#include "framework.h"
#include "ElevatorSim.h"

class ElevatorView
{
public:
    HWND hWnd;
    HWND hParent;
    ElevatorSimulator* pSimulator;
    bool isRunning;
    UINT_PTR timerId;

    RECT clientRect;
    int floorHeight;
    int elevatorWidth;
    int elevatorGap;

    ElevatorView();
    ~ElevatorView();

    BOOL Create(HWND hParentWnd, ElevatorSimulator* pSim);
    void Destroy();
    void Show(int nCmdShow);
    void StartSimulation();
    void StopSimulation();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    static ElevatorView* GetThis(HWND hWnd);
    void OnPaint(HDC hdc);
    void OnSize(int width, int height);
    void OnTimer();
    void OnLButtonDown(int x, int y);
    void DrawBuilding(HDC hdc);
    void DrawElevators(HDC hdc);
    void DrawFloorButtons(HDC hdc);
    void DrawElevatorButtons(HDC hdc);
    int GetElevatorX(int elevatorIndex);
    int GetFloorY(int floor);
    int HitTestFloorButton(int x, int y, int& floor, ElevatorDirection& direction);
    int HitTestElevatorButton(int x, int y, int& elevatorId, int& floor);
};
