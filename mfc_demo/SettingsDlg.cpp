#include "framework.h"
#include "SettingsDlg.h"
#include <commctrl.h>

SettingsDlg::SettingsDlg()
    : numElevators(1)
    , numFloors(10)
    , linkedOperation(false)
{
}

INT_PTR SettingsDlg::DoModal(HWND hParent)
{
    return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SETTINGS_DLG), hParent,
        (DLGPROC)DlgProc, (LPARAM)this);
}

INT_PTR CALLBACK SettingsDlg::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    SettingsDlg* pThis = nullptr;

    if (message == WM_INITDIALOG)
    {
        pThis = (SettingsDlg*)lParam;
        SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pThis);
        return pThis->OnInitDialog(hDlg);
    }
    else
    {
        pThis = (SettingsDlg*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    }

    if (pThis)
    {
        switch (message)
        {
        case WM_COMMAND:
            {
                int wmId = LOWORD(wParam);
                switch (wmId)
                {
                case IDOK:
                    if (pThis->OnOK(hDlg))
                    {
                        EndDialog(hDlg, IDOK);
                    }
                    return (INT_PTR)TRUE;
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    return (INT_PTR)TRUE;
                }
            }
            break;
        }
    }

    return (INT_PTR)FALSE;
}

BOOL SettingsDlg::OnInitDialog(HWND hDlg)
{
    WCHAR buf[32];

    swprintf_s(buf, L"%d", numElevators);
    SetDlgItemText(hDlg, IDC_EDIT_ELEVATORS, buf);

    swprintf_s(buf, L"%d", numFloors);
    SetDlgItemText(hDlg, IDC_EDIT_FLOORS, buf);

    CheckDlgButton(hDlg, IDC_CHECK_LINKED, linkedOperation ? BST_CHECKED : BST_UNCHECKED);

    return TRUE;
}

BOOL SettingsDlg::OnOK(HWND hDlg)
{
    WCHAR buf[32];

    GetDlgItemText(hDlg, IDC_EDIT_ELEVATORS, buf, _countof(buf));
    int elevators = _wtoi(buf);
    if (elevators < 1 || elevators > 10)
    {
        MessageBox(hDlg, L"电梯数量必须在1-10之间", L"错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    GetDlgItemText(hDlg, IDC_EDIT_FLOORS, buf, _countof(buf));
    int floors = _wtoi(buf);
    if (floors < 2 || floors > 50)
    {
        MessageBox(hDlg, L"楼层数量必须在2-50之间", L"错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    numElevators = elevators;
    numFloors = floors;
    linkedOperation = (IsDlgButtonChecked(hDlg, IDC_CHECK_LINKED) == BST_CHECKED);

    return TRUE;
}
