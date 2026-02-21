#include "Commands.hpp"

void Commands::add(Command* cmd) {
    commands.push_back(cmd);
}

std::vector<Command*> Commands::get() {
    return commands;
}
