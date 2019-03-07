//
// Created by thahnen on 07.03.19.
//

#ifndef WORKONLABELS_FILEHANDLER_H
#define WORKONLABELS_FILEHANDLER_H

//#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <nlohmann/json.hpp>

#include "structures.h"


/***********************************************************************************************************************
 *
 *      FUNKTION: readJSON
 *      ==================
 *
 *      1) Returns: std::vector<FrameData>  :   Alle eingelesenen Frame-Daten aus einem Labelbox-Export
 *
 *      2) Parameters:
 *          - std::string path_to_json      :   Dateipfad zur JSON-Datei
 *
 ***********************************************************************************************************************/
std::vector<FrameData> readJSON(std::string path_to_json) {
    std::vector<FrameData> frames;

    // TODO: Ueberpruefen, ob Pfad auch existiert
    boost::filesystem::ifstream file{boost::filesystem::path(path_to_json)};

    nlohmann::json json_data;

    // TODO: Einlesefehler abfangen!
    file >> json_data;
    file.close();

    // TODO: Ueberpruefen, ob Root-Element Liste ist!
    for (auto frame : json_data) {
        FrameData data_for_this_frame;

        // TODO: Ueberpruefen, ob Objekte jeweils existieren!
        for (auto label : frame["Label"]["object"]) {
            Polygon poly;

            // TODO: Ueberpruefen, ob Objekte jeweils existiert!
            for (auto point : label["geometry"]) {
                int x = point["x"];
                int y = point["y"];

                // Werte sollten vorher schon in (x: 0-719, y: 0-639) liegen
                // Anpassung nur fuer Konturen und Mittelpunkte noetig!
                if (x > 718) x = 718;
                else if (x <= 0) x = 1;

                if (y > 638) y = 638;
                else if (y <= 0) y = 1;

                poly.vertices.push_back(cv::Point(x, y));
            }
            data_for_this_frame.found_polygons.push_back(poly);
        }
        frames.push_back(data_for_this_frame);
    }
    return frames;
}


/***********************************************************************************************************************
 *
 *      FUNKTION: writeOutput
 *      =====================
 *
 *      1) ...
 *
 *      2) ...
 *
 *      3) ... ich komme noch ...
 *
 ***********************************************************************************************************************/


#endif //WORKONLABELS_FILEHANDLER_H
