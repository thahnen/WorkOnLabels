//
// Created by thahnen on 07.03.19.
//

#ifndef WORKONLABELS_UTIL_H
#define WORKONLABELS_UTIL_H

#include <vector>
#include "structures.h"


/*
 *
 */
std::vector<PathNode*> getLastPathNodes(PathNode* searchNode, int timestamp_now) {
    std::vector<PathNode*> nodes, nachfolger_nodes;
    nodes.push_back(searchNode);

    for (PathNode* nachfolger : searchNode->nachfolger) {
        if (nachfolger->nachfolger.size() == 0) {
            continue;
        } else {
            nachfolger_nodes = getLastPathNodes(nachfolger, timestamp_now);
            if (nachfolger_nodes.size() != 0) {
                nodes.insert(std::end(nodes), std::begin(nachfolger_nodes), std::end(nachfolger_nodes));
            }
        }
    }

    int highest_timestamp = 0;
    for (int i = 0; i < nodes.size(); i++) {
        if (nodes[i]->timestamp > highest_timestamp) highest_timestamp = nodes[i]->timestamp;
    }

    if (highest_timestamp == 0) return std::vector<PathNode*>();

    for (PathNode* node : nodes) {
        if (node->timestamp != highest_timestamp) {
            std::remove(nodes.begin(), nodes.end(), node);
        }
    }

    return nodes;
}


#endif //WORKONLABELS_UTIL_H
