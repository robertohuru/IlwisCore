#include "coverage.h"
#include "featurecoverage.h"
#include "feature.h"
#include "featureiterator.h"
#include "drawingcolor.h"
#include "drawerfactory.h"
#include "rootdrawer.h"
#include "table.h"
#include "range.h"
#include "itemrange.h"
#include "identifieritem.h"
#include "identifierrange.h"
#include "colorrange.h"
#include "colorlookup.h"
#include "itemdomain.h"
#include "representation.h"
#include "attributevisualproperties.h"
#include "featurelayerdrawer.h"
#include "tesselation/ilwistesselator.h"
#include "openglhelper.h"

using namespace Ilwis;
using namespace Geodrawer;

REGISTER_DRAWER(FeatureLayerDrawer)

FeatureLayerDrawer::FeatureLayerDrawer(DrawerInterface *parentDrawer, RootDrawer *rootdrawer, const IOOptions &options) : LayerDrawer("FeatureLayerDrawer", parentDrawer, rootdrawer, options)
{
}

DrawerInterface *FeatureLayerDrawer::create(DrawerInterface *parentDrawer, RootDrawer *rootdrawer, const IOOptions &options)
{
    return new FeatureLayerDrawer(parentDrawer, rootdrawer, options)    ;
}


bool FeatureLayerDrawer::prepare(DrawerInterface::PreparationType prepType, const IOOptions &options)
{
    if(!LayerDrawer::prepare(prepType, options))
        return false;

    if ( hasType(prepType, DrawerInterface::ptGEOMETRY) && !isPrepared(DrawerInterface::ptGEOMETRY)){



        IFeatureCoverage features = coverage().as<FeatureCoverage>();
        if ( !features.isValid()){
            return ERROR2(ERR_COULDNT_CREATE_OBJECT_FOR_2,"FeatureCoverage", TR("Visualization"));
        }
        // set all to 0
        _indices = std::vector<VertexIndex>();
        _vertices = QVector<QVector3D>();
        _normals = QVector<QVector3D>();
        // get a description of how to render
        AttributeVisualProperties attr = visualAttribute(activeAttribute());
        int columnIndex = features->attributeDefinitions().columnIndex(activeAttribute());
        if ( columnIndex == iUNDEF){ // test a number of fallbacks to be able to show at least something
            columnIndex = features->attributeDefinitions().columnIndex(COVERAGEKEYCOLUMN);
            attr =  visualAttribute(COVERAGEKEYCOLUMN);
            if ( columnIndex == iUNDEF){
                columnIndex = features->attributeDefinitions().columnIndex(FEATUREVALUECOLUMN);
                attr =  visualAttribute(FEATUREVALUECOLUMN);
                if ( columnIndex == iUNDEF){ // default to a indexeditemdomain
                    IIndexedIdDomain itemdom;
                    itemdom.prepare();
                    IndexedIdentifierRange *rng = new IndexedIdentifierRange("feature", features->featureCount());
                    itemdom->range(rng);
                    attr = AttributeVisualProperties(itemdom);
                }
            }
        }
        Raw raw = 0;
        for(const SPFeatureI& feature : features){
            QVariant value =  columnIndex != iUNDEF ? feature(columnIndex) : raw++;
            if ( value.isValid() && value.toInt() != iUNDEF) {
                quint32 noOfVertices = OpenGLHelper::getVertices(rootDrawer()->coordinateSystem(),
                                                                 features->coordinateSystem(),
                                                                 feature->geometry(),
                                                                 feature->featureid(),
                                                                 _vertices,
                                                                 _normals,
                                                                 _indices,
                                                                 _boundaryIndex);
                for(int i =0; i < noOfVertices; ++i){
                    if ( _boundaryIndex == iUNDEF || i < _boundaryIndex){
                        QColor clr = attr.value2color(value);
                        _colors.push_back(VertexColor(clr.redF(), clr.greenF(), clr.blueF(), 1.0));
                    }else {
                        _colors.push_back(VertexColor(128,0,128,1));
                    }
                }
            }
        }
        _prepared |= DrawerInterface::ptGEOMETRY;

    }

    //initialize();
    return true;
}

