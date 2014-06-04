#include <QRegExp>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QSettings>
#include <QSqlField>
#include "identity.h"
#include "kernel.h"
#include "oshelper.h"
#include "connectorinterface.h"
#include "mastercatalog.h"
#include "ilwisobjectconnector.h"
#include "catalogexplorer.h"
#include "catalogconnector.h"
#include "abstractfactory.h"
#include "connectorfactory.h"
#include "ilwiscontext.h"
#include "catalog.h"
#include "symboltable.h"
#include "operationExpression.h"



Ilwis::MasterCatalog *Ilwis::MasterCatalog::_masterCatalog = 0;

using namespace Ilwis;

MasterCatalog* Ilwis::mastercatalog() {
    if (Ilwis::MasterCatalog::_masterCatalog == 0) {
        Ilwis::MasterCatalog::_masterCatalog = new Ilwis::MasterCatalog();
        Ilwis::MasterCatalog::_masterCatalog->prepare();

    }
    return Ilwis::MasterCatalog::_masterCatalog;
}

MasterCatalog::MasterCatalog()
{
}

MasterCatalog::~MasterCatalog()
{
    _lookup.clear();
    _knownHashes.clear();
    _catalogs.clear();
}

bool MasterCatalog::prepare()
{
    QSettings settings("52n","ilwis4");

    //TODO: this piece code must move to a more appropriate place; at this moment it
    // is not guaranteed that the catalogconnectors are already known due to the order of the plugins
    // as they are loaded.
//    QString local = settings.value("localfilesystem",QVariant(sUNDEF)).toString();

    return true;
}


bool MasterCatalog::addContainer(const QUrl &inlocation)
{
    if ( !inlocation.isValid()) // it is valid to try this with an empty url; just wont do anything
        return true;

    QString original = inlocation.toString().trimmed();;
    QString loc = original;
    if ( loc[loc.size() - 1] == '/' ){
        loc = loc.left(loc.size() - 1);
        int count = loc.count("/");
        if ( loc.indexOf(":///") != -1){
            if ( count == 3)
                loc = original;
        }else if ( loc.indexOf("://") != -1){
            if ( count == 2){
                loc= original;
            }
        }
    }

    if ( loc.indexOf("ilwis://tables") == 0||
         loc.indexOf("ilwis://factory") == 0 ||
         loc.indexOf("ilwis://system") == 0 ||
         loc.indexOf("ilwis://operations") == 0 ||
         loc == "file://" ||
         loc == "ilwis:/" ||
         loc.isEmpty())
        return true;
    QUrl location(loc);
    if ( _catalogs.find(location) != _catalogs.end())
        return true;

    ICatalog catalog(loc);
    if ( !catalog.isValid()){
        return false;
    }

    addItems({catalog->source()});
    _catalogs.insert(location);
    return true;
}


ESPIlwisObject MasterCatalog::get(const QUrl &resource, IlwisTypes type) const
{
    quint64 id = url2id(resource, type);
    return get(id);
}

ESPIlwisObject MasterCatalog::get(quint64 id) const
{
    if ( id != i64UNDEF) {
        auto iter = _lookup.find(id);
        if ( iter != _lookup.end())
            return iter.value();
    }
    return ESPIlwisObject();
}

bool MasterCatalog::contains(const QUrl& url, IlwisTypes type) const{
    auto hash = Ilwis::qHash2(url, type);
    auto  iter = _knownHashes.find(hash);
    if ( iter != _knownHashes.end()) {
        auto id = url2id(url, type);
        if ( id != i64UNDEF)
            return true;
    }
    return false;
}

bool MasterCatalog::usesContainers(const QUrl &url) const
{
    return _containerExceptions.find(url.scheme()) == _containerExceptions.end();
}

void MasterCatalog::addContainerException(const QString &scheme)
{
    _containerExceptions.insert(scheme);
}

bool MasterCatalog::removeItems(const QList<Resource> &items){
    for(const Resource &resource : items) {
        auto iter = _knownHashes.find(Ilwis::qHash(resource));
        if ( iter != _knownHashes.end()) {
            _knownHashes.erase(iter);
        }
        QString stmt = QString("DELETE FROM mastercatalog WHERE resource = %1" ).arg(resource.id());
        QSqlQuery db(kernel()->database());
        if(!db.exec(stmt)) {
            kernel()->issues()->logSql(db.lastError());
            return false;
        }
        stmt = QString("DELETE FROM catalogitemproperties WHERE resource = %1").arg(resource.id());
        if(!db.exec(stmt)) {
            kernel()->issues()->logSql(db.lastError());
            return false;
        }
    }

    return true;
}

