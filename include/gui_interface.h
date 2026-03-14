#ifndef GUI_INTERFACE_H
#define GUI_INTERFACE_H

#include <raylib/raylib.h>
#include <graph.h>
#include <physics.h>

typedef struct {
    Camera2D camera;

    Graph *graph;
    Vertex *startVertex;
    Vertex *endVertex;
    Vertex *selectedVertex;
    Vertex *draggingVertex;

    Body *bodies;
    bool physicsEnabled;


    float springLength;
    float springStiffness;
    float coulombConstant;

    bool physsimButton;
    bool jiggleButton;
    bool startVertButton;
    bool endVertButton;
    char statusBar[128];

    Font font;

} GUIState;

void GUIInit(GUIState *state, const char *appName, const char *fontFile);

bool GUILoadGraph(GUIState *state, Graph *graph);

void GUIUnloadGraph(GUIState *state);

void GUIUpdate(GUIState *state);

void GUIDraw(GUIState *state);

#endif
