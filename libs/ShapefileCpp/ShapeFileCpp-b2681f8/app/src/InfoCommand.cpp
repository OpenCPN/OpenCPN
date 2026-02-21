#include "InfoCommand.hpp"

InfoCommand::InfoCommand(CLI::App* app): Command ("info") {
    CLI::App* cmd = app->add_subcommand("info", "Get information on a Shapefile");
    cmd->add_option("-f", options.fileName, "Shapefile file")->required();
}

void InfoCommand::execute(std::istream& istream, std::ostream& ostream) {
    shp::ShapefileReader shp {options.fileName};
    ostream << "# Features = " << shp.getCount() << std::endl;
    ostream << "Bounds = " << shp.getBounds() << std::endl;
    ostream << "Geometry Type = " << shp.getGeometryType() << std::endl;
    ostream << "Schema:" << std::endl;
    for(auto field : shp.getFields()) {
        ostream << "  " << field.getName() << " = " << field.getType() << " (width = " << field.getWidth() << ", number of decimals = " << field.getNumberOfDecimals() << ")" << std::endl;
    }
}
