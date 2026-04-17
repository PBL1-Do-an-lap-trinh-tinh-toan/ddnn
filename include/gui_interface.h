#ifndef GUI_INTERFACE_H
#define GUI_INTERFACE_H

#include <raylib/raylib.h>
#include <graph.h>
#include <physics.h>

typedef enum {
    MODE_NORMAL,
    MODE_VERTEX_INSPECT,
    MODE_EDGE_INSPECT,
    MODE_CREATE_EDGE,
} GUIMode;

typedef struct {
    bool aboutPage;

    Camera2D camera;

    GUIMode current_mode;

    Graph *graph;
    Vertex *selectedVertex;
    Edge *selectedEdge;
    Vertex *draggingVertex;

    Vertex *pathStartVertex;
    Vertex *pathEndVertex;

    Vertex *edgeStartVertex;

    bool physicsEnabled;

    float springLength;
    float springStiffness;
    float coulombConstant;

    weight_unit_t shortestPathResult;

    char statusBar[128];

    Font font;

} GUIState;

void GUIInit(GUIState *state, const char *appName, const char *fontFile);

bool GUILoadGraph(GUIState *state, Graph *graph);

void GUIUnloadGraph(GUIState *state);

void GUIUpdate(GUIState *state);

void GUIDraw(GUIState *state);

#endif
