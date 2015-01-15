#ifndef COVERAGELAYERMODEL_H
#define COVERAGELAYERMODEL_H

#include <QQmlListProperty>
#include "resourcemodel.h"
#include "propertyeditors/propertyeditor.h"
#include "ilwiscoreui_global.h"

class VisualizationManager;
class PropertyEditor;

class ILWISCOREUISHARED_EXPORT CoverageLayerModel : public ResourceModel
{
    Q_OBJECT

    Q_PROPERTY(QQmlListProperty<PropertyEditor> propertyEditors READ propertyEditors NOTIFY propertyEditorChanged)


public:
    CoverageLayerModel();
    CoverageLayerModel(const Ilwis::Resource &resource, const QList<PropertyEditor *> &editors, QObject *obj=0);

    Q_INVOKABLE PropertyEditor* propertyEditor(const QString& name);



private:
    QQmlListProperty<PropertyEditor> propertyEditors();
    QList<PropertyEditor *> _propertyEditors;

signals:
    void propertyEditorChanged();

};

typedef QQmlListProperty<CoverageLayerModel> CoverageLayerModelList;
Q_DECLARE_METATYPE(CoverageLayerModelList)

#endif // COVERAGELAYERMODEL_H
