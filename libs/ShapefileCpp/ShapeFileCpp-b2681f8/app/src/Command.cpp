#include "Command.hpp"

Command::Command(std::string commandName) {
    name = commandName;
};

std::string Command::getName() const {
    return name;
};
