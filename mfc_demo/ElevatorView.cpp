#include "framework.h"
#include "ElevatorView.h"
#include <algorithm>

extern HINSTANCE hInst;

#define WM_USER_ELEVATORVIEW (WM_USER + 100)

const WCHAR szElevatorViewClass[] = L"ElevatorViewClass";

ElevatorView::ElevatorView()
    : hWnd(NULL)
    , hParent(NULL)
    , pSimulator(nullptr)
    , isRunning(false)
    , timerId(0)
    , floorHeight(0)
    , elevatorWidth(0)
    , elevatorGap(0)
{
    ZeroMemory(&clientRect, sizeof(clientRect));
}

ElevatorView::~ElevatorView()
{
    Destroy();
}

static ATOM MyRegisterElevatorViewClass()
{
    static bool registered = false;
    if (registered)
        return TRUE;

    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = ElevatorView::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(LONG_PTR);
    wcex.hInstance = hInst;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szElevatorViewClass;
    wcex.hIconSm = NULL;

    registered = (RegisterClassExW(&wcex) != 0);
    return registered ? TRUE : FALSE;
}

BOOL ElevatorView::Create(HWND hParentWnd, ElevatorSimulator* pSim)
{
    if (!MyRegisterElevatorViewClass())
        return FALSE;

    hParent = hParentWnd;
    pSimulator = pSim;

    RECT rect;
    GetClientRect(hParent, &rect);

    hWnd = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        szElevatorViewClass,
        L"电梯运行仿真",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,
        hParent,
        NULL,
        hInst,
        (LPVOID)this);

    return (hWnd != NULL);
}

void ElevatorView::Destroy()
{
    if (isRunning)
    {
        StopSimulation();
    }
    if (hWnd)
    {
        DestroyWindow(hWnd);
        hWnd = NULL;
    }
}

void ElevatorView::Show(int nCmdShow)
{
    if (hWnd)
    {
        ShowWindow(hWnd, nCmdShow);
    }
}

void ElevatorView::StartSimulation()
{
    if (isRunning)
        return;
    isRunning = true;
    if (hWnd)
    {
        timerId = SetTimer(hWnd, 1, 500, NULL);
    }
}

void ElevatorView::StopSimulation()
{
    if (!isRunning)
        return;
    isRunning = false;
    if (hWnd && timerId)
    {
        KillTimer(hWnd, timerId);
        timerId = 0;
    }
}

ElevatorView* ElevatorView::GetThis(HWND hWnd)
{
    return (ElevatorView*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
}

LRESULT CALLBACK ElevatorView::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    ElevatorView* pThis = nullptr;

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        pThis = (ElevatorView*)pcs->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
    }
    else
    {
        pThis = GetThis(hWnd);
    }

    if (pThis)
    {
        switch (message)
        {
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hWnd, &ps);
                pThis->OnPaint(hdc);
                EndPaint(hWnd, &ps);
            }
            return 0;
        case WM_SIZE:
            {
                int width = LOWORD(lParam);
                int height = HIWORD(lParam);
                pThis->OnSize(width, height);
            }
            return 0;
        case WM_TIMER:
            pThis->OnTimer();
            return 0;
        case WM_LBUTTONDOWN:
            {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                pThis->OnLButtonDown(x, y);
            }
            return 0;
        case WM_DESTROY:
            pThis->StopSimulation();
            pThis->hWnd = NULL;
            return 0;
        }
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void ElevatorView::OnSize(int width, int height)
{
    clientRect.right = width;
    clientRect.bottom = height;

    if (pSimulator && pSimulator->numFloors > 0)
    {
        floorHeight = std::max(30, (height - 100) / pSimulator->numFloors);
    }
    if (pSimulator && pSimulator->numElevators > 0)
    {
        elevatorGap = 20;
        elevatorWidth = std::max(50, (width - 200 - elevatorGap * (pSimulator->numElevators - 1)) / pSimulator->numElevators);
        elevatorWidth = std::min(elevatorWidth, 100);
    }

    InvalidateRect(hWnd, NULL, TRUE);
}

void ElevatorView::OnPaint(HDC hdc)
{
    if (!pSimulator)
        return;

    DrawBuilding(hdc);
    DrawFloorButtons(hdc);
    DrawElevators(hdc);
    DrawElevatorButtons(hdc);
}

void ElevatorView::DrawBuilding(HDC hdc)
{
    if (!pSimulator)
        return;

    RECT buildingRect;
    buildingRect.left = 100;
    buildingRect.top = 50;
    buildingRect.right = buildingRect.left + elevatorWidth * pSimulator->numElevators + elevatorGap * (pSimulator->numElevators - 1) + 40;
    buildingRect.bottom = clientRect.bottom - 50;

    HBRUSH hBrush = CreateSolidBrush(RGB(240, 240, 240));
    FillRect(hdc, &buildingRect, hBrush);
    DeleteObject(hBrush);

    Rectangle(hdc, buildingRect.left, buildingRect.top, buildingRect.right, buildingRect.bottom);

    int totalHeight = buildingRect.bottom - buildingRect.top;
    floorHeight = totalHeight / pSimulator->numFloors;

    for (int i = 1; i <= pSimulator->numFloors; i++)
    {
        int y = buildingRect.bottom - i * floorHeight;
        MoveToEx(hdc, buildingRect.left, y, NULL);
        LineTo(hdc, buildingRect.right, y);

        WCHAR buf[16];
        swprintf_s(buf, L"%d", i);
        TextOut(hdc, 60, y + floorHeight / 2 - 8, buf, (int)wcslen(buf));
    }
}

void ElevatorView::DrawElevators(HDC hdc)
{
    if (!pSimulator)
        return;

    RECT buildingRect;
    buildingRect.left = 100;
    buildingRect.top = 50;
    buildingRect.right = buildingRect.left + elevatorWidth * pSimulator->numElevators + elevatorGap * (pSimulator->numElevators - 1) + 40;
    buildingRect.bottom = clientRect.bottom - 50;

    elevatorWidth = 60;
    elevatorGap = 10;

    for (size_t i = 0; i < pSimulator->elevators.size(); i++)
    {
        const Elevator& e = pSimulator->elevators[i];
        int x = buildingRect.left + 20 + (int)i * (elevatorWidth + elevatorGap);

        HBRUSH hShaftBrush = CreateSolidBrush(RGB(180, 180, 180));
        RECT shaftRect;
        shaftRect.left = x;
        shaftRect.top = buildingRect.top;
        shaftRect.right = x + elevatorWidth;
        shaftRect.bottom = buildingRect.bottom;
        FillRect(hdc, &shaftRect, hShaftBrush);
        DeleteObject(hShaftBrush);
        Rectangle(hdc, shaftRect.left, shaftRect.top, shaftRect.right, shaftRect.bottom);

        int elevatorY = buildingRect.bottom - e.currentFloor * floorHeight;
        RECT elevatorRect;
        elevatorRect.left = x + 5;
        elevatorRect.top = elevatorY + 5;
        elevatorRect.right = x + elevatorWidth - 5;
        elevatorRect.bottom = elevatorY + floorHeight - 5;

        HBRUSH hElevatorBrush = CreateSolidBrush(RGB(100, 180, 255));
        FillRect(hdc, &elevatorRect, hElevatorBrush);
        DeleteObject(hElevatorBrush);
        Rectangle(hdc, elevatorRect.left, elevatorRect.top, elevatorRect.right, elevatorRect.bottom);

        WCHAR buf[32];
        swprintf_s(buf, L"%d", e.id + 1);
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkMode(hdc, TRANSPARENT);
        TextOut(hdc, elevatorRect.left + 20, elevatorRect.top + 5, buf, (int)wcslen(buf));

        swprintf_s(buf, L"%dF", e.currentFloor);
        TextOut(hdc, elevatorRect.left + 10, elevatorRect.top + floorHeight / 2 - 8, buf, (int)wcslen(buf));

        SetTextColor(hdc, RGB(0, 0, 0));
    }
}

