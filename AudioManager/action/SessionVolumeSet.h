#pragma once

#include "SessionVolumeControl.h"

namespace action
{

    class SessionVolumeSet : public SessionVolumeControl
    {
    public:
        SessionVolumeSet(const std::string& application, IAudioSessionControl2* session);

        // forbid copies
        SessionVolumeSet(const SessionVolumeSet& other) = delete;
        SessionVolumeSet& operator=(const SessionVolumeSet& other) = delete;

        // Inherited via SessionVolumeControl
        virtual void Execute(ExecValue value) override;

    protected:
        virtual void Print(std::ostream& os) const override;
    };

}
