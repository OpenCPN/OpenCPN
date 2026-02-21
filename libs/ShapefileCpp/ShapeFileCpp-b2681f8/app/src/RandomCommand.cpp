#include "RandomCommand.hpp"

RandomCommand::RandomCommand(CLI::App* app): Command ("random") {
    CLI::App* cmd = app->add_subcommand("random", "Create a new shapefile with a number of random points");
    cmd->add_option("-f", options.fileName, "Shapefile file")->required();
    cmd->add_option("-n", options.number, "Number of random points")->required();
}

void RandomCommand::execute(std::istream& istream, std::ostream& ostream) {
    shp::Schema schema { shp::GeometryType::Point, std::vector<shp::Field> {
        shp::Field {"id", shp::FieldType::Integer, 3, 0}
    }};
    shp::ShapefileWriter shp {std::filesystem::absolute(options.fileName), schema};

    shp::Bounds bounds = shp::Bounds {-180,-90,180,90};
    double width = abs(bounds.getMinX()) + abs(bounds.getMaxX());
    double height = abs(bounds.getMinY()) + abs(bounds.getMaxY());

    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_real_distribution<double> distr(0, 1);

    for (int i = 0; i < options.number; i++) {
        std::unique_ptr<shp::Geometry> pt = std::make_unique<shp::Point>(
            bounds.getMinX() + (distr(eng) * width),
            bounds.getMinY() + (distr(eng) * height)
        );
        std::map<std::string, std::any> props {{"id", i}};
        const shp::Feature feature = {std::move(pt), props};
        shp.write(feature);
    }
}
