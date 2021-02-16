#include "AudioManager.h"
#include "AudioSessionNotifier.h"
#include "Helpers.h"
#include "Macros.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <unordered_set>

#include <audiopolicy.h>
#include <AudioSessionTypes.h>
#include <mmdeviceapi.h>

#include <pugixml.hpp>

// begin helper functions

static void PrintAudioSessionDetails(IAudioSessionControl2* session)
{
    wchar_t* displayName = nullptr;
    wchar_t* iconPath = nullptr;
    wchar_t* sessionId = nullptr;
    wchar_t* sessionInstanceId = nullptr;
    GUID groupingParam;
    DWORD pid;
    AudioSessionState state;
    HRESULT res = session->GetProcessId(&pid);
    if (FAILED(res))
        printf("unable to retrieve PID\n");
    else
        printf("Process ID: %u\n", pid);
    res = session->GetDisplayName(&displayName);
    if (FAILED(res))
        printf("unable to retrieve session display name\n");
    else
        wprintf(L"Display Name: %s\n", displayName);
    res = session->GetIconPath(&iconPath);
    if (FAILED(res))
        printf("unable to retrieve session icon path\n");
    else
        wprintf(L"Icon Path: %s\n", iconPath);
    res = session->GetGroupingParam(&groupingParam);
    if (FAILED(res))
        printf("unable to retrieve session grouping param\n");
    else
    {
        wchar_t guidStr[256];
        int res = StringFromGUID2(groupingParam, guidStr, 256);
        if (res != 0)
            wprintf(L"Grouping Param: %s\n", guidStr);
        else
            printf("unable to retrieve session grouping param\n");
    }
    res = session->GetSessionIdentifier(&sessionId);
    if (FAILED(res))
        printf("unable to retrieve session ID\n");
    else
        wprintf(L"Session ID: %s\n", sessionId);
    res = session->GetSessionInstanceIdentifier(&sessionInstanceId);
    if (FAILED(res))
        printf("unable to retrieve session instance ID\n");
    else
        wprintf(L"Session instance ID: %s\n", sessionInstanceId);
    res = session->GetState(&state);
    if (FAILED(res))
        printf("unable to retrieve session state\n");
    else
        printf("State: %s\n", helpers::SessionStateToText(state).c_str());

    CoTaskMemFree(displayName);
    CoTaskMemFree(iconPath);
    CoTaskMemFree(sessionId);
    CoTaskMemFree(sessionInstanceId);
}

midi::MidiMessage GetMidiMessageFromAttrs(const pugi::xml_node& action)
{
    // first and second attributes are required
    midi::MidiData firstByte = static_cast<midi::MidiData>(action.attribute("first").as_uint());
    midi::MidiData secondByte = static_cast<midi::MidiData>(action.attribute("second").as_uint());

    // if no third attribute defined - use as multi-input message, i.e., knob or fader
    pugi::xml_attribute thirdAttribute = action.attribute("third");
    if (!thirdAttribute)
        return midi::MidiMessage(firstByte, secondByte);
    // if third attribute defined - use as single-input message, i.e., button press
    else
        return midi::MidiMessage(firstByte, secondByte, static_cast<midi::MidiData>(thirdAttribute.as_uint()));
}

template<typename Hash, typename Eq>
void GeneralExpiredSessionErasure(std::unordered_multimap<midi::MidiMessage, std::unique_ptr<action::IAction>, Hash, Eq>& mapping)
{
    for (auto it = mapping.begin(); it != mapping.end(); /* nothing */)
    {
        // TODO handle Expired() exception
        if (it->second->Expired())
        {
            std::cout << "Erased " << *it->second << '\n';
            it = mapping.erase(it);
        }
        else
        {
            it++;
        }
    }
}

template<typename Hash, typename Eq>
bool GeneralActionExecution(const std::unordered_multimap<midi::MidiMessage, std::unique_ptr<action::IAction>, Hash, Eq>& mapping, const midi::MidiMessage& inputMsg)
{
    using mapIter = typename std::unordered_multimap<midi::MidiMessage, std::unique_ptr<action::IAction>, Hash, Eq>::const_iterator;
    bool actionExecuted = false;
    std::pair<mapIter, mapIter> range = mapping.equal_range(inputMsg);
    for (mapIter it = range.first; it != range.second; it++)
    {
        it->second->Execute(inputMsg.third);
        actionExecuted = true;
        DBG(std::cout << "executed " << *it->second << "\n");
    }
    return actionExecuted;
}

// end helper functions

