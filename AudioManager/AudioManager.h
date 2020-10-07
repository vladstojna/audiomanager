#pragma once

#include <string>
#include <map>
#include <mutex>

#include "Midi.h"
#include "action/Generator.h"
#include "action/IAction.h"

struct IAudioSessionManager2;
struct IAudioSessionNotification;
struct IAudioSessionControl2;

namespace manager
{

class AudioManager
{
private:
    std::unordered_multimap<std::string, std::pair<midi::MidiMessage, action::Generator>> _pendingMapping;
    std::unordered_multimap<midi::MidiMessage, std::unique_ptr<action::IAction>, midi::MidiMessage::Hasher> _activeMapping;

    IAudioSessionManager2* _sessionManager;
    IAudioSessionNotification* _sessionNotification;

    bool _finished;
    mutable std::mutex _mapMutex;
    mutable std::mutex _cleanupMutex;
    std::condition_variable _cleanupCondition;
    std::thread _cleanupThread;

public:
    AudioManager(const std::string& configPath, const std::chrono::seconds& cleanupPeriod = std::chrono::seconds(600));
    ~AudioManager();

    void ExecuteAction(const midi::MidiMessage& inputMessage, midi::MidiData midiValue) const;
    void EnumerateAudioSessions() const;

    // delete copy constructor and assignment operator
    AudioManager(const AudioManager& other) = delete;
    AudioManager& operator=(const AudioManager& other) = delete;

    friend std::ostream& operator<<(std::ostream& os, const AudioManager& am);

private:
    std::string MatchSessionWithApp(IAudioSessionControl2* session);
    void ParseXmlConfig(const std::string& configPath);
    void OnNewAudioSessionCreated(IAudioSessionControl2* session);
    void EraseExpiredSessions();
    void EndCleanup();
    void PeriodicCleanup(const std::chrono::seconds& interval);
};

std::ostream& operator<<(std::ostream& os, const AudioManager& am);

}
