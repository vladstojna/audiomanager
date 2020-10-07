#pragma once

#include "IAction.h"

#include <functional>

struct IAudioSessionControl2;

namespace action
{
    template<typename T, typename... Args>
    using GeneratorT = std::function<std::unique_ptr<T>(Args...)>;

    using Generator = GeneratorT<IAction, const std::string&, IAudioSessionControl2*>;

    std::unique_ptr<IAction> GetGlobalAction(const std::string& name);

    const Generator& GetGenerator(const std::string& name);

}