manager::AudioManager::AudioManager(const std::string& configPath, const std::chrono::seconds& cleanupPeriod) :
    _pendingMapping(),
    _singleValueMapping(),
    _multiValueMapping(),
    _sessionManager(nullptr),
    _sessionNotification(new event::AudioSessionNotifier([this](IAudioSessionControl2* session)
        {
            session->AddRef();
            OnNewAudioSessionCreated(session);
            helpers::SafeRelease(&session);
        })),
    _finished(false),
    _mapMutex(),
    _cleanupMutex(),
    _cleanupCondition(),
    _cleanupThread() // start execution when constructor ends
{
    ParseXmlConfig(configPath);

    IMMDevice* mmDevice = nullptr;
    IMMDeviceEnumerator* mmDeviceEnum = nullptr;
    IAudioSessionEnumerator* sessionEnum = nullptr;
    IAudioSessionControl* session = nullptr;
    IAudioSessionControl2* sessionTwo = nullptr;

    HRESULT res = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID*)&mmDeviceEnum);
    if (FAILED(res))
        goto cleanup;
    res = mmDeviceEnum->GetDefaultAudioEndpoint(EDataFlow::eRender, ERole::eConsole, &mmDevice);
    if (FAILED(res))
        goto cleanup;
    res = mmDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_INPROC_SERVER, nullptr, (LPVOID*)&_sessionManager);
    if (FAILED(res))
        goto cleanup;
    res = _sessionManager->RegisterSessionNotification(_sessionNotification);
    if (FAILED(res))
        goto cleanup;
    res = _sessionManager->GetSessionEnumerator(&sessionEnum);
    if (FAILED(res))
        goto cleanup;

    int sessionCount;
    res = sessionEnum->GetCount(&sessionCount);
    if (FAILED(res))
        goto cleanup;

    for (int i = 0; i < sessionCount; i++)
    {
        res = sessionEnum->GetSession(i, &session);
        if (FAILED(res))
            goto cleanuploop;
        session->AddRef();
        res = session->QueryInterface(__uuidof(IAudioSessionControl2), (LPVOID*)&sessionTwo);
        if (FAILED(res))
            goto cleanuploop;

        OnNewAudioSessionCreated(sessionTwo);

    cleanuploop:
        helpers::SafeRelease(&session);
        helpers::SafeRelease(&sessionTwo);
        if (FAILED(res))
            break;
    }

cleanup:
    if (FAILED(res))
    {
        if (_sessionManager != nullptr && _sessionNotification != nullptr)
            _sessionManager->UnregisterSessionNotification(_sessionNotification);
        helpers::SafeRelease(&sessionEnum);
        helpers::SafeRelease(&mmDevice);
        helpers::SafeRelease(&mmDeviceEnum);
        EndCleanup();
        throw std::runtime_error("Failed to create audio manager with error code " + std::to_string(GetLastError()));
    }

    // start cleanup thread execution
    _cleanupThread = std::thread(&AudioManager::PeriodicCleanup, this, cleanupPeriod);
}

manager::AudioManager::~AudioManager()
{
    EndCleanup();
    if (_sessionManager != nullptr && _sessionNotification != nullptr)
    {
        _sessionManager->UnregisterSessionNotification(_sessionNotification);
    }
    helpers::SafeRelease(&_sessionNotification);
    helpers::SafeRelease(&_sessionManager);
}

void manager::AudioManager::ExecuteAction(const midi::MidiMessage& inputMessage) const
{
    std::scoped_lock lock(_mapMutex);
    // if no elements found in multi-value map, try single-value map
    if (!GeneralActionExecution(_multiValueMapping, inputMessage))
        GeneralActionExecution(_singleValueMapping, inputMessage);
}

void manager::AudioManager::EmplaceActiveMapping(const midi::MidiMessage& msg, std::unique_ptr<action::IAction>&& action)
{
    if (msg.IsMultiValue())
        _multiValueMapping.emplace(msg, std::move(action));
    else
        _singleValueMapping.emplace(msg, std::move(action));
}

void manager::AudioManager::ParseXmlConfig(const std::string& configPath)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(configPath.c_str());

    if (result.status != pugi::status_ok)
    {
        throw std::runtime_error("Error when parsing XML config: " + std::string(result.description()));
    }

    // get 'global' node
    pugi::xml_node node = doc.first_child().first_child();

    // iterate 'global' children
    for (pugi::xml_node action = node.first_child(); action; action = action.next_sibling())
    {
        midi::MidiMessage msg = GetMidiMessageFromAttrs(action);
        EmplaceActiveMapping(msg, action::GetGlobalAction(action.child_value()));
    }

    // iterate over 'session' nodes
    for (node = node.next_sibling(); node; node = node.next_sibling())
    {
        std::unordered_set<std::string> executables;
        executables.reserve(128);

        // iterate 'paths'
        pugi::xml_node paths = node.first_child();
        for (pugi::xml_node path = paths.first_child(); path; path = path.next_sibling())
        {
            std::filesystem::path fsPath(path.child_value());
            if (std::filesystem::is_directory(fsPath))
                helpers::FindFilesRecursively(executables, fsPath, ".exe");
            else
                executables.emplace(fsPath.filename().string());
        }

        // iterate 'actions'
        for (pugi::xml_node action = paths.next_sibling().first_child(); action; action = action.next_sibling())
        {
            midi::MidiMessage msg = GetMidiMessageFromAttrs(action);
            for (auto const& exec : executables)
                _pendingMapping.emplace(exec,
                    std::make_pair(msg, action::GetGenerator(action.child_value())));
        }
    }
}

