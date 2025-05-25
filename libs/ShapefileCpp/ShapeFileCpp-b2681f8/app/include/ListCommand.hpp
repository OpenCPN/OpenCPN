#pragma once

#include "CLI/CLI.hpp"
#include "Command.hpp"
#include "Commands.hpp"

class ListOptions {
};

class ListCommand : public Command {

    public:

        ListCommand(CLI::App* app, Commands* commands);

        void execute(std::istream& istream, std::ostream& ostream);

    private:

        ListOptions options;

        Commands* commands;
};
