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

inline bool operator==(const Polygon& poly1, const Polygon& poly2) {
    // Test beider Vektoren sollte so gehen (werden ja eigentlich nicht umgestellt beim kopieren!)
    return ((poly1.center == poly2.center) && (poly1.vertices == poly2.vertices));
}


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


#endif //WORKONLABELS_STRUCTURES_H
