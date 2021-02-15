#include "Generator.h"
#include "KeyboardAction.h"
#include "ConditionalAction.h"
#include "GlobalVolumeSet.h"
#include "SessionVolumeMute.h"
#include "SessionVolumeSet.h"

template<typename T, typename... Args>
auto CreateGlobalGenerator(Args&&... args)
{
#if __cplusplus == 201703L
    return[args = std::make_tuple(std::forward<Args>(args)...)]() mutable
    {
        return std::apply([](auto&&... args)
            {
                return std::make_unique<T>(args...);
            }, std::move(args));
    };
#else
    return[... args = std::forward<Args>(args)]() mutable
    {
        return std::make_unique<T>(std::move(args)...);
    };
#endif
}

constexpr char PLAY_PAUSE[] = "PlayPause";
constexpr char PREV_TRACK[] = "PrevTrack";
constexpr char NEXT_TRACK[] = "NextTrack";
constexpr char VOLUME_MUTE[] = "VolumeMute";
constexpr char VOLUME_SET[] = "VolumeSet";

using namespace action;

const std::unordered_map<std::string, GeneratorT<IAction>> typeMapGlobal =
{
    {
        PLAY_PAUSE, CreateGlobalGenerator<KeyboardAction>("Play/Pause", VK_MEDIA_PLAY_PAUSE, KEYEVENTF_EXTENDEDKEY)
    },
    {
        PREV_TRACK, CreateGlobalGenerator<KeyboardAction>("PreviousTrack", VK_MEDIA_PREV_TRACK, KEYEVENTF_EXTENDEDKEY)
    },
    {
        NEXT_TRACK, CreateGlobalGenerator<KeyboardAction>("NextTrack", VK_MEDIA_NEXT_TRACK, KEYEVENTF_EXTENDEDKEY)
    },
    {
        VOLUME_MUTE, CreateGlobalGenerator<KeyboardAction>("GlobalVolumeMute", VK_VOLUME_MUTE, KEYEVENTF_EXTENDEDKEY)
    },
    {
        VOLUME_SET, CreateGlobalGenerator<GlobalVolumeSet>()
    }
};

const std::unordered_map<std::string, Generator> typeMapSession =
{
    {
        VOLUME_MUTE, [](const std::string& app, IAudioSessionControl2* session)
            {
                return std::make_unique<SessionVolumeMute>(app, session);
            }
    },
    {
        VOLUME_SET, [](const std::string& app, IAudioSessionControl2* session)
            {
                return std::make_unique<SessionVolumeSet>(app, session);
            }
    }
};

std::unique_ptr<IAction> action::GetGlobalAction(const std::string& name)
{
    return typeMapGlobal.at(name)();
}

const Generator& action::GetGenerator(const std::string& name)
{
    return typeMapSession.at(name);
}
