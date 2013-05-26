#include "kernel.h"
#include "astnode.h"
#include "operationnode.h"
#include "addnode.h"
#include "symboltable.h"
#include "commandhandler.h"

AddNode::AddNode()
{
}

QString AddNode::nodeType() const
{
    return "add";
}

bool AddNode::evaluate(SymbolTable &symbols, int scope)
{
    if(!OperationNode::evaluate(symbols, scope))
        return false;

    bool ret = true;
    foreach(RightTerm term, _rightTerm) {
        term._rightTerm->evaluate(symbols, scope) ;
        const NodeValue& vright = term._rightTerm->value();
        QString pp = vright.toString();
        if ( term._operator == OperationNode::oADD ){
            ret = handleAdd(vright);
        } else   if ( term._operator == OperationNode::oSUBSTRACT ){
            ret = handleSubstract(vright);
        }
        if (!ret)
            return false;
    }

    return ret;
}

bool AddNode::handleAdd(const NodeValue& vright) {
    QString expr;
    //bool ok1, ok2;
    if ( SymbolTable::isNumerical(vright) && SymbolTable::isNumerical(_value)) {
       _value = {vright.toDouble() + _value.toDouble(), NodeValue::ctNumerical};
       return true;
    }
    return handleBinaryCoverageCases(vright, "binarymathraster", "add");
}

bool AddNode::handleSubstract(const NodeValue& vright) {
    QString expr;
    if ( SymbolTable::isNumerical(vright) && SymbolTable::isNumerical(_value)) {
       _value = {_value.toDouble() -  vright.toDouble(), NodeValue::ctNumerical};
       return true;
    }
    return handleBinaryCoverageCases(vright, "binarymathraster", "substract");
}

