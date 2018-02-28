#include "NodeVisitor.h"
#include "Ast/AstNode.h"

namespace Strela {
    void NodeVisitor::visitChild(AstNode* child) {
        child->accept(*this);
    }
}