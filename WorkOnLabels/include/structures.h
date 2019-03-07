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
    BEHIND_0 = 0,       // 0 danach da      -> Objekt hinzugekommen ODER verschwunden
    BEHIND_1 = 1,       // 1 danach da      -> Objekt weitergefuert
    BEHIND_N = 2,       // >1 danach da     -> Objekte aufgesplittet in mehrere
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
    NodeType typ;                                   // Dient zur Label-Vergabe
    int timestamp;                                  // Der Zeitstempel, damit nachher in Funktion ueberfuehrbar
    Polygon objekt;                                 // In aktuellem Frame erkanntes Objekt mit Eigenschaften!
    std::vector<struct PathNode*> nachfolger;       // Alle Nachfolger-Nodes 0-n moeglich!
} PathNode;


#endif //WORKONLABELS_STRUCTURES_H
