#include "FeatureIterator.hpp"

namespace shp {

    FeatureIterator::FeatureIterator(ShapefileReader& shp, int i) : shapefile(shp), index(i) {}

    FeatureIterator FeatureIterator::operator++() { 
        ++index;
        return *this;
    
    }
    Feature FeatureIterator::operator++(int) { 
        Feature featureHolder {shapefile.getFeature(index)};
        ++*this;
        return featureHolder; 
    }

    Feature FeatureIterator::operator*() const { 
        return shapefile.getFeature(index); 
    }

    bool FeatureIterator::operator==(const FeatureIterator& rhs) { 
        return index == rhs.index; 
    }
    
    bool FeatureIterator::operator!=(const FeatureIterator& rhs) { 
        return index != rhs.index; 
    }

}