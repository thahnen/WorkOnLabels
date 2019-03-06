#include <iostream>
#include <vector>

#include <boost/filesystem.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using namespace cv;
namespace fs = boost::filesystem;
using json = nlohmann::json;


typedef struct Polygon {
    vector<Point> vertices;
    Point middle;
} Polygon;


typedef struct FoundObject {
    vector<Point> contour;
    Point2f middle;
} FoundObject;

typedef struct ObjectGraph {
    int index;
    struct ObjectGraph* next;
} ObjectGraph;


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
     *
     *******************************************************************************************************************/

    // Der Pfad, weil die Anwendung im "cmake-..."-Ordner liegt!
    fs::ifstream file{fs::path("../media/DVS_HGH 36.min.json")};

    json json_data;
    file >> json_data;
    file.close();

    // Liste aller Objekte, die im JSON sind (aus Labelbox)
    vector<vector<Polygon>> objects_over_frames;

    for (auto& frame : json_data) {
        vector<Polygon> objects_in_frame;

        for (auto& label : frame["Label"]["object"]) {
            auto& geometry = label["geometry"];
            Polygon poly;

            for (int i = 0; i < geometry.size(); i++) {
                auto point = geometry[i];

                // Fuer Konturen/ Mittelpunkte darf kein Punkt am Rand liegen!
                if (point["x"] >= 719) {
                    point["x"] = 718;
                } else if (point["x"] <= 0) {
                    point["x"] = 1;
                }

                if (point["y"] >= 639) {
                    point["y"] = 638;
                } else if (point["x"] <= 0) {
                    point["x"] = 1;
                }

                poly.vertices.push_back(Point(point["x"], point["y"]));
            }
            objects_in_frame.push_back(poly);
        }
        objects_over_frames.push_back(objects_in_frame);
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
                poly.middle = mc[i];

                // Hier ueberpruefen, ob der Mittelpunkt am Rand liegt, dann wird das fallen gelassen!
                if (mc[i].x < 10 || mc[i].x > 710 || mc[i].y < 0 || mc[i].y > 630) {
                    fallen_lassen.push_back(i);
                }
            }

            // Fuer spaetere Weiterarbeit die Objekte abspeichern!
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

    }
}
