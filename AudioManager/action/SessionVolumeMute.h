#pragma once

#include "SessionVolumeControl.h"

namespace action
{

    class SessionVolumeMute : public SessionVolumeControl
    {
    public:
        SessionVolumeMute(const std::string& application, IAudioSessionControl2* session);

        // forbid copies
        SessionVolumeMute(const SessionVolumeMute& other) = delete;
        SessionVolumeMute& operator=(const SessionVolumeMute& other) = delete;

        // Inherited via SessionVolumeControl
        virtual void Execute(ExecValue value) override;

    protected:
        virtual void Print(std::ostream& os) const override;
    };

}
