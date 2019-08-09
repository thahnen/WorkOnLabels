#include <iostream>
#include <vector>
#include <ctime>

#include <boost/filesystem.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <nlohmann/json.hpp>

#include "structures.h"
#include "filehandler.h"
#include "PathNode.h"

using namespace std;
using namespace cv;
namespace fs = boost::filesystem;
using json = nlohmann::json;


int main() {
    Mat zero = Mat::zeros(Size(720, 640), CV_8UC1);
    Mat json_geometrie = zero.clone();

    // Liste aller Frames (hier aus JSON, spaeter aus Echtzeit-Daten)
    vector<FrameData> frames;


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

    clock_t begin = clock();


    //frames = readJSON("../media/DVS_HGH 01.min.json");    // hier klappt es noch nicht: Datensatz aber auch nicht wie 36!
    frames = readJSON("../media/DVS_HGH 36.min.json");


    cout << "Verbrauchte Zeit zum Einlesen der Daten: " << (double(clock() - begin) / CLOCKS_PER_SEC) << endl;

    /*******************************************************************************************************************
     *
     *      2. Alle Konturen + Mittelpunkte der Objekte mit OpenCV erkennen!
     *      ===============================================================
     *
     *      => Speichert fuer alle Frames die Objekte (Kontur-Punkte + Mittelpunkt) ab
     *      => nimmt die Geometrie-Daten von Labelbox, bei richtigen Werten muss das anders gemacht werden!
     *
     *******************************************************************************************************************/

    begin = clock();


    unsigned int index = 0;

    // Hier MUSS eine Forward-Reference hin, sonst kann in "frame" nicht geschrieben werden, da nur Kopie!
    for (FrameData& frame : frames) {
        Mat binary = zero.clone();


        // TODO: Keine Konturen erkennen muessen, an Vertices der Polygone Momente und Mittelpunkte bilden!


        for (Polygon poly : frame.found_polygons) {
            fillConvexPoly(binary, &poly.vertices[0], poly.vertices.size(), Scalar(255));
        }

        // Dient nur dazu, Konturen zu finden, um daraus die Mittelpunkte zu errechnen
        Mat canny_output;
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;

        Canny(binary, canny_output, 50, 3*50, 7);
        findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

        vector<Moments> mu(contours.size());
        for (int i = 0; i < contours.size(); i++) {
            mu[i] = moments(contours[i], false);
        }

        vector<Point2f> mc(contours.size());
        for (int i = 0; i < contours.size(); i++) {
            mc[i] = Point2f(mu[i].m10/mu[i].m00, mu[i].m01/mu[i].m00);
        }

        // Hier alle Mittelpunkte durchgehen und wenn zwei gleich/ aehnlich, ersten loeschen!
        vector<Point> mittelpunkte;

        for (Point2f& point : mc) {
            bool in_mittelpunkte = false;

            // Nur die Punkte nehmen, die innerhalb des Fensters liegen
            if (!(point.x >= 0 && point.x <= 720 && point.y >= 0 && point.y <= 640)) continue;

            // Jeden Punkt mit jedem bereits vorhandenen Mittelpunkt vergleichen!
            for (int i = 0; i < mittelpunkte.size(); i++) {
                if (point.x < mittelpunkte[i].x+3 && point.x > mittelpunkte[i].x-3
                    && point.y < mittelpunkte[i].y+3 && point.y > mittelpunkte[i].y-3) {
                    in_mittelpunkte = true;
                } else {
                    for (vector<Point> kontur : contours) {
                        double vorhanden_in_poly = pointPolygonTest(kontur, mittelpunkte[i], false);
                        double neu_in_poly = pointPolygonTest(kontur, point, false);
                        if ((vorhanden_in_poly == 1 && neu_in_poly == 1) || (vorhanden_in_poly == 0 && neu_in_poly == 0)) {
                            // Es ist bereits einer vorhanden!
                            // TODO: Überprüfen, welcher besser ist, oder ob einer der beiden nur Bullshit ist!
                            // TODO: ggf für alle Kanten überprüfen, welcher am ehesten von allen gleichweit entfernt war!
                            in_mittelpunkte = true;
                        }
                    }
                }
            }

            if (!in_mittelpunkte) {
                mittelpunkte.push_back(Point((int)point.x, (int)point.y));
            }

        }

        if (mittelpunkte.size() != frame.found_polygons.size()) {
            cerr << "Nicht gleiche Anzahl an Konturen gefunden!" << endl;
            cerr << "Konturen (ueber Mittelpunkte): " << mittelpunkte.size() << ", Polygone: " << frame.found_polygons.size() << endl;

            Mat konturen = zero.clone();
            Mat polygone = zero.clone();

            for (vector<Point> kontur: contours) {
                fillConvexPoly(konturen, &kontur[0], kontur.size(), Scalar(255));
            }

            for (Polygon poly : frame.found_polygons) {
                fillConvexPoly(polygone, &poly.vertices[0], poly.vertices.size(), Scalar(255));
            }

            imshow("Konturen", konturen);
            imshow("Polygone", polygone);
            waitKey(0);

            return 1;
        }

        // Es gibt genauso viele Mittelpunkte wie Polygone
        //cout << "Frame: " << index << endl;
        for (int i = 0; i < frame.found_polygons.size(); i++) {
            frame.found_polygons[i].center = Point(mittelpunkte[i].x, mittelpunkte[i].y);
            //cout << "Polygon " << i << ": Mittelpunkt: " << frame.found_polygons[i].center << endl;
        }

        index++;
    }

    // Hier keine Forward Reference, da die Daten nicht bearbeitet werden sollen!
    for (FrameData frame : frames) {
        Mat data = zero.clone();
        Mat points = zero.clone();

        for (Polygon poly : frame.found_polygons) {
            fillConvexPoly(data, &poly.vertices[0], poly.vertices.size(), Scalar(255));
            circle(points, poly.center, 4, Scalar(255), -1);
        }

        imshow("Polygone mit Mittelpunkt", data);
        imshow("Nur Mittelpunkte", points);
        waitKey(0);
    }


    cout << "Verbrauchte Zeit Zuordnng Frames -> Polygone -> Mittelpunkt: " << (double(clock() - begin) / CLOCKS_PER_SEC) << endl;

    /*******************************************************************************************************************
     *
     *      3. Objekte ueber Frames hinweg verbinden!
     *      ========================================
     *
     *      1) In jedem Frame (bis auf erstem):
     *      => für jedes bestehende Node:
     *          => war das Polygon im Node aus vorherigem Frame "Vorgaenger"?
     *              => NEIN: als neuer Pfad hinzufuegen
     *              => JA: war es nur einer?
     *                  => JA: an alten anheften!
     *                  => NEIN: alle zusammenfassen!
     *
     *      2) Fuer ersten Frame:
     *      => einfach alle als Node hinzufuegen!
     *
     *******************************************************************************************************************/

    begin = clock();


    vector<PathNode> different_paths;
    index = 0;

    // Hier keine Forward Reference, da die Daten nicht bearbeitet werden sollen!
    for (FrameData frame : frames) {
        if (index != 0) {
            for (Polygon poly : frame.found_polygons) {
                vector<int> mgl_vorgaenger;

                for (int i = 0; i < different_paths.size(); i++) {
                    // Zeiger, da hier das Element (noch) nicht veraendert wird
                    PathNode* vorhanden = &different_paths[i];

                    // TODO: entweder wird das in Zukunft noch geaendert oder danach beim zusammenfuegen der Pfade!
                    // Wenn der bestehende Pfad nicht auch im letzten Frame war, dann wird er nicht beachtet!
                    if (vorhanden->timestamp < index-1) continue;


                    int ABSTAND = 10;
                    // 1) Liegt der Mittelpunkt des vorherigen Polygon um neuen Mittelpunkt herum
                    if (vorhanden->objekt.center.x >= poly.center.x-ABSTAND && vorhanden->objekt.center.x <= poly.center.x+ABSTAND
                        && vorhanden->objekt.center.y >= poly.center.y-ABSTAND && vorhanden->objekt.center.y <= poly.center.y+ABSTAND) {
                        mgl_vorgaenger.push_back(i);
                    }

                    // 2) irgendwas anderes
                    else {
                        //
                    }
                }

                PathNode new_node(BEHIND_0, index, poly);

                if (mgl_vorgaenger.size() > 0) {
                    if (mgl_vorgaenger.size() > 1) cout << "Frame: " << index << " hat Polygon mit mehr als einem Vorgaenger!" << endl;
                    for (int path_index : mgl_vorgaenger) {
                        NodeType old_type = different_paths[path_index].typ;
                        if (old_type == BEHIND_0) different_paths[path_index].typ = BEHIND_1;
                        else if (old_type == BEHIND_1) different_paths[path_index].typ = BEHIND_N;

                        // Hier soll es kopiert werden!
                        PathNode old = different_paths[path_index];
                        new_node.vorgaenger.push_back(different_paths[path_index]);
                    }
                }

                different_paths.push_back(new_node);
            }
        } else {
            for (Polygon poly : frame.found_polygons) {
                different_paths.push_back(PathNode(BEHIND_0, index, poly));
            }
        }

        // Jetzt alle Pfade nochmal durchgehen und die, die Teile von anderen sind, loeschen!
        different_paths.erase(remove_if(
                different_paths.begin(),
                different_paths.end(),
                [&](PathNode elem) {
                    int timestamp = elem.timestamp;

                    // Testen, ob alle Element danach einen höheren Timestamp haben
                    for (unsigned int i = find(different_paths.begin(), different_paths.end(), elem)-different_paths.begin()+1; i < different_paths.size(); i++) {
                        // Element aus folgendem Frame? Weil nur dann geht das ^^
                        if ((different_paths[i].timestamp == timestamp+1) && (different_paths[i].vorgaenger.size() > 0)) {
                            // Element koennte das gerade zu testende Objekt beinhalten!
                            for (PathNode vorgaenger : different_paths[i].vorgaenger) {
                                if (elem == vorgaenger) return true;
                            }
                        }
                    }

                    return false;
                }
                ),different_paths.end());

        index++;
    }


    cout << "Verbrauchte Zeit Erstellung grundlegende Pfade: " << (double(clock() - begin) / CLOCKS_PER_SEC) << endl;
}
