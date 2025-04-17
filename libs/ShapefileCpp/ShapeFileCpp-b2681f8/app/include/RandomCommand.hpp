#pragma once

#include <string>
#include <stdlib.h>
#include <random>
#include "CLI/CLI.hpp"
#include "Command.hpp"
#include "ShapefileWriter.hpp"
#include "Bounds.hpp"

class RandomOptions {
    public:
        std::string fileName;
        int number;
};

class RandomCommand : public Command {

    public:

        RandomCommand(CLI::App* app);

        void execute(std::istream& istream, std::ostream& ostream);

    private:

        RandomOptions options;

};
