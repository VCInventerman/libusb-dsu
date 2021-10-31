#pragma once

#include <map>
#include <string>
#include <string_view>

class CommandEval
{
public:
    using This = CommandEval;

    enum class ReturnCode
    {
        Success = 0,
        InvalidString = -1,
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