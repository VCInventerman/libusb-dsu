#pragma once

#include <map>
#include <string>
#include <string_view>

template <typename ContainerT, typename ElmT, typename CompT = std::equal_to<ElmT>>
Size findNthChar(const ContainerT& container, const ElmT elm, Size target)
{
    CompT comp;
    Size found = 0;

    for (ContainerT::const_iterator i = container.cbegin(); i != container.cend(); i++)
    {
        if (comp(*i, elm) && (++found == target))
        {
            return std::distance(i, begin);
        }
    }

    return std::string::npos;
}

class CommandEval
{
public:
    using This = CommandEval;

    enum class ReturnCode
    {
        Success = 0,
        InvalidInput = -1,
        CommandNotFound = -2,
        Error = -3,
    };

    std::map<std::string_view, std::function<ReturnCode(std::string_view)>> commands;

    CommandEval() = default;

    bool addCommand(std::string_view name, std::function<ReturnCode(std::string_view)> func)
    {
        return commands.emplace(name, func).second;
    }

    ReturnCode exec(const std::string_view command)
    {
        std::string_view commandName = command.substr(0, command.find(' '));

        auto itr = commands.find(commandName);
        if (itr != commands.cend())
        {
            return itr->second(command);
        }
        else
        {
            return ReturnCode::CommandNotFound;
        }
    }
};