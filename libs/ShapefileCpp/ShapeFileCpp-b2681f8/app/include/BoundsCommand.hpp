#pragma once

#include <string>
#include "CLI/CLI.hpp"
#include "Command.hpp"
#include "ShapefileReader.hpp"

class BoundsOptions {
    public:
        std::string fileName;
};

class BoundsCommand : public Command {

    public:

        BoundsCommand(CLI::App* app);

        void execute(std::istream& istream, std::ostream& ostream);

    private:

        BoundsOptions options;

};
