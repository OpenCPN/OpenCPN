#pragma once

#include <vector>
#include "Command.hpp"

class Commands {

    private:

        std::vector<Command*> commands;

    public:

        void add(Command* cmd);

        std::vector<Command*> get();

};
