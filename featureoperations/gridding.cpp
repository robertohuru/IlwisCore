#include <functional>
#include <future>
#include "kernel.h"
#include "coverage.h"
#include "numericrange.h"
#include "numericdomain.h"
#include "columndefinition.h"
#include "table.h"
#include "attributerecord.h"
#include "polygon.h"
#include "geometry.h"
#include "feature.h"
#include "factory.h"
#include "abstractfactory.h"
#include "featurefactory.h"
#include "featurecoverage.h"
#include "featureiterator.h"
#include "symboltable.h"
#include "OperationExpression.h"
#include "operationmetadata.h"
#include "operation.h"
//#include "commandhandler.h"
#include "gridding.h"

using namespace Ilwis;
using namespace FeatureOperations;

Gridding::Gridding()
{
}

Gridding::Gridding(quint64 metaid, const Ilwis::OperationExpression &expr) : OperationImplementation(metaid, expr)
{

}

bool Gridding::execute(ExecutionContext *ctx, SymbolTable &symTable)
{
    if (_prepState == sNOTPREPARED)
        if((_prepState = prepare(ctx, symTable)) != sPREPARED)
            return false;

    for(int fx=0; fx < _xsize; ++fx) {
        for(int fy=0; fy < _ysize; ++fy) {
            Coordinate2d c1(_top + std::vector<double>{_cellXSize * fx, _cellYSize * fy });
            Coordinate2d c2(_top +std::vector<double>{_cellXSize * (fx+1), _cellYSize * fy });
            Coordinate2d c3(_top + std::vector<double>{_cellXSize * (fx+1), _cellYSize * (fy+1) });
            Coordinate2d c4(_top + std::vector<double>{_cellXSize * fx, _cellYSize * (fy+1) });
            Polygon pol;
            pol.outer().push_back(c1);
            pol.outer().push_back(c2);
            pol.outer().push_back(c3);
            pol.outer().push_back(c4);
            pol.outer().push_back(c1);
            _outfeatures->newFeature({pol});
        }
    }

    if ( ctx != 0) {
        QVariant value;
        value.setValue<IFeatureCoverage>(_outfeatures);
        ctx->addOutput(symTable, value, _outfeatures->name(), itFEATURE,_outfeatures->source());
    }
    return true;
}

OperationImplementation *Gridding::create(quint64 metaid, const Ilwis::OperationExpression &expr)
{
    return new  Gridding(metaid, expr)   ;
}

quint64 Gridding::createMetadata()
{
    QString url = QString("ilwis://operations/gridding");
    Resource resource(QUrl(url), itOPERATIONMETADATA);
    resource.addProperty("namespace","ilwis");
    resource.addProperty("longname","gridding");
    resource.addProperty("syntax","gridding(coordinatesyste,top-coordinate,x-cell-size, y-cell-size, horizontal-cells, vertical-cells)");
    resource.addProperty("description",TR("generates a new featurecoverage(polygons) were the polygons form a rectangular grid"));
    resource.addProperty("inparameters","6");
    resource.addProperty("pin_1_type", itCOORDSYSTEM);
    resource.addProperty("pin_1_name", TR("coordinate-syste,"));
    resource.addProperty("pin_1_desc",TR("The coordinate system of the to be created polygon coverage"));
    resource.addProperty("pin_2_type", itCOORDINATE);
    resource.addProperty("pin_2_name", TR("top corner"));
    resource.addProperty("pin_2_desc",TR("The top corner of the polygonmap expressed in coordinates of the coordinate system"));
    resource.addProperty("pin_3_type", itDOUBLE);
    resource.addProperty("pin_3_name", TR("X cell size"));
    resource.addProperty("pin_3_desc",TR("Size in the x direction of a cell in the grid expressed in untis of the coordinate system"));
    resource.addProperty("pin_4_type", itDOUBLE);
    resource.addProperty("pin_4_name", TR("Y cell size"));
    resource.addProperty("pin_4_desc",TR("Size in the y direction of a cell in the grid expressed in untis of the coordinate system"));
    resource.addProperty("pin_5_type", itINTEGER);
    resource.addProperty("pin_5_name", TR("Horizontal cells"));
    resource.addProperty("pin_5_desc",TR("Number of cells in the x directions"));
    resource.addProperty("pin_6_type", itINTEGER);
    resource.addProperty("pin_6_name", TR("Vertical cells"));
    resource.addProperty("pin_6_desc",TR("Number of cells in the y directions"));
    resource.addProperty("outparameters",1);
    resource.addProperty("pout_1_type", itPOLYGON);
    resource.addProperty("pout_1_name", TR("output polygon coverage"));
    resource.addProperty("pout_1_desc",TR("output polygon coverage"));

    resource.prepare();
    url += "=" + QString::number(resource.id());
    resource.setUrl(url);

    mastercatalog()->addItems({resource});
    return resource.id();
}

OperationImplementation::State Gridding::prepare(ExecutionContext *ctx, const SymbolTable &symTable)
{
    QString csyName =  _expression.parm(0).value();
    if (!_csy.prepare(csyName)) {
        ERROR2(ERR_COULD_NOT_LOAD_2,csyName,"" );
        return sPREPAREFAILED;
    }

    QString name = _expression.parm(1).value();
    QVariant var = symTable.getValue(name);
    _top = var.value<Coordinate2d>();
    if (!_top.isValid() || _top.is0()) {
        _top = var.value<Coordinate>();
    }
    if (!_top.isValid() || _top.is0()) {
        ERROR2(ERR_ILLEGAL_VALUE_2,"parameter value",name);
        return sPREPAREFAILED;
    }
    bool ok;
    _cellXSize = _expression.parm(2).value().toDouble(&ok);
    if ( !ok ) {
        ERROR2(ERR_ILLEGAL_VALUE_2,"parameter value","3");
        return sPREPAREFAILED;
    }

    _cellYSize = _expression.parm(3).value().toDouble(&ok);
    if ( !ok ) {
        ERROR2(ERR_ILLEGAL_VALUE_2,"parameter value","4");
        return sPREPAREFAILED;
    }

    _xsize = _expression.parm(4).value().toDouble(&ok);
    if ( !ok ) {
        ERROR2(ERR_ILLEGAL_VALUE_2,"parameter value","5");
        return sPREPAREFAILED;
    }

    _ysize = _expression.parm(5).value().toDouble(&ok);
    if ( !ok ) {
        ERROR2(ERR_ILLEGAL_VALUE_2,"parameter value","6");
        return sPREPAREFAILED;
    }
    QString outputName = _expression.parm(0,false).value();
    QString url = "ilwis://internalcatalog/" + outputName;
    Resource resource(url, itFLATTABLE);
    _attTable.prepare(resource);
    IDomain covdom;
    if (!covdom.prepare("count")){
        return sPREPAREFAILED;
    }
    _attTable->addColumn(FEATUREIDCOLUMN,covdom);
    _outfeatures.prepare();
    _outfeatures->setCoordinateSystem(_csy);
    _outfeatures->attributeTable(_attTable);
    Box2D<double> env(_top, _top + std::vector<double>{_cellXSize * _xsize, _cellYSize * _ysize });
    _outfeatures->envelope(env);


    return sPREPARED;
}