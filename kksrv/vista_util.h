#pragma once

HRESULT GetLoggedOnUserToken(HANDLE* token);
HANDLE GetLinkedToken(HANDLE hToken);