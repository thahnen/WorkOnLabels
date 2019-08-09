//
// Created by Tobias Hahnen on 2019-03-10.
//

#include "PathNode.h"


PathNode::PathNode(NodeType typ, int timestamp, Polygon objekt) {
    this->typ = typ;
    this->timestamp = timestamp;
    this->objekt = objekt;
}

bool PathNode::operator==(const PathNode &node) {
    // GGF muss hier geschaut werden, wenn die Vektoren nicht umgestellt sind oder so
    return ((this->typ == node.typ) && (this->timestamp == node.timestamp)
        && (this->objekt == node.objekt) && (this->vorgaenger.size() == node.vorgaenger.size()));
}