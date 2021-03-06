#include <QVector3D>
#include "coverage.h"
#include "featurecoverage.h"
#include "feature.h"
#include "geometryhelper.h"
#include "drawers/attributevisualproperties.h"
#include "drawers/drawerattributesetterfactory.h"
#include "simplelinesetter.h"

using namespace Ilwis;
using namespace Geodrawer;

REGISTER_DRAWER_ATTRIBUTE_SETTER(SimpleLineSetter)

SimpleLineSetter::SimpleLineSetter(const IOOptions &options) : BaseSpatialAttributeSetter(options)
{

}

SimpleLineSetter::~SimpleLineSetter()
{

}

DrawerAttributeSetter *SimpleLineSetter::create(const IOOptions &options)
{
    return new SimpleLineSetter(options);

}

FeatureDrawing SimpleLineSetter::setSpatialAttributes(const Ilwis::SPFeatureI &feature,
                                               QVector<QVector3D> &vertices,
                                               QVector<QVector3D> &) const
{
    const UPGeometry& geometry = feature->geometry();
    FeatureDrawing drawing(itLINE);
    int n = geometry->getNumGeometries();
    for(int  geom = 0; geom < n; ++geom ){
        const geos::geom::Geometry *subgeom = geometry->getGeometryN(geom);
        if (!subgeom)
            continue;
        auto *coords = subgeom->getCoordinates();
        quint32 oldend = vertices.size();
        drawing._indices.push_back(VertexIndex(oldend, coords->size(), GL_LINE_STRIP, feature->featureid()));
        vertices.resize(oldend + coords->size());

        Coordinate crd;
        for(int i = 0; i < coords->size(); ++i){
            coords->getAt(i, crd);
            if ( coordinateConversionNeeded()){
                crd = _targetSystem->coord2coord(_sourceSystem, crd);
            }
            vertices[oldend + i] = QVector3D(crd.x, crd.y, crd.z);
        }
        delete coords;
    }
    return drawing;
}

void SimpleLineSetter::setColorAttributes(const VisualAttribute &attr, const QVariant &value, const FeatureDrawing& drawing, std::vector<VertexColor> &colors) const
{
    for(int j =0; j < drawing._indices.size(); ++j){
        if ( value.isValid() && value.toInt() != iUNDEF) {
            const auto& indices = drawing[j];
            for(int i =indices._start; i < indices._start + indices._count; ++i){
                QColor clr = attr.value2color(value);
                colors.push_back(VertexColor(clr.redF(), clr.greenF(), clr.blueF(), 1.0));
            }
        }
    }
}

