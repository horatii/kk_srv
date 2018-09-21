// signal.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "signal.h"

#include <base/at_exit.h>
#include <base/logging.h>
#include <base/command_line.h>
#include <base/base_paths.h>
#include <base/path_service.h>
#include <base/files/file_path.h>

#include <base/logging.h>

#include <base/win/registry.h>
#include <base/win/scoped_handle.h>

const wchar_t* kEventName = L"Global\\WaitEventName";

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
    std::wstring log_file = dir_exe.Append(L"wait.log").value();
    logging::LoggingSettings logginge_settings;
    logginge_settings.log_file = log_file.c_str();
    logging::InitLogging(logginge_settings);

    base::win::ScopedHandle hEvent(OpenEvent(EVENT_MODIFY_STATE,FALSE, kEventName));
    SetEvent(hEvent.Get());
}