void ElevatorView::DrawFloorButtons(HDC hdc)
{
    if (!pSimulator)
        return;

    RECT buildingRect;
    buildingRect.left = 100;
    buildingRect.top = 50;
    buildingRect.right = buildingRect.left + elevatorWidth * pSimulator->numElevators + elevatorGap * (pSimulator->numElevators - 1) + 40;
    buildingRect.bottom = clientRect.bottom - 50;

    int btnWidth = 20;
    int btnHeight = 15;
    int btnX = buildingRect.right + 10;

    for (int i = 1; i <= pSimulator->numFloors; i++)
    {
        int floorY = buildingRect.bottom - i * floorHeight;
        int centerY = floorY + floorHeight / 2;

        if (i < pSimulator->numFloors)
        {
            RECT upBtn;
            upBtn.left = btnX;
            upBtn.top = centerY - btnHeight - 3;
            upBtn.right = btnX + btnWidth;
            upBtn.bottom = centerY - 3;

            HBRUSH hBrush = CreateSolidBrush(RGB(255, 200, 100));
            FillRect(hdc, &upBtn, hBrush);
            DeleteObject(hBrush);
            Rectangle(hdc, upBtn.left, upBtn.top, upBtn.right, upBtn.bottom);

            TextOut(hdc, upBtn.left + 6, upBtn.top + 1, L"^", 1);
        }

        if (i > 1)
        {
            RECT downBtn;
            downBtn.left = btnX;
            downBtn.top = centerY + 3;
            downBtn.right = btnX + btnWidth;
            downBtn.bottom = centerY + btnHeight + 3;

            HBRUSH hBrush = CreateSolidBrush(RGB(255, 200, 100));
            FillRect(hdc, &downBtn, hBrush);
            DeleteObject(hBrush);
            Rectangle(hdc, downBtn.left, downBtn.top, downBtn.right, downBtn.bottom);

            TextOut(hdc, downBtn.left + 6, downBtn.top + 1, L"v", 1);
        }
    }
}

void ElevatorView::DrawElevatorButtons(HDC hdc)
{
    if (!pSimulator)
        return;

    int panelX = clientRect.right - 200;
    int panelY = 50;
    int panelWidth = 150;

    for (size_t e = 0; e < pSimulator->elevators.size(); e++)
    {
        const Elevator& elevator = pSimulator->elevators[e];

        WCHAR buf[32];
        swprintf_s(buf, L"电梯 %d 内部按键", (int)e + 1);
        TextOut(hdc, panelX, panelY, buf, (int)wcslen(buf));
        panelY += 25;

        int cols = 5;
        int btnSize = 24;
        int gap = 5;

        for (int i = 0; i < pSimulator->numFloors; i++)
        {
            int floor = pSimulator->numFloors - i;
            int col = i % cols;
            int row = i / cols;

            int btnX = panelX + col * (btnSize + gap);
            int btnY = panelY + row * (btnSize + gap);

            RECT btnRect;
            btnRect.left = btnX;
            btnRect.top = btnY;
            btnRect.right = btnX + btnSize;
            btnRect.bottom = btnY + btnSize;

            COLORREF color = elevator.floorButtons[floor] ? RGB(255, 100, 100) : RGB(200, 255, 200);
            HBRUSH hBrush = CreateSolidBrush(color);
            FillRect(hdc, &btnRect, hBrush);
            DeleteObject(hBrush);
            Rectangle(hdc, btnRect.left, btnRect.top, btnRect.right, btnRect.bottom);

            swprintf_s(buf, L"%d", floor);
            SetBkMode(hdc, TRANSPARENT);
            TextOut(hdc, btnRect.left + 5, btnRect.top + 3, buf, (int)wcslen(buf));
        }

        panelY += ((pSimulator->numFloors + cols - 1) / cols) * (btnSize + gap) + 20;
    }
}