bool MasterCatalog::addItems(const std::vector<Resource>& items)
{
    if( items.size() == 0) // nothing to do; not wrong perse
            return true;

    QSqlQuery queryItem(kernel()->database()), queryProperties(kernel()->database());

    bool ok = queryItem.prepare("INSERT INTO mastercatalog VALUES(\
                  :itemid,:name,:code,:container,:resource,:rawresource,:urlquery,:type,:extendedtype, :size,:dimensions \
                  )" );
    if (!ok) {
        kernel()->issues()->logSql(queryItem.lastError());
        return false;

    }

    ok = queryProperties.prepare("INSERT INTO catalogitemproperties VALUES(\
                   :propertyvalue,:propertyname,:itemid\
                 )" );
    if (!ok) {
        kernel()->issues()->logSql(queryItem.lastError());
        return false;
    }

    for(const Resource &resource : items) {
        if (!resource.isValid())
           continue;
        if ( mastercatalog()->contains(resource.url(), resource.ilwisType()))
          continue;

        _knownHashes.insert(Ilwis::qHash(resource));
        resource.store(queryItem, queryProperties);
    }


    return true;

}

quint64 MasterCatalog::url2id(const QUrl &url, IlwisTypes tp) const
{
    auto query = QString("select itemid,type from mastercatalog where resource = '%1'").arg(url.toString());
    auto results = kernel()->database().exec(query);
    while ( results.next()) {
        auto rec = results.record();
        auto iid = rec.value(0).toLongLong();
        auto itype = rec.value(1).toLongLong();
        if ( (itype & tp) || tp == itUNKNOWN)
            return iid;
    }

    return i64UNDEF;

}

Resource MasterCatalog::id2Resource(quint64 iid) const {
    auto query = QString("select * from mastercatalog where itemid = %1").arg(iid);
    auto results = kernel()->database().exec(query);
    if ( results.next()) {
        auto rec = results.record();
        return Resource(rec);
    }
    return Resource();
}

quint64 MasterCatalog::name2id(const QString &name, IlwisTypes tp) const
{
    quint64 id = IlwisObject::internalname2id(name);
    if ( id == i64UNDEF){
        Resource resource = name2Resource(name,tp);
        id =  resource.id();
    }
    return id;
}

IlwisTypes MasterCatalog::id2type(quint64 iid) const {
    QString query = QString("select type from mastercatalog where itemid = %1").arg(iid);
    QSqlQuery results = kernel()->database().exec(query);
    if ( results.next()) {
        return results.value(0).toLongLong();
    }
    return itUNKNOWN;
}


Resource MasterCatalog::name2Resource(const QString &name, IlwisTypes tp) const
{
    if ( tp == itUNKNOWN) { // no definitive type given we, try to find based on name/type combination. will work mostely but not guaranteed. this is anyway a last resort method
        std::vector<IlwisTypes> types { itRASTER, itFEATURE, itTABLE, itGEOREF, itCOORDSYSTEM, itDOMAIN};
        for(IlwisTypes type: types) {
            Resource resource = name2Resource(name, type);
            if (resource.isValid())
                return resource;
        }

        return Resource();
    }
    auto resolvedName = name2url(name, tp);
    if (!resolvedName.isValid())
        return Resource();

    resolvedName = OSHelper::neutralizeFileName(resolvedName.toString());
    auto query = QString("select * from mastercatalog where resource = '%1' and (type & %2) != 0").arg(resolvedName.toString()).arg(tp);
    auto results = kernel()->database().exec(query);
    if ( results.next()) {
        auto rec = results.record();
        return Resource(rec);

    } else {
        query = QString("select propertyvalue from catalogitemproperties,mastercatalog \
                        where mastercatalog.resource='%1' and mastercatalog.itemid=catalogitemproperties.itemid\
                and (mastercatalog.extendedtype & %2) != 0").arg(resolvedName.toString()).arg(tp);
        auto viaExtType = kernel()->database().exec(query);
        bool isExternalRef = true;
        while ( viaExtType.next()){ // external reference finding
            isExternalRef = false;
            bool ok;
            auto propertyid = viaExtType.value(0).toLongLong(&ok);
            if (!ok) {
                kernel()->issues()->log(TR("Invalid catalog property, mastercatalog corrupted?"),IssueObject::itWarning);
            }
            auto type = id2type(propertyid);
            if ( type & tp)
                return id2Resource(propertyid);
        }
        if ( !isExternalRef) { // it was not an external reference but an internal one; if it was external it will never come here
            // this is a new resource which only existed as reference but now gets real, so add it to the catalog
            Resource resource(QUrl(resolvedName), tp);
            const_cast<MasterCatalog *>(this)->addItems({resource});
            return resource;
        }
    }
    return Resource();
}

