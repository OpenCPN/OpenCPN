#pragma once

#include <iterator>
#include "ShapefileReader.hpp"

namespace shp {

    class ShapefileReader;

    class FeatureIterator {

        private:

            ShapefileReader& shapefile;

            int index;

        public:

            FeatureIterator(ShapefileReader& shp, int i);

            typedef Feature value_type;
            typedef std::ptrdiff_t difference_type;
            typedef Feature* pointer;
            typedef Feature& reference;
            typedef std::input_iterator_tag iterator_category;

            FeatureIterator operator++();
            Feature operator++(int);
            Feature operator*() const;
            bool operator==(const FeatureIterator& rhs);
            bool operator!=(const FeatureIterator& rhs);

    };

}