void ElevatorView::OnTimer()
{
    if (pSimulator && isRunning)
    {
        pSimulator->Update();
        InvalidateRect(hWnd, NULL, FALSE);
    }
}

void ElevatorView::OnLButtonDown(int x, int y)
{
    if (!pSimulator)
        return;

    int floor;
    ElevatorDirection direction;

    if (HitTestFloorButton(x, y, floor, direction) >= 0)
    {
        pSimulator->RequestElevator(floor, direction);
        InvalidateRect(hWnd, NULL, FALSE);
        return;
    }

    int elevatorId;
    if (HitTestElevatorButton(x, y, elevatorId, floor) >= 0)
    {
        pSimulator->PressElevatorButton(elevatorId, floor);
        InvalidateRect(hWnd, NULL, FALSE);
        return;
    }
}

int ElevatorView::HitTestFloorButton(int x, int y, int& floor, ElevatorDirection& direction)
{
    if (!pSimulator)
        return -1;

    RECT buildingRect;
    buildingRect.left = 100;
    buildingRect.top = 50;
    buildingRect.right = buildingRect.left + 60 * pSimulator->numElevators + 10 * (pSimulator->numElevators - 1) + 40;
    buildingRect.bottom = clientRect.bottom - 50;

    int totalHeight = buildingRect.bottom - buildingRect.top;
    floorHeight = totalHeight / pSimulator->numFloors;

    int btnWidth = 20;
    int btnHeight = 15;
    int btnX = buildingRect.right + 10;

    if (x < btnX || x > btnX + btnWidth)
        return -1;

    for (int i = 1; i <= pSimulator->numFloors; i++)
    {
        int floorY = buildingRect.bottom - i * floorHeight;
        int centerY = floorY + floorHeight / 2;

        if (i < pSimulator->numFloors)
        {
            int upTop = centerY - btnHeight - 3;
            int upBottom = centerY - 3;
            if (y >= upTop && y <= upBottom)
            {
                floor = i;
                direction = ElevatorDirection::UP;
                return 0;
            }
        }

        if (i > 1)
        {
            int downTop = centerY + 3;
            int downBottom = centerY + btnHeight + 3;
            if (y >= downTop && y <= downBottom)
            {
                floor = i;
                direction = ElevatorDirection::DOWN;
                return 1;
            }
        }
    }

    return -1;
}

int ElevatorView::HitTestElevatorButton(int x, int y, int& elevatorId, int& floor)
{
    if (!pSimulator)
        return -1;

    int panelX = clientRect.right - 200;
    int panelY = 50;

    if (x < panelX)
        return -1;

    int cols = 5;
    int btnSize = 24;
    int gap = 5;

    for (size_t e = 0; e < pSimulator->elevators.size(); e++)
    {
        int titleHeight = 25;
        int panelHeight = ((pSimulator->numFloors + cols - 1) / cols) * (btnSize + gap) + 20;

        if (y < panelY || y > panelY + titleHeight + panelHeight)
        {
            panelY += titleHeight + panelHeight;
            continue;
        }

        int btnAreaY = panelY + titleHeight;

        int localX = x - panelX;
        int localY = y - btnAreaY;

        if (localX < 0 || localY < 0)
        {
            panelY += titleHeight + panelHeight;
            continue;
        }

        int col = localX / (btnSize + gap);
        int row = localY / (btnSize + gap);

        if (col >= cols || col < 0)
        {
            panelY += titleHeight + panelHeight;
            continue;
        }

        int idx = row * cols + col;
        if (idx < 0 || idx >= pSimulator->numFloors)
        {
            panelY += titleHeight + panelHeight;
            continue;
        }

        int btnX = col * (btnSize + gap);
        int btnY = row * (btnSize + gap);

        if (localX >= btnX && localX < btnX + btnSize &&
            localY >= btnY && localY < btnY + btnSize)
        {
            elevatorId = (int)e;
            floor = pSimulator->numFloors - idx;
            return 0;
        }

        panelY += titleHeight + panelHeight;
    }

    return -1;
}
