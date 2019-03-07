#include <iostream>
#include <vector>

#include <boost/filesystem.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <nlohmann/json.hpp>

#include "structures.h"

using namespace std;
using namespace cv;
namespace fs = boost::filesystem;
using json = nlohmann::json;


typedef struct FoundObject {
    vector<Point> contour;
    Point2f middle;
} FoundObject;


int main() {
    Mat zero = Mat::zeros(Size(720, 640), CV_8UC1);
    Mat json_geometrie = zero.clone();


    /*******************************************************************************************************************
     *
     *      1. Alle Objekte aus JSON einlesen!
     *      =================================
     *
     *      => nur bei Labelbox noetig!
     *      => bisher werden nur die Geometrien der Objekte abgespeichert!
     *      => ggf weitere Informationen noetig!
     *
     *******************************************************************************************************************/

    // Der Pfad, weil die Anwendung im "cmake-..."-Ordner liegt!
    fs::ifstream file{fs::path("../media/DVS_HGH 36.min.json")};

    json json_data;
    file >> json_data;
    file.close();

    // Liste aller Objekte, die im JSON sind (aus Labelbox)
    vector<vector<Polygon>> objects_over_frames;                        // nur noch hier bis alles fuer neues umbenannt!

    // Liste aller Frames (hier aus JSON, spaeter aus Echtzeit-Daten)
    vector<FrameData> frames;

    for (auto& frame : json_data) {
        vector<Polygon> objects_in_frame;                               // nur noch hier bis alles fuer neues umbenannt!
        FrameData data_for_this_frame;

        for (auto& label : frame["Label"]["object"]) {
            Polygon poly;

            for (auto& point : label["geometry"]) {
                int x = point["x"];
                int y = point["y"];

                // Werte sollten vorher schon in (x: 0-719, y: 0-639) liegen
                // Anpassung nur fuer Konturen und Mittelpunkte noetig!
                if (x > 718) x = 718;
                else if (x <= 0) x = 1;

                if (y > 638) y = 638;
                else if (y <= 0) y = 1;

                poly.vertices.push_back(Point(x, y));
            }
            objects_in_frame.push_back(poly);
            data_for_this_frame.found_polygons.push_back(poly);

        }
        objects_over_frames.push_back(objects_in_frame);
        frames.push_back(data_for_this_frame);
    }


    /*******************************************************************************************************************
     *
     *      2. Alle Konturen + Mittelpunkte der Objekte mit OpenCV erkennen!
     *      ===============================================================
     *
     *      => Speichert fuer alle Frames die Objekte (Kontur-Punkte + Mittelpunkt) ab
     *      => nimmt die Geometrie-Daten von Labelbox, bei richtigen Werten muss das anders gemacht werden!
     *
     *******************************************************************************************************************/

    for (FrameData frame : frames) {
        Mat binary = zero.clone();

        for (Polygon poly : frame.found_polygons) {
            // &poly.vertices[0] -> veraendert Vector in Array!
            fillConvexPoly(binary, &poly.vertices[0], poly.vertices.size(), Scalar(255));

            Mat canny_output;
            vector<vector<Point>> contours;
            vector<Vec4i> hierarchy;

            Canny(binary, canny_output, 50, 3*50, 5);
            findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

            vector<Moments> mu(contours.size());
            for (int i = 0; i < contours.size(); i++) {
                mu[i] = moments(contours[i], false);
            }

            vector<Point2f> mc(contours.size());
            for (int i = 0; i < contours.size(); i++) {
                mc[i] = Point2f(mu[i].m10/mu[i].m00, mu[i].m01/mu[i].m00);
                poly.center = mc[i];                                        // sollte immer auf letztes Element gesetzt werden!
            }
        }
    }

    // Liste aller Frames mit jeweils aller Objekte und deren Eigenschaften
    vector<vector<FoundObject>> gefundene_objekte;

    // Jetzt hat man alle Objekte ueber alle Frames extrahiert!
    for (vector<Polygon> frame : objects_over_frames) {
        Mat xyz = zero.clone();
        Mat konturen_mittelpunkt;

        vector<FoundObject> objekte_in_diesem_frame;

        for (Polygon poly : frame) {
            fillConvexPoly(xyz, &poly.vertices[0], poly.vertices.size(), Scalar(255));

            Mat canny_output;
            vector<vector<Point>> contours;
            vector<Vec4i> hierarchy;

            Canny(xyz, canny_output, 50, 150, 5);
            findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

            vector<Moments> mu(contours.size());
            for (int i = 0; i < contours.size(); i++) {
                mu[i] = moments(contours[i], false);
            }

            vector<Point2f> mc(contours.size());
            vector<int> fallen_lassen;
            for (int i = 0; i < contours.size(); i++) {
                mc[i] = Point2f(mu[i].m10/mu[i].m00, mu[i].m01/mu[i].m00);
                poly.center = mc[i];

                // Hier ueberpruefen, ob der Mittelpunkt am Rand liegt, dann wird das fallen gelassen!
                if (mc[i].x < 10 || mc[i].x > 710 || mc[i].y < 0 || mc[i].y > 630) {
                    fallen_lassen.push_back(i);
                }
            }

            // Fuer spaetere Weiterarbeit die Objekte abspeichern!          // sind hier nicht viele Kopien von demselben drin?
            for (int i = 0; i < contours.size(); i++) {
                objekte_in_diesem_frame.push_back({
                    contours[i], mc[i]
                });
            }

            // Hier wird nur eingezeichnet, alles andere ist bisher vorhanden!
            konturen_mittelpunkt = Mat(canny_output.size(), CV_8UC1, Scalar(255));
            for (int i = 0; i < contours.size(); i++) {
                if (find(fallen_lassen.begin(), fallen_lassen.end(), i) != fallen_lassen.end()) {
                    continue;
                }

                Scalar color(0);
                drawContours(konturen_mittelpunkt, contours, i, color, 2, 8, hierarchy, 0, Point());
                circle(konturen_mittelpunkt, mc[i], 4, color, -1, 8, 0);
            }
        }

        // Alle Objekte dieses Frames in die Liste mit allen Frames hinzufuegen!
        gefundene_objekte.push_back(objekte_in_diesem_frame);

        imshow("Contours", konturen_mittelpunkt);
        imshow("Eingezeichnete Polygone", xyz);
        waitKey(0);
    }


    /*******************************************************************************************************************
     *
     *      3. Objekte ueber Frames hinweg verbinden!
     *      ========================================
     *
     *******************************************************************************************************************/

    // Hier werden die Indizes der Objekte abgespeichert, die ueber Frame hinweg gleich sind!
    vector<vector<int>> same_objects;

    for (vector<FoundObject> frame : gefundene_objekte) {
        // Fuer jedes Frame gibt es eine Liste von Objekten!


    }
}
