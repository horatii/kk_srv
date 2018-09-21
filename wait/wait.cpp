// wait.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "wait.h"

#include <base/at_exit.h>
#include <base/logging.h>
#include <base/command_line.h>
#include <base/base_paths.h>
#include <base/path_service.h>
#include <base/files/file_path.h>

#include <base/logging.h>

#include <base/win/registry.h>
#include <base/win/scoped_handle.h>


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


    base::CommandLine* current_command_line = base::CommandLine::ForCurrentProcess();
    base::string16 event_name = current_command_line->GetSwitchValueNative("eventname");

    // set SECURITY_DESCRIPTOR
    SECURITY_DESCRIPTOR secutity;
    ::InitializeSecurityDescriptor(&secutity, SECURITY_DESCRIPTOR_REVISION);
    ::SetSecurityDescriptorDacl(&secutity, TRUE, NULL, FALSE);
    SECURITY_ATTRIBUTES securityAttr;
    // set SECURITY_ATTRIBUTES
    securityAttr.nLength = sizeof SECURITY_ATTRIBUTES;
    securityAttr.bInheritHandle = FALSE;
    securityAttr.lpSecurityDescriptor = &secutity;

    base::win::ScopedHandle hEvent(CreateEvent(&securityAttr, TRUE,  FALSE, event_name.c_str()));
    WaitForSingleObject(hEvent.Get(), INFINITE);

    HKEY current_user_key;
    RegOpenCurrentUser(KEY_READ, &current_user_key);

    base::win::RegKey reg_key(current_user_key, L"Software\\Test\\", KEY_READ);
    if (!reg_key.Valid()) {
        LOG(WARNING) << "RegKey open key error.";
        return 0;
    }

    DWORD count = 0;
    if (reg_key.ReadValueDW(L"count", &count)) {
        LOG(WARNING) << "RegKey read value key error.";
        return 0;
    }
    
    LOG(INFO) << "count: " << count;

    return 0;
}
