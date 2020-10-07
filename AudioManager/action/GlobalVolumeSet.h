#pragma once

#include "IAction.h"

struct IAudioEndpointVolume;

namespace action
{

    class GlobalVolumeSet : public IAction
    {
    private:
        IAudioEndpointVolume* _endpointVolume;

    public:
        GlobalVolumeSet();
        ~GlobalVolumeSet();

        // Inherited via IAction
        virtual void Execute(ExecValue value) override;

    protected:
        virtual void Print(std::ostream& os) const override;
    };

}
