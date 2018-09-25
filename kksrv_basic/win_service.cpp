#include "win_service.h"

#include <base/lazy_instance.h>
#include <base/memory/scoped_ptr.h>

WinService* _winservice_pointer = nullptr;

WinService::WinService(Delegate* delegate) 
    : delegate_(delegate)
{
    ZeroMemory(&status_, sizeof(SERVICE_STATUS));
    status_.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    status_.dwCurrentState = SERVICE_STOPPED;
    status_.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    status_.dwWin32ExitCode = 0;
    status_.dwServiceSpecificExitCode = 0;
    status_.dwCheckPoint = 0;
    status_.dwWaitHint = 0;

    _winservice_pointer = this;
}

WinService::~WinService()
{
    _winservice_pointer = nullptr;
}

BOOL WinService::WinMain()
{
    SERVICE_TABLE_ENTRY service_table[] = {
        { delegate_->ServiceName(), _ServiceMain },
        { NULL, NULL }
    };
    return StartServiceCtrlDispatcher(service_table);
}

void WinService::OnShutdown()
{
    SetServiceStatus(SERVICE_STOP_PENDING);
    delegate_->OnShutdown();
}

void WinService::OnStop()
{
    SetServiceStatus(SERVICE_STOP_PENDING);
    delegate_->OnStop();
}

VOID WinService::SetServiceStatus(DWORD dwStatus)
{
    status_.dwCurrentState = dwStatus;
    ::SetServiceStatus(status_handle_, &status_);
}

void WinService::ServiceMain(DWORD dwArgc, LPWSTR *lpszArgv)
{
    status_.dwCurrentState = SERVICE_START_PENDING;
    status_handle_ = RegisterServiceCtrlHandler(delegate_->ServiceName(), _ServiceCtrlHandler);
    SetServiceStatus(SERVICE_START_PENDING);
    SetServiceStatus(SERVICE_RUNNING);
    delegate_->Run();
    SetServiceStatus(SERVICE_STOPPED);
}

void WinService::ServiceCtrlHandler(DWORD dwControl)
{
    switch (dwControl) {
    case SERVICE_CONTROL_SHUTDOWN:
        OnShutdown();
        break;
    case SERVICE_CONTROL_STOP:
        OnStop();
        break;
    }
}

void WINAPI WinService::_ServiceMain(DWORD dwArgc, LPWSTR *lpszArgv)
{
    _winservice_pointer->ServiceMain(dwArgc, lpszArgv);
}

void WINAPI WinService::_ServiceCtrlHandler(DWORD dwControlt)
{
    _winservice_pointer->ServiceCtrlHandler(dwControlt);
}
