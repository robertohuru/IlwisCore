#ifndef COORDINATESYSTEMVIALATLON_H
#define COORDINATESYSTEMVIALATLON_H

#include "kernel_global.h"

namespace Ilwis {

class Ellipsoid;
class Projection;
class GeodeticDataum;

typedef IlwisData<Ellipsoid> IEllipsoid;
typedef IlwisData<Projection> IProjection;


class KERNELSHARED_EXPORT ConventionalCoordinateSystem : public CoordinateSystem
{
public:
    ConventionalCoordinateSystem();
    ConventionalCoordinateSystem(const Resource& resource);
    ~ConventionalCoordinateSystem();

    Coordinate coord2coord(const ICoordinateSystem &sourceCs, const Coordinate& crdSource) const;
    LatLon coord2latlon(const Coordinate &crdSource) const;
    Coordinate latlon2coord(const LatLon& ll) const;
    const std::unique_ptr<Ilwis::GeodeticDatum> &datum() const;
    void setDatum(Ilwis::GeodeticDatum *datum);
    IEllipsoid ellipsoid() const;
    void setEllipsoid(const IEllipsoid& ell);
    void setProjection(const IProjection& proj);
    IProjection projection() const;
    virtual bool isLatLon() const;
    bool prepare(const QString& parms);
    bool prepare();
    IlwisTypes ilwisType() const;
    bool isValid() const;
    bool isEqual(const Ilwis::IlwisObject *obj) const;
    bool isUnknown() const;
    bool canConvertToLatLon() const;
    bool canConvertToCoordinate() const;
    QString toWKT(quint32 spaces=0) const;
    QString toProj4() const;
    QString unit() const;
    void unit(const QString &unit);
    bool isCompatibleWith(const IlwisObject *obj) const;
private:
    IEllipsoid _ellipsoid;
    IProjection _projection;
    std::unique_ptr<GeodeticDatum> _datum;
    QString _unit;
};

typedef IlwisData<ConventionalCoordinateSystem> IConventionalCoordinateSystem;

}

#endif // COORDINATESYSTEMVIALATLON_H
