#pragma once

#include <unordered_set>

#include <AudioSessionTypes.h>

namespace std::filesystem { class path; }

namespace helpers
{

std::string Timestamp(const std::string& format = "%T");

std::wstring Utf8ToUtf16(const std::string& str);

std::string Utf16ToUtf8(const std::wstring& str);

std::string SessionStateToText(AudioSessionState state);

std::string GetExecutableNameFromPid(unsigned long pid);

std::string GetExecutableNameFromSessionId(const wchar_t* sessionId);

void FindFilesRecursively(std::unordered_set<std::string>& files, const std::filesystem::path& directory, const std::filesystem::path& extension);

// templates

template<typename T>
void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = nullptr;
    }
}

}