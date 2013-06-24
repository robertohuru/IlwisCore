#include <iostream>
#include "kernel.h"
#include "ilwisdata.h"
#include "symboltable.h"
#include "OperationExpression.h"
#include "operationmetadata.h"
#include "operation.h"
#include "commandhandler.h"
#include "text2output.h"

using namespace Ilwis;
using namespace BaseOperations;

Text2Output::Text2Output()
{
}

Text2Output::Text2Output(quint64 metaid, const Ilwis::OperationExpression &expr) :
    OperationImplementation(metaid, expr)
{
}

bool Text2Output::execute(ExecutionContext *ctx, SymbolTable &symTable)
{
    if (_prepState == sNOTPREPARED)
        if((_prepState = prepare(ctx, symTable)) != sPREPARED)
            return false;
    std::cout << _text.toStdString() << "\n"    ;

    return true;
}

Ilwis::OperationImplementation *Text2Output::create(quint64 metaid, const Ilwis::OperationExpression &expr)
{
    return new Text2Output(metaid, expr);
}

Ilwis::OperationImplementation::State Text2Output::prepare(ExecutionContext *ctx, const Ilwis::SymbolTable &)
{
    _text = _expression.parm(0).value();
    if ( _expression.parameterCount() == 2)     {
        //TODO, file based cases
    }
    return sPREPARED;

}

quint64 Text2Output::createMetadata()
{
    QString url = QString("ilwis://operations/text2output");
    Resource res(QUrl(url), itOPERATIONMETADATA);
    res.addProperty("namespace","ilwis");
    res.addProperty("longname","text2output");
    res.addProperty("syntax","text2output(text[,target-file])");
    res.addProperty("inparameters","1|2");
    res.addProperty("pin_1_type", itSTRING);
    res.addProperty("pin_1_name", TR("input string"));
    res.addProperty("pin_1_desc",TR("input string"));
    res.addProperty("pin_2_type", itSTRING);
    res.addProperty("pin_2_name", TR("filename or path"));
    res.addProperty("pin_2_desc",TR("optional file were strings will be written; if no path is provided, current working folder will be used"));
    res.addProperty("outparameters",0);
    res.prepare();
    url += "=" + QString::number(res.id());
    res.setUrl(url);

    mastercatalog()->addItems({res});
    return res.id();
}