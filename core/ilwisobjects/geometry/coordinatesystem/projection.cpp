#include <QString>
#include <functional>

#include "kernel.h"
#include "ilwis.h"
#include "geometries.h"
#include "ilwisobject.h"
#include "ilwisdata.h"
#include "ellipsoid.h"
#include "geodeticdatum.h"
#include "projection.h"
#include "projectionimplementation.h"
#include "proj4parameters.h"
#include "box.h"
#include "coordinatesystem.h"
#include "conventionalcoordinatesystem.h"


using namespace Ilwis;

std::map<QString, QString> Projection::_projcode2Name;


Projection::Projection() : _wkt(sUNDEF), _authority(sUNDEF)
{
    if ( _projcode2Name.size() == 0){
        QSqlQuery db(kernel()->database());
        QString query = "Select code, name from projection";
        db.exec(query);
        while(db.next()){
            QString code = db.value(0).toString();
            QString nme = db.value(1).toString();
            _projcode2Name[code] = nme;
        }
    }
}

Projection::~Projection()
{
}

Coordinate Projection::latlon2coord(const LatLon &ll) const
{
    if ( _implementation.isNull()) {
        ERROR1(ERR_NO_INITIALIZED_1, name());

        return crdUNDEF;
    }
    return _implementation->latlon2coord(ll);
}

LatLon Projection::coord2latlon(const Coordinate &crd) const
{
    return _implementation->coord2latlon(crd);

}

bool Projection::prepare(const QString &parms)
{
    return _implementation->prepare(parms);
}

bool Projection::prepare()
{
    return IlwisObject::prepare();
}

void Projection::setImplementation(ProjectionImplementation *impl)
{
    _implementation.reset(impl);
}


QVariant Projection::parameter(Projection::ProjectionParamValue type) const
{
    if ( _implementation.isNull()) {
        ERROR1(ERR_NO_INITIALIZED_1, name());
        return QVariant();
    }
    return _implementation->parameter(type);
}

bool Projection::isSet(Projection::ProjectionParamValue type) const
{
    if ( _implementation.isNull()) {
        ERROR1(ERR_NO_INITIALIZED_1, name());
        return false;
    }
    return _implementation->isSet(type);
}

void Projection::setParameter(Projection::ProjectionParamValue type, const QVariant &value)
{
    if ( _implementation.isNull()) {
        ERROR1(ERR_NO_INITIALIZED_1, name());
        return;
    }
    _implementation->setParameter(type, value);
}

QString Projection::parameterName(Projection::ProjectionParamValue pv) const
{
    switch (pv){
    case pvX0:
        return "false easting";
    case pvY0:
        return "false northing";
    case pvLAT0:
        return "latitude of origin";
    case pvLAT1:
        return "standard parallel 1";
    case pvLAT2:
        return "standard parallel 2";
    case pvLATTS:
        return "latitude of true scale";
    case pvLON0:
        return "central meridian";
    case pvK0:
        return "scale factor";
    case pvZONE:
        return "zone";

     default:
        break;
    }
    return "";
}

QString Projection::projectionCode2Name(const QString &code)
{
    auto iter = _projcode2Name.find(code);
    if ( iter != _projcode2Name.end())
         return iter->second;
    return sUNDEF;
}

QString Projection::authority() const
{
    return _authority;
}

void Projection::setAuthority(const QString& auth){
    _authority = auth;
}

QString Projection::wkt() const
{
    return _wkt;
}

void Projection::setWkt(const QString &wkt)
{
    _wkt = wkt;
}

QString Projection::toProj4() const
{
    if ( _implementation.isNull()) {
        ERROR1(ERR_NO_INITIALIZED_1, name());
        return sUNDEF;
    }
    return _implementation->toProj4();
}

IlwisTypes Projection::ilwisType() const
{
    return itPROJECTION;
}

QString Projection::toWKT(quint32 spaces)
{
  if ( _implementation.isNull()) {
      return sUNDEF;
  }
  QString proj = "PROJECTION[\"" + _wkt + "\"],";
  proj += _implementation->toWKT(spaces);
  return proj;
}

void Projection::setCoordinateSystem(ConventionalCoordinateSystem *csy)
{
    if ( _implementation.isNull()) {
        ERROR1(ERR_NO_INITIALIZED_1, name());
        return;
    }
    _implementation->setCoordinateSystem(csy);
}

Projection::FindMatch Projection::find(const QString &v1, const QString &value)
{
    if ( v1 == value)
        return fmMatch;
    return fmNoValue;
}


bool Projection::isEqual(const IlwisObject *obj) const
{
    if ( !obj || !hasType(obj->ilwisType(), itPROJECTION))
        return false;

    if(id() == obj->id())
        return true;

    const Projection *prj = static_cast<const Projection *>(obj);
    if ( !_implementation.isNull()) {
        return _implementation->isEqual(prj->_implementation)    ;
    }
    return false;
}

bool Projection::isValid() const
{
    return !_implementation.isNull();
}

bool Projection::canConvertToLatLon() const
{
    if ( isValid())    {
        return _implementation->canConvertToLatLon();
    }
    return false;
}

bool Projection::canConvertToCoordinate() const
{
    if ( isValid())    {
        return _implementation->canConvertToCoordinate();
    }
    return false;
}





