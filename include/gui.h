#ifndef GUI_INTERFACE_H
#define GUI_INTERFACE_H

#include <raylib/raylib.h>
#include <graph.h>
#include <physics.h>
#include <gui_window_file_dialog.h>

#define MARGIN 5
#define ITEM_WIDTH (PANEL_WIDTH - (MARGIN * 2))
#define START_Y 39

typedef enum {
    MODE_NORMAL,
    MODE_VERTEX_INSPECT,
    MODE_EDGE_INSPECT,
    MODE_CREATE_EDGE,
} GUIMode;

typedef struct {
    bool aboutPage;
    bool pathPage;

    Camera2D camera;

    GuiWindowFileDialogState fileDialogState;

    GUIMode current_mode;

    Graph *graph;
    Vertex *selectedVertex;
    Edge *selectedEdge;
    Vertex *draggingVertex;

    Vertex *pathStartVertex;
    Vertex *pathEndVertex;

    Vertex *edgeStartVertex;

    bool physicsEnabled;

    float currentTemperature;
    float springLength;
    float springStiffness;
    float coulombConstant;

    weight_unit_t shortestPathResult;

    char statusBar[128];

    Font font;

} GUIState;

// gui_graph_action.c
bool GUILoadGraph(GUIState *state, Graph *graph);
void GUIUnloadGraph(GUIState *state);
void GUIFindShortestPath(GUIState *state);
void GUISetStartVert(GUIState *state);
void GUISetEndVert(GUIState *state);
void GUISwapStartEndVert(GUIState *state);
void GUIAddEdge(GUIState *state, Vertex *startVert, Vertex *endVert);
void GUIDeleteEdge(GUIState *state);
void GUIReverseEdge(GUIState *state);
void GUIAddVertex(GUIState *state);
void GUIDeleteVert(GUIState *state);

// gui_graph_draw.c
void GUIDrawGraph(GUIState *state);

// gui_view_draw.c
void GUIDrawNormalView(GUIState *state, Rectangle *panelArea);
void GUIDrawVertexInspect(GUIState *state, Rectangle *panelArea);
void GUIDrawEdgeInspect(GUIState *state, Rectangle *panelArea);
void GUIDrawCreateEdge(GUIState *state, Rectangle *panelArea);
void GUIDrawPathPage(GUIState *state);
void GUIDrawAboutPage(GUIState *state);

// gui.c
void GUIInit(GUIState *state, const char *appName, const char *fontFile);
bool GUILoadGraph(GUIState *state, Graph *graph);
void GUIUnloadGraph(GUIState *state);
void GUIUpdate(GUIState *state);
void GUIDraw(GUIState *state);

#endif
