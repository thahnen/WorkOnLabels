//
// Created by Tobias Hahnen on 2019-03-10.
//

#ifndef WORKONLABELS_PATHNODE_H
#define WORKONLABELS_PATHNODE_H


#include <vector>
#include <structures.h>


/***********************************************************************************************************************
 *
 *      Damit lassen sich Pfade ueber Frames hinweg zusammenfassen (als erster Schritt!)
 *
 *      1) Wenn jeder PathNode fuer
 *      - jeden Vorgaenger nur einen Vorgagener
 *      - jeden Nachfolger nur einen Nachfolger
 *      hat, dann bekommt es ein eindeutiges Label fuer ganze Zeit!
 *
 *      2) Wenn nicht, dann muss evaluiert werden anhand NodeType!
 *
 ***********************************************************************************************************************/
class PathNode {
public:
    PathNode(NodeType typ, int timestamp, Polygon objekt);
    ~PathNode() = default;

    bool operator==(PathNode& node);

    std::vector<PathNode> vorgaenger;
    NodeType typ;
    int timestamp;
    Polygon objekt;
private:
};


#endif //WORKONLABELS_PATHNODE_H
