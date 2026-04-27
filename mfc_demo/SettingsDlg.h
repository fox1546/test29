#pragma once

#include "framework.h"
#include "resource.h"

class SettingsDlg
{
public:
    int numElevators;
    int numFloors;
    bool linkedOperation;

    SettingsDlg();
    INT_PTR DoModal(HWND hParent);

private:
    static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    BOOL OnInitDialog(HWND hDlg);
    BOOL OnOK(HWND hDlg);
};
