//
// Created by Tobias Hahnen on 2019-03-06.
//

#ifndef WORKONLABELS_STRUCTURES_H
#define WORKONLABELS_STRUCTURES_H

#include <vector>
#include <opencv4/opencv2/opencv.hpp>


/***********************************************************************************************************************
 *
 *      Beschreibt vom Algorithmus gefundendes Polygon/ in Labelbox nachgebessertes Polygon:
 *
 *      1) Liste aller Punkte, die Grenzen des Polygon beschreiben
 *      2) Mittelpunkt des Polygons, zur Pfadbestimmung
 *      [ 3) Weiter Informationen, die zur Beschreibung notwendig sind? ]
 *
 ***********************************************************************************************************************/
typedef struct Polygon {
    std::vector<cv::Point> vertices;
    cv::Point center;
    // ...
} Polygon;


/***********************************************************************************************************************
 *
 *      Alle Daten, die einen Frame beschreiben!
 *
 *      1) Liste aller in dem Frame gefundenden Objekte (Polygone)
 *      [ 2) Weitere Informationen aus JSON? Echtzeit-Infos? ]
 *
 ***********************************************************************************************************************/
typedef struct FrameData {
    std::vector<Polygon> found_polygons;
    // ...
} FrameData;


/***********************************************************************************************************************
 *
 *      Damit laesst sich der Typ des momentanen Nodes beschreiben (fuer weitere Schritte!)
 *
 *      1) Man kann damit die Anzahl der Objekte ueber ein Pfad-Netz approximieren!
 *
 ***********************************************************************************************************************/
typedef enum NodeType : int {
    BEFORE_0_BEHIND_0 = 0,      // 0 vorher da, 0 danach da     -> ???
    BEFORE_0_BEHIND_1 = 1,      // 0 vorher da, 1 danach da     -> Objekt hinzugekommen
    BEFORE_0_BEHIND_N = 2,      // 0 vorher da, >1 danach da    -> Objekte hinzugekommen und aufgesplittet in mehrere
    BEFORE_1_BEHIND_0 = 3,      // 1 vorher da, 0 danach da     -> Objekt verschwunden
    BEFORE_1_BEHIND_1 = 4,      // 1 vorher da, 1 danach da     -> Objekt weiterhin vorhanden
    BEFORE_1_BEHIND_N = 5,      // 1 vorher da, >1 danach da    -> Objekt aufgesplittet in mehrere
    BEFORE_N_BEHIND_0 = 6,      // >1 vorher da, 0 danach da    -> Mehrere Objekte zusammengekommen und verschwunden
    BEFORE_N_BEHIND_1 = 7,      // >1 vorher da, 1 danach da    -> Mehrere Objekte zusammengekommen
    BEFORE_N_BEHIND_N = 8,      // >1 vorher da, >1 danach da   -> Mehrere Objekte zusammengekommen und aufgesplittet in mehrere
} NodeType;


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
typedef struct PathNode {
    std::vector<struct PathNode*> vorgaenger;   // Alle Vorgaenger-Nodes 0-n moeglich!
    NodeType typ;                               // Dient zur Label-Vergabe
    Polygon objekt;                             // In aktuellem Frame erkanntes Objekt mit Eigenschaften!
    std::vector<struct PathNode*> nachfolger;   // Alle Nachfolger-Nodes 0-n moeglich!
} PathNode;


#endif //WORKONLABELS_STRUCTURES_H
