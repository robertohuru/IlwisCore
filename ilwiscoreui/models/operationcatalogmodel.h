#ifndef OPERATIONSMODEL_H
#define OPERATIONSMODEL_H

#include "catalogmodel.h"
#include "operationmodel.h"
#include "operationsbykeymodel.h"
#include "ilwiscoreui_global.h"

class OperationModel;
class OperationsByKeyModel;

class ILWISCOREUISHARED_EXPORT OperationCatalogModel : public CatalogModel
{
    Q_OBJECT
    Q_PROPERTY(QMLOperationList operations READ operations NOTIFY operationsChanged)
    Q_PROPERTY(QQmlListProperty<OperationsByKeyModel> operationKeywords READ operationKeywords CONSTANT)
public:


    OperationCatalogModel(QObject * parent=0);

   QMLOperationList operations();
   QQmlListProperty<OperationsByKeyModel> operationKeywords();
   void nameFilter(const QString&);

   Q_INVOKABLE quint64 operationId(quint32 index, bool byKey) const;
   Q_INVOKABLE quint64 serviceId(quint32 index) const;
   Q_INVOKABLE QStringList serviceNames() const;
   Q_INVOKABLE QString executeoperation(quint64 operationid, const QString &parameters);

private:
    QList<OperationModel *> _currentOperations;
    QList<OperationsByKeyModel *> _operationsByKey;
    std::vector<Ilwis::Resource> _services;

signals:
    void updateCatalog(const QUrl& url);
    void operationsChanged();
    void error(const QString& err);

};


#endif // OPERATIONSMODEL_H