std::string manager::AudioManager::MatchSessionWithApp(IAudioSessionControl2* session)
{
    HRESULT res = session->IsSystemSoundsSession();
    if (res == S_OK)
        return "";

    // try getting process executable name from PID and fall back to
    // extracting the executable name from the audio session ID if this fails
    DWORD pid;
    res = session->GetProcessId(&pid);
    if (FAILED(res))
    {
        helpers::SafeRelease(&session);
        throw std::runtime_error("unable to get session PID");
    }

    std::string processName;
    try
    {
        processName = helpers::GetExecutableNameFromPid(pid);
    }
    catch (const std::runtime_error& e)
    {
        DWORD error = GetLastError();
        if (error == ERROR_ACCESS_DENIED)
        {
            std::cout << "unable to get executable name for PID "
                << pid << " (error code " << error
                << "), attempting to extract executable name from session ID\n";

            wchar_t* sessionId;
            HRESULT res = session->GetSessionIdentifier(&sessionId);
            if (FAILED(res))
            {
                helpers::SafeRelease(&session);
                throw std::runtime_error("unable to get session ID");
            }
            processName = helpers::GetExecutableNameFromSessionId(sessionId);
            CoTaskMemFree(sessionId);
        }
        else
        {
            // rethrow exception if some other error happened
            throw e;
        }
    }
    
    if (_pendingMapping.find(processName) != _pendingMapping.end())
    {
        return processName;
    }
    return "";
}

void manager::AudioManager::OnNewAudioSessionCreated(IAudioSessionControl2* session)
{
    const std::string match = MatchSessionWithApp(session);
    if (match.empty())
        return;

    std::cout << "New audio session created for '" << match << "'\n";
    using MapIter = decltype(_pendingMapping)::const_iterator;

    std::scoped_lock lock(_mapMutex);
    std::pair<MapIter, MapIter> range = _pendingMapping.equal_range(match);
    for (MapIter it = range.first; it != range.second; it++)
    {
        const auto& [message, generator] = it->second;
        EmplaceActiveMapping(message, generator(match, session));
    }
}

void manager::AudioManager::EnumerateAudioSessions() const
{
    IAudioSessionEnumerator* enumerator = nullptr;
    IAudioSessionControl* session = nullptr;
    IAudioSessionControl2* sessionTwo = nullptr;
    int sessionCount;
    HRESULT res = _sessionManager->GetSessionEnumerator(&enumerator);
    if (FAILED(res))
    {
        printf("unable to get session enumerator\n");
        return;
    }
    res = enumerator->GetCount(&sessionCount);
    if (FAILED(res))
        goto cleanup;

    for (int i = 0; i < sessionCount; i++)
    {
        res = enumerator->GetSession(i, &session);
        if (FAILED(res))
            goto cleanuploop;
        session->AddRef();
        res = session->QueryInterface(__uuidof(IAudioSessionControl2), (LPVOID*)&sessionTwo);
        if (FAILED(res))
            goto cleanuploop;

        PrintAudioSessionDetails(sessionTwo);
        printf("\n");

    cleanuploop:
        helpers::SafeRelease(&session);
        helpers::SafeRelease(&sessionTwo);
        if (FAILED(res))
            break;
    }

cleanup:
    if (FAILED(res))
    {
        helpers::SafeRelease(&enumerator);
        printf("Failed to enumerate audio sessions (error code %u)\n", GetLastError());
    }

}

void manager::AudioManager::EraseExpiredSessions()
{
    std::cout << "Erasing expired audio sessions...\n";
    std::scoped_lock lock(_mapMutex);
    GeneralExpiredSessionErasure(_singleValueMapping);
    GeneralExpiredSessionErasure(_multiValueMapping);
}

void manager::AudioManager::EndCleanup()
{
    {
        std::scoped_lock lock(_cleanupMutex);
        _finished = true;
    }
    _cleanupCondition.notify_one();
    _cleanupThread.join();
}

void manager::AudioManager::PeriodicCleanup(const std::chrono::seconds& interval)
{
    std::cout << "Started cleanup thread\n";
    while (true)
    {
        std::unique_lock lock(_cleanupMutex);
        _cleanupCondition.wait_for(lock, interval);
        if (_finished)
        {
            std::cout << "Finished cleanup thread\n";
            break;
        }
        EraseExpiredSessions();
    }
}

std::ostream& manager::operator<<(std::ostream& os, const AudioManager& am)
{
    os << "*** Registered Apps (" << am._pendingMapping.size() << ") ***\n";
    for (auto const& [key, value] : am._pendingMapping)
        os << key << " : " << value.first << '\n';
    os << "*** Active Mapping (" << am._singleValueMapping.size() + am._multiValueMapping.size() << ") ***\n";
    for (auto const& [key, value] : am._singleValueMapping)
        os << key << " : " << *value << '\n';
    for (auto const& [key, value] : am._multiValueMapping)
        os << key << " : " << *value << '\n';
    return os;
}