void FeatureLayerDrawer::unprepare(DrawerInterface::PreparationType prepType)
{
    LayerDrawer::unprepare(prepType);

    if ( hasType(prepType, DrawerInterface::ptGEOMETRY))    {
        _prepared &= ~ ptGEOMETRY;
    }
}

void FeatureLayerDrawer::setActiveVisualAttribute(const QString &attr)
{
    IFeatureCoverage features = coverage().as<FeatureCoverage>();
    if ( features.isValid())    {
        if ( features->attributeDefinitions().columnIndex(attr) != iUNDEF){

            IRepresentation newrpr = Representation::defaultRepresentation(features->attributeDefinitions().columndefinition(attr).datadef().domain());
            if ( newrpr.isValid()){
                LayerDrawer::setActiveVisualAttribute(attr);
            }
        }
    }
}

void FeatureLayerDrawer::coverage(const ICoverage &cov)
{
    LayerDrawer::coverage(cov);
    setActiveVisualAttribute(sUNDEF);
    IFeatureCoverage features = coverage().as<FeatureCoverage>();

    for(int i = 0; i < features->attributeDefinitions().definitionCount(); ++i){
        IlwisTypes attrType = features->attributeDefinitions().columndefinition(i).datadef().domain()->ilwisType();
        if ( hasType(attrType, itNUMERICDOMAIN | itITEMDOMAIN)){
            AttributeVisualProperties props(features->attributeDefinitions().columndefinition(i).datadef().domain());
            if ( attrType == itNUMERICDOMAIN){
                SPNumericRange numrange = features->attributeDefinitions().columndefinition(i).datadef().range<NumericRange>();
                props.actualRange(NumericRange(numrange->min(), numrange->max(), numrange->resolution()));
            } else if ( attrType == itITEMDOMAIN){
                int count = features->attributeDefinitions().columndefinition(i).datadef().domain()->range<>()->count();
                props.actualRange(NumericRange(0, count - 1,1));
            }
            visualAttribute(features->attributeDefinitions().columndefinition(i).name(), props);
            // try to find a reasonable default for the activeattribute
            if ( activeAttribute() == sUNDEF){
                if ( features->attributeDefinitions().columnIndex(FEATUREVALUECOLUMN) != iUNDEF){
                    setActiveVisualAttribute(FEATUREVALUECOLUMN);
                }else if ( features->attributeDefinitions().columnIndex(COVERAGEKEYCOLUMN) != iUNDEF){
                    setActiveVisualAttribute(COVERAGEKEYCOLUMN);
                }
                else if ( hasType(features->attributeDefinitions().columndefinition(i).datadef().domain()->ilwisType(), itNUMERICDOMAIN)){
                    setActiveVisualAttribute(features->attributeDefinitions().columndefinition(i).name());
                }
            }
        }
    }
}

ICoverage FeatureLayerDrawer::coverage() const
{
    return SpatialDataDrawer::coverage();
}

DrawerInterface::DrawerType FeatureLayerDrawer::drawerType() const
{
    return DrawerInterface::dtMAIN;
}


bool FeatureLayerDrawer::draw(const IOOptions& )
{
    if ( !isActive())
        return false;

    if (!isPrepared()){
        return false;
    }

    if(!_shaders.bind())
        return false;

    _shaders.setUniformValue(_modelview, rootDrawer()->mvpMatrix());
    _shaders.enableAttributeArray(_vboNormal);
    _shaders.enableAttributeArray(_vboPosition);
    _shaders.enableAttributeArray(_vboColor);
    _shaders.setAttributeArray(_vboPosition, _vertices.constData());
    _shaders.setAttributeArray(_vboNormal, _normals.constData());
    _shaders.setAttributeArray(_vboColor, GL_FLOAT, (void *)_colors.data(),4);
    for(int i =0; i < _indices.size(); ++i){
       if ( _indices[i]._geomtype == itLINE){
           glDrawArrays(GL_LINE_STRIP,_indices[i]._start,_indices[i]._count);
       } else if ( _indices[i]._geomtype == itPOLYGON ){
           glDrawArrays(GL_TRIANGLE_FAN,_indices[i]._start,_indices[i]._count);
       }
   }
    _shaders.disableAttributeArray(_vboNormal);
    _shaders.disableAttributeArray(_vboPosition);
    _shaders.disableAttributeArray(_vboColor);
    _shaders.release();

    return true;
}


