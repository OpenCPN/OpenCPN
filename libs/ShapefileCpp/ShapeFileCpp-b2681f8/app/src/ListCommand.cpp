#include "ListCommand.hpp"

ListCommand::ListCommand(CLI::App* app, Commands* cmds): Command ("list") {
    CLI::App* cmd = app->add_subcommand("list", "List all commands");
    commands = cmds;
}

void ListCommand::execute(std::istream& istream, std::ostream& ostream) {
    for(auto cmd : commands->get()) {
        ostream << cmd->getName() << std::endl;
    }
}
