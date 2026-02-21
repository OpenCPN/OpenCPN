#pragma once

#include <string>
#include "CLI/CLI.hpp"
#include "Command.hpp"
#include "ShapefileReader.hpp"

class CountOptions {
    public:
        std::string fileName;
};

class CountCommand : public Command {

    public:

        CountCommand(CLI::App* app);

        void execute(std::istream& istream, std::ostream& ostream);

    private:

        CountOptions options;

};
