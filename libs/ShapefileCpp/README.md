[![CI](https://github.com/jericks/ShapefileCpp/actions/workflows/ci.yml/badge.svg)](https://github.com/jericks/ShapefileCpp/actions/workflows/ci.yml)

ShapefileCpp
============

A C++ Shapefile API the wraps the C [shapelib](http://shapelib.maptools.org/) library.

Create a new Shapefile:

```cpp
shp::Schema schema { shp::GeometryType::Point, std::vector<shp::Field> {
    shp::Field {"id", shp::FieldType::Integer, 3, 0}, 
    shp::Field {"name", shp::FieldType::String, 100, 0}
}};
shp::ShapefileWriter shp {std::filesystem::absolute(path), schema};

std::unique_ptr<shp::Geometry> pt = std::make_unique<shp::Point>(123.45, -47.65);
std::map<std::string, std::any> props {{"id",1},{"name", std::string{"Seattle"}}};
const shp::Feature feature = {std::move(pt), props};
shp.write(feature);
```

Read an existing Shapefile:

```cpp
shp::ShapefileReader shp {std::filesystem::absolute("points.shp")};
shp::Bounds b = shp.getBounds();
std::cout << "Bounds = " << b << "\n";
int numberOfFeatures = shp.getCount();
std::cout << "Number of Features = " << numberOfFeatures << "\n";
for(auto const& feature : shp) {
    std:cout << f.getGeometry().wkt() << "\n";
}
```

Read features using a lambda:

```cpp
shp::ShapefileReader shp {std::filesystem::absolute("points.shp")};
shp.features([&](shp::Feature f) {
  std:cout << f.getGeometry().wkt() << "\n";
});
```

Command line interface
----------------------

ShapefileCpp has a simple command line tool called **shp-cli**.

List all of the shp-cli subcommands.

```bash
% shp-cli list
```

```
bounds
count
list
random
```

Create a Shapefile with random points.

```
% shp-cli random -f points.shp -n 10
```

Get the bounds of the Shapefile.

```bash
% shp-cli bounds -f points.shp
```

```
POLYGON ((-151.378 -80.1131, 179.342 -80.1131, 179.342 75.8812, -151.378 -80.1131))
```

Count the number of features in a Shapefile.

```bash
% shp-cli count  -f points.shp
```

```
10
```

Get information from a Shapefile.

```bash
% shp-cli info -f points.shp
```

```
# Features = 10
Bounds = BOUNDS (-158.899, -158.899, 171.62, 79.2818)
Geometry Type = Point
Schema:
  id = Integer (width = 9, number of decimals = 0)
  name = String (width = 254, number of decimals = 0)
```

Build
-----

```bash
mkdir build
cd build
conan install ..
cmake ..
make
make test
```

