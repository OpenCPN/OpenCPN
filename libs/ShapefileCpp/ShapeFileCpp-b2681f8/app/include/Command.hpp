#pragma once

#include <string>
#include <iostream>

class Command {

    public:

        Command(std::string name);

        std::string getName() const;

        virtual void execute(std::istream& istream, std::ostream& ostream) = 0;

    private:

        std::string name;

};
