#pragma once

#include "IAction.h"

#include <iosfwd>
#include <string>

struct IAudioSessionControl2;
struct ISimpleAudioVolume;
struct IAudioSessionEvents;

namespace action
{

    class SessionVolumeControl : public IAction
    {
    private:
        std::string _application;
        IAudioSessionControl2* _session;
        ISimpleAudioVolume* _audioVolume;
        IAudioSessionEvents* _sessionEvents;

    protected:
        SessionVolumeControl(const std::string& application, IAudioSessionControl2* session);
        ~SessionVolumeControl();

    public:
        IAudioSessionControl2* GetAudioSessionControl() const
        {
            return _session;
        }

        ISimpleAudioVolume* GetSimpleAudioVolume() const
        {
            return _audioVolume;
        }

        const std::string& GetApplication() const
        {
            return _application;
        }

        // Inherited via IAction
        virtual bool Expired() const override;

        // Forbid copies
        SessionVolumeControl(const SessionVolumeControl& other) = delete;
        SessionVolumeControl& operator=(const SessionVolumeControl& other) = delete;

    protected:
        // Inherited via IAction
        virtual void Print(std::ostream& os) const override;
    };

}
