#include "BoundsCommand.hpp"

BoundsCommand::BoundsCommand(CLI::App* app): Command ("bounds") {
    CLI::App* cmd = app->add_subcommand("bounds", "Bounds the number of features in a Shapefile");
    cmd->add_option("-f", options.fileName, "Shapefile file")->required();
}

void BoundsCommand::execute(std::istream& istream, std::ostream& ostream) {
    shp::ShapefileReader shp {options.fileName};
    ostream << shp.getBounds().toPolygon()->wkt() << std::endl;
}
