#include "CountCommand.hpp"

CountCommand::CountCommand(CLI::App* app): Command ("count") {
    CLI::App* cmd = app->add_subcommand("count", "Count the number of features in a Shapefile");
    cmd->add_option("-f", options.fileName, "Shapefile file")->required();
}

void CountCommand::execute(std::istream& istream, std::ostream& ostream) {
    shp::ShapefileReader shp {options.fileName};
    ostream << shp.getCount() << std::endl;
}
