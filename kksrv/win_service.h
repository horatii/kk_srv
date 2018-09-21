#pragma once

#include <base/strings/string16.h>


class WinServiceDelegate
{
public:
    virtual ~WinServiceDelegate() {}

    virtual LPWSTR ServiceName() = 0;
    virtual void Run() = 0;
    virtual void OnShutdown() = 0;
    virtual void OnStop() = 0;    
};

class WinService
{
public:
    WinService(WinServiceDelegate* delegate);
    ~WinService();

    BOOL WinMain();

    VOID SetServiceStatus(DWORD dwStatus);

protected:
    void OnShutdown();
    void OnStop();

private:
    void ServiceMain(DWORD dwArgc, LPWSTR *lpszArgv);
    void ServiceCtrlHandler(DWORD dwControl);

    static void WINAPI _ServiceMain(DWORD dwArgc, LPWSTR *lpszArgv);
    static void WINAPI _ServiceCtrlHandler(DWORD dwControlt);

    WinServiceDelegate*    delegate_;
    SERVICE_STATUS         status_;
    SERVICE_STATUS_HANDLE  status_handle_;
};
