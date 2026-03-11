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

    Body *bodies;
    bool physicsEnabled;

    bool physsimButton;
    char statusBar[128];

} GUIState;

void GUIInit(GUIState *state, const char *appName);

bool GUILoadGraph(GUIState *state, Graph *graph);

void GUIUnloadGraph(GUIState *state);

void GUIUpdate(GUIState *state);

void GUIDraw(GUIState *state);

#endif
