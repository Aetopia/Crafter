#define _MINAPPMODEL_H_
#include <initguid.h>
#include <shobjidl.h>
#include <appmodel.h>
#include <aclapi.h>
#include <sddl.h>
#include <commctrl.h>
#include <commdlg.h>

WCHAR szLibFileName[MAX_PATH] = {};

HRESULT TaskDialogCallbackProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LONG_PTR lpRefData)
{
    switch (uMsg)
    {
    case TDN_CREATED:
        lParam = (LPARAM)LoadImageW(GetModuleHandleW(NULL), NULL, IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
        SendMessageW(hWnd, WM_SETICON, ICON_SMALL, lParam);
        SendMessageW(hWnd, WM_SETICON, ICON_BIG, lParam);
        break;

    case TDN_BUTTON_CLICKED:
        if (wParam == IDCANCEL)
            ExitProcess(0);
        else if (wParam == IDOK)
            break;

        if (GetOpenFileNameW(&((OPENFILENAMEW){.lStructSize = sizeof(OPENFILENAMEW),
                                               .hwndOwner = hWnd,
                                               .lpstrFilter = L"Application extensions (*.dll)\0*.dll*\0\0",
                                               .lpstrFile = szLibFileName,
                                               .Flags = OFN_FILEMUSTEXIST,
                                               .FlagsEx = OFN_EX_NOPLACESBAR,
                                               .nMaxFile = MAX_PATH})))
            WritePrivateProfileStringW(L"", L"", szLibFileName, (PWSTR)lpRefData);
        return S_FALSE;
    }
    return S_OK;
}

void WinMainCRTStartup()
{
    WCHAR szFileName[MAX_PATH] = {};
    QueryFullProcessImageNameW(GetCurrentProcess(), 0, szFileName, &((DWORD){MAX_PATH}));
    for (DWORD _ = lstrlenW(szFileName); _ < -1; _--)
        if (szFileName[_] == '\\')
        {
            szFileName[_ = _ + 1] = '\0';
            lstrcatW(szFileName, L".ini");
            break;
        }
    GetPrivateProfileStringW(L"", L"", NULL, szLibFileName, MAX_PATH, szFileName);

    TaskDialogIndirect(&((TASKDIALOGCONFIG){
                           .cbSize = sizeof(TASKDIALOGCONFIG),
                           .pszWindowTitle = L"Crafter",
                           .dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_USE_HICON_MAIN | TDF_SIZE_TO_CONTENT |
                                      TDF_USE_COMMAND_LINKS | TDF_POSITION_RELATIVE_TO_WINDOW | TDF_EXPAND_FOOTER_AREA,
                           .lpCallbackData = (LONG_PTR)szFileName,
                           .cButtons = 2,
                           .pfCallback = TaskDialogCallbackProc,
                           .pButtons = (TASKDIALOG_BUTTON[]){{.nButtonID = IDOK, .pszButtonText = L"Play"},
                                                             {.nButtonID = IDYES, .pszButtonText = L"Select"}}}),
                       NULL, NULL, NULL);

    PACL OldAcl = NULL;
    GetNamedSecurityInfoW(szLibFileName, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, &OldAcl, NULL, NULL);

    PSID Sid = NULL;
    ConvertStringSidToSidW(L"S-1-15-2-1", &Sid);

    PACL NewAcl = NULL;
    SetEntriesInAclW(
        1,
        &((EXPLICIT_ACCESSW){
            .grfAccessPermissions = GENERIC_READ | GENERIC_EXECUTE,
            .grfAccessMode = SET_ACCESS,
            .grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT,
            .Trustee = {.TrusteeForm = TRUSTEE_IS_SID, .TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP, .ptstrName = Sid}}),
        OldAcl, &NewAcl);

    SetNamedSecurityInfoW(szLibFileName, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, NewAcl, NULL);

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    IPackageDebugSettings *pPackageDebugSettings = NULL;
    CoCreateInstance(&CLSID_PackageDebugSettings, NULL, CLSCTX_INPROC_SERVER, &IID_IPackageDebugSettings,
                     (LPVOID *)&pPackageDebugSettings);

    WCHAR _[PACKAGE_FULL_NAME_MAX_LENGTH] = {};
    GetPackagesByPackageFamily(L"Microsoft.MinecraftUWP_8wekyb3d8bbwe", &((UINT){1}), (PWSTR[]){},
                               &((UINT32){PACKAGE_FULL_NAME_MAX_LENGTH}), _);
    pPackageDebugSettings->lpVtbl->EnableDebugging(pPackageDebugSettings, _, NULL, NULL);

    IApplicationActivationManager *pApplicationActivationManager = NULL;
    CoCreateInstance(&CLSID_ApplicationActivationManager, NULL, CLSCTX_INPROC_SERVER,
                     &IID_IApplicationActivationManager, (LPVOID *)&pApplicationActivationManager);
    CoAllowSetForegroundWindow((IUnknown *)pApplicationActivationManager, NULL);

    DWORD dwProcessId = 0;
    pApplicationActivationManager->lpVtbl->ActivateApplication(
        pApplicationActivationManager, L"Microsoft.MinecraftUWP_8wekyb3d8bbwe!App", NULL, AO_NOERRORUI, &dwProcessId);

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
    LPVOID lpBaseAddress =
        VirtualAllocEx(hProcess, NULL, sizeof(szLibFileName), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    WriteProcessMemory(hProcess, lpBaseAddress, szLibFileName, sizeof(szLibFileName), NULL);
    HANDLE hThread =
        CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryW, lpBaseAddress, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);
    VirtualFreeEx(hProcess, lpBaseAddress, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    ExitProcess(0);
}