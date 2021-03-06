#ifndef PROJECTION_H
#define PROJECTION_H

#include "kernel_global.h"

namespace Ilwis {

class ProjectionImplementation;
class ConventionalCoordinateSystem;

class KERNELSHARED_EXPORT Projection : public IlwisObject
{
public:
     enum ProjectionParamValue { pvNONE, pvX0, pvY0, pvLON0,
     pvLATTS, pvLAT0, pvK0, pvNORTH,
     pvZONE, pvLAT1, pvLAT2, pvHEIGHT,
     pvTILTED, pvTILT, pvAZIMYAXIS, pvAZIMCLINE,
     pvPOLE, pvNORIENTED, pvELLCODE, pvLAST };

    enum FindMatch{fmNoItem, fmNoValue,fmMatch};

    Projection();
    virtual ~Projection();

    virtual Coordinate latlon2coord(const LatLon&) const;
    virtual LatLon coord2latlon(const Coordinate&) const;

    bool prepare(const QString& parms);
    bool prepare();
    void setImplementation(ProjectionImplementation *impl);

    QString authority() const;
    void setAuthority(const QString &auth);
    QString wkt() const;
    void setWkt(const QString &wkt);
    QString toProj4() const;

    IlwisTypes ilwisType() const;
    QString toWKT(quint32 spaces=0);

    QVariant parameter(Projection::ProjectionParamValue type) const;
    void setParameter(Projection::ProjectionParamValue type, const QVariant& value);
    QString parameterName(Projection::ProjectionParamValue pv) const;
    static QString projectionCode2Name(const QString& code);

    bool isEqual(const IlwisObject* obj) const;
    bool isValid() const;
    bool canConvertToLatLon() const;
    bool canConvertToCoordinate() const;

    void setCoordinateSystem(ConventionalCoordinateSystem *csy);
    bool isSet(Projection::ProjectionParamValue type) const;
protected:
    static Projection::FindMatch find(const QString& v1, const QString& value) ;

private:
    QString _wkt;
    QString _authority;
    QScopedPointer<ProjectionImplementation> _implementation;
    static std::map<QString, QString> _projcode2Name;
};
typedef Ilwis::IlwisData<Ilwis::Projection> IProjection;
}



#endif // PROJECTION_H