QUrl MasterCatalog::name2url(const QString &name, IlwisTypes tp) const{
    if ( name.contains(QRegExp("\\\\|/"))) { // is there already path info; then assume it is already a unique resource
        bool ok = OSHelper::isAbsolute(name); // name might be a partial path
        if ( ok && !OSHelper::isFileName(name))
    // translate name to url; we have the following cases
    // * already is a path, no big deal; just make an url of it
    // * is a code; construct a path leading to the internal connector as all codes are handled there
    // * no, path, no code. So it must be in the current working catalog.
    // * backup case. we try it by name. may fail as names are not necessarily unique. system is robust enough to handle fail cases

            return name;
        if ( ok){
            return QUrl::fromLocalFile(name).toString();
        }else {
            QString resolvedName =  context()->workingCatalog()->source().url().toString() + "/" + name;
            return resolvedName;
        }

    }
    if ( name.indexOf("code=wkt:")==0) { // second case -- handle most codes cases
        auto code = name.right(name.size() - 5);
        auto wkt = code.mid(4);
        auto table = "projection";
        if ( tp & itELLIPSOID )
            table = "ellipsoid";
        else if ( tp & itGEODETICDATUM)
            table = "datum";
        //auto query = QString("select code from %1 where wkt='%2'").arg(table, wkt);
        auto query = QString("select code from %1 where wkt like '%%2%'").arg(table, wkt);
        auto codes = kernel()->database().exec(query);
        if ( codes.next()) {
            QString res = QString("ilwis://tables/%1?code=%2").arg(table,codes.value(0).toString());
            return res;
        }else {
            kernel()->issues()->log(TR(ERR_FIND_SYSTEM_OBJECT_1).arg(wkt));
            return QUrl();
        }
    } else if ( name.indexOf("code=epsg:")== 0 ) {
        auto code = name.right(name.size() - 5);
        return QString("ilwis://tables/projectedcsy?code=%1").arg(code);

    } else if ( name.left(6) == "code=proj4:") {
        auto code = name.right(name.size() - 5);
        return QString("ilwis://projection/code=%1").arg(code);
    } else if ( name.left(12) == "code=domain:") {
        return QString("ilwis://internalcatalog/%1").arg(name);
    }else if ( name.left(12) == "code=georef:") {
        QString shortname = name.mid(name.indexOf(":") + 1);
        return QString("ilwis://internalcatalog/%1").arg(shortname);
    }else if ( name.left(9) == "code=csy:") {
        QString shortname = name.mid(name.indexOf(":") + 1);
        return QString("ilwis://internalcatalog/%1").arg(shortname);
    }
    if ( context()->workingCatalog().isValid()) { // thirde case -- use the working catalog to extend the path
        auto resolvedName = context()->workingCatalog()->resolve(name, tp);
        if ( resolvedName != sUNDEF)
            return resolvedName;
    }
    auto code = name;
    if ( code.indexOf("code=") == 0)
        code = code.mid(5);

    // fourth case -- try name
    auto query = QString("select resource,type from mastercatalog where name = '%1' or code='%1'").arg(code);
    auto results = kernel()->database().exec(query);
    while ( results.next()) {
        auto rec = results.record();
        auto type = rec.value(1).toLongLong();
        if ( type & tp)
            return rec.value(0).toString();
    }
    return QUrl();

}

bool MasterCatalog::isRegistered(quint64 id) const
{
    return _lookup.contains(id);
}

bool MasterCatalog::unregister(quint64 id)
{
    QHash<quint64,ESPIlwisObject>::const_iterator iter = _lookup.find(id);
    if ( iter != _lookup.end()) {
        _lookup.remove(id);
    }

    return true;

}

std::vector<Resource> MasterCatalog::select(const QUrl &resource, const QString &selection) const
{
    QString rest = selection == "" ? "" : QString("and (%1)").arg(selection);
    QString query = QString("select * from mastercatalog where container = '%1' %2").arg(resource.toString(), rest);
    QSqlQuery results = kernel()->database().exec(query);
    std::vector<Resource> items;
    while( results.next()) {
        QSqlRecord rec = results.record();
        items.push_back(Resource(rec));
    }
    return items;

}

void MasterCatalog::registerObject(ESPIlwisObject &data)
{
    if ( data.get() == 0) {
        QHash<quint64,ESPIlwisObject>::iterator iter = _lookup.find(data->id());
        data = iter.value();
    } else {
        if ( !data->isAnonymous())
            addItems({data->source()});
        _lookup[data->id()] = data;

    }
}



#ifdef QT_DEBUG

void MasterCatalog::dumpLookup() const
{
    for(auto pr : _lookup) {
        qDebug() << pr->name();
    }
}

quint32 MasterCatalog::usecount(quint64 id)
{
    auto iter = _lookup.find(id);
    if ( iter != _lookup.end()) {
        return iter.value().use_count();
    }
    return 0;
}

#endif
