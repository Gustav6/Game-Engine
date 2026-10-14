#include "Application.h"

#include "Ensure.h"

std::filesystem::path GetExecutableDirectory()
{
    wchar_t buf[MAX_PATH];
    DWORD len = GetModuleFileNameW(NULL, buf, MAX_PATH);

    // Ensure the fetching of the path did not fail!
    ensure(len != 0 && len != MAX_PATH);

    std::filesystem::path exePath(buf);

    return exePath.parent_path();
}
