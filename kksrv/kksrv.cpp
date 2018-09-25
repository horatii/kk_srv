// kksrv.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "kksrv.h"

#include "../kksrv_basic/win_service.h"
#include "../kksrv_basic/queue_timer.h"
#include "../kksrv_basic/vista_util.h"

#include <base/at_exit.h>
#include <base/logging.h>
#include <base/command_line.h>
#include <base/base_paths.h>
#include <base/path_service.h>
#include <base/files/file_path.h>
#include <base/win/registry.h>

#include <base/process/launch.h>
#include <base/synchronization/waitable_event.h>

#include <atlbase.h>
#include <atlsecurity.h>

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "wtsapi32.lib")
#pragma comment(lib, "shlwapi.lib")

const wchar_t* kEventName = L"Global\\WaitEventName";

class QueueTimerWorker 
{
public:
    QueueTimerWorker(HANDLE timer_queue) {
        queue_timer_.reset(new omaha::QueueTimer(timer_queue, _Callback, this));
    }

    virtual ~QueueTimerWorker() {}

    void Start(int due_time, int period, uint32_t flags) {
        queue_timer_->Start(due_time, period, false);
    }

    virtual void Work() = 0;

    static void _Callback(omaha::QueueTimer* timer) {
        QueueTimerWorker* pThis = reinterpret_cast<QueueTimerWorker*>(timer->ctx());
        pThis->Work();
    }

private:
    scoped_ptr<omaha::QueueTimer> queue_timer_;
};

class LaunchProcessTimerWorker : public QueueTimerWorker
{
public:
    LaunchProcessTimerWorker(HANDLE timer_queue) : QueueTimerWorker(timer_queue) {
    }

    virtual void Work()
    {
        base::FilePath dir_exe;
        base::PathService::Get(base::DIR_EXE, &dir_exe);

        HANDLE user_token = INVALID_HANDLE_VALUE;
        if (FAILED(GetLoggedOnUserToken(&user_token))) {
            LOG(WARNING) << "GetLoggedOnUserToken Failed.";
            return;
        }
        HANDLE linked_token = nullptr;//GetLinkedToken(user_token);

        base::LaunchOptions opt;
        opt.wait = true;
        opt.as_user = nullptr;// linked_token ? linked_token : user_token;

        base::CommandLine command_line(dir_exe.Append(L"wait.exe"));
        command_line.AppendSwitchNative("EventName", kEventName);
        base::LaunchProcess(command_line, opt);
        if (linked_token) {
            CloseHandle(linked_token);
        }
        CloseHandle(user_token);
    }
};

class RegLookupTimerWorker : public QueueTimerWorker
{
public:
    RegLookupTimerWorker(HANDLE timer_queue) : QueueTimerWorker(timer_queue) {
    }

    virtual void Work()
    {
        HKEY current_user_key;
        RegOpenCurrentUser(KEY_READ, &current_user_key);

        base::win::RegKey reg_key(current_user_key, L"Software\\Test\\", KEY_READ);
        if (!reg_key.Valid()) {
            LOG(WARNING) << "RegKey open key error.";
            return;
        }

        DWORD count = 0;
        if (reg_key.ReadValueDW(L"count", &count)) {
            LOG(WARNING) << "RegKey read value key error.";
            return;
        }        
        LOG(INFO) << "count: " << count;
    }
};

class WinServiceDelegateImpl : public WinService::Delegate
{
public:
    WinServiceDelegateImpl()
    : stop_event_(false, false)
    {
        timer_queue_ = ::CreateTimerQueue();
    }

    virtual LPWSTR ServiceName() override
    {
        return L"kksrv";
    }

    virtual void Run() override
    { 
        LaunchProcessTimerWorker worker_(timer_queue_);
        RegLookupTimerWorker     worker2_(timer_queue_);

        worker_.Start(0, 5000, WT_EXECUTEINTIMERTHREAD | WT_EXECUTEONLYONCE);        
        worker_.Start(0, 5000, WT_EXECUTEINTIMERTHREAD | WT_EXECUTEONLYONCE);

        stop_event_.Wait();
    }

    virtual void OnShutdown() override
    {       
        stop_event_.Signal();
    }


    virtual void OnStop() override
    {
        stop_event_.Signal();
    }

private:
    HANDLE timer_queue_;    
    base::WaitableEvent stop_event_;
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    base::AtExitManager at_exit_manager;
    base::CommandLine::Init(0, 0);

    // 初始化日志
    base::FilePath dir_exe;
    base::PathService::Get(base::DIR_EXE, &dir_exe);
    std::wstring log_file = dir_exe.Append(L"kksrv.log").value();
    logging::LoggingSettings logginge_settings;
    logginge_settings.log_file = log_file.c_str();
    logging::InitLogging(logginge_settings);

    WinServiceDelegateImpl delegate;
    WinService service(&delegate);
    service.WinMain();
}