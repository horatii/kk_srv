#pragma once

#include <windows.h>

HRESULT GetLoggedOnUserToken(HANDLE* token);
HANDLE GetLinkedToken(HANDLE hToken);