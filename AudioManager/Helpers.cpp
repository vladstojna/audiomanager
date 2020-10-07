#include "Helpers.h"

#include <ctime>
#include <chrono>
#include <filesystem>
#include <stdexcept>
#include <regex>

#include <Windows.h>
#include <Psapi.h>

std::tm GetLocaltime(const std::time_t& time)
{
    std::tm myTm;
#if defined(__unix__)
    localtime_r(&time, &myTm);
#elif defined(_MSC_VER)
    localtime_s(&myTm, &time);
#else
    static std::mutex mutex;
    std::scoped_lock lock(mutex);
    myTm = *std::localtime(&time);
#endif
    return myTm;
}

std::string helpers::Timestamp(const std::string& format)
{
    using namespace std::chrono;

    system_clock::time_point p = system_clock::now();
    milliseconds ms = duration_cast<milliseconds>(p.time_since_epoch());
    seconds s = duration_cast<seconds>(ms);
    std::tm myTm = GetLocaltime(s.count());

    char buffer[128];
    size_t sz = std::strftime(buffer, sizeof(buffer), format.c_str(), &myTm);
    return std::string(buffer, sz + snprintf(buffer + sz, 128 - sz, ".%03llu", ms.count() % milliseconds::period::den));
}

std::wstring helpers::Utf8ToUtf16(const std::string& str)
{
    std::wstring returnValue;

    int wideCharSize = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, str.c_str(), -1, nullptr, 0);
    if (wideCharSize == 0)
    {
        return returnValue;
    }
    returnValue.resize(wideCharSize);
    wideCharSize = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, str.c_str(), -1, &returnValue[0], wideCharSize);
    if (wideCharSize == 0)
    {
        returnValue.resize(0);
        return returnValue;
    }
    returnValue.resize((size_t)wideCharSize - 1);
    return returnValue;
}

std::string helpers::Utf16ToUtf8(const std::wstring& str)
{
    std::string returnValue;

    int mbCharSize = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, str.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (mbCharSize == 0)
    {
        return returnValue;
    }
    returnValue.resize(mbCharSize);
    mbCharSize = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, str.c_str(), -1, &returnValue[0], mbCharSize, nullptr, nullptr);
    if (mbCharSize == 0)
    {
        returnValue.resize(0);
        return returnValue;
    }
    returnValue.resize((size_t)mbCharSize - 1);
    return returnValue;
}

std::string helpers::SessionStateToText(AudioSessionState state)
{
    switch (state)
    {
    case AudioSessionState::AudioSessionStateActive:
        return std::string("active");
    case AudioSessionState::AudioSessionStateInactive:
        return std::string("inactive");
    case AudioSessionState::AudioSessionStateExpired:
        return std::string("expired");
    default:
        return std::string("undefined");
    }
}

std::string helpers::GetExecutableNameFromPid(unsigned long pid)
{
    wchar_t szProcessName[MAX_PATH];
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

    if (hProcess == nullptr)
    {
        throw std::runtime_error("OpenProcess exited with error code " + std::to_string(GetLastError()));
    }

    if (GetProcessImageFileName(hProcess, szProcessName, MAX_PATH) == 0)
    {
        CloseHandle(hProcess);
        throw std::runtime_error("GetProcessImageFileName exited with error code " + std::to_string(GetLastError()));
    }

    CloseHandle(hProcess);
    return std::filesystem::path(szProcessName).filename().string();
}

std::string helpers::GetExecutableNameFromSessionId(const wchar_t* sessionId)
{
    // match anything written between the last \ and %
    std::basic_regex<wchar_t> regex(LR"(.+\\(.+?)%)");
    std::match_results<const wchar_t*> baseMatch;
    if (std::regex_search(sessionId, baseMatch, regex) == false)
    {
        return "";
    }
    // return first matched group instead of the whole match
    return Utf16ToUtf8(baseMatch[1].str());
}

void helpers::FindFilesRecursively(std::unordered_set<std::string>& files, const std::filesystem::path& directory, const std::filesystem::path& extension)
{
    for (auto const& entry : std::filesystem::recursive_directory_iterator(directory))
    {
        if (entry.is_regular_file() && entry.path().has_filename() && entry.path().extension() == extension)
        {
            files.emplace(entry.path().filename().string());
        }
    }
}
