#ifndef GUI_EXTRAS_H
#define GUI_EXTRAS_H

#include <raylib/raylib.h>

#include <graph.h>

static const int SCREEN_WIDTH = 1200;
static const int SCREEN_HEIGHT = 600;
 
static const int PANEL_WIDTH = 200;
static const int STATUS_BAR_HEIGHT = 20;
 
static const int CANVAS_WIDTH = SCREEN_WIDTH - PANEL_WIDTH;
static const int CANVAS_HEIGHT = SCREEN_HEIGHT - STATUS_BAR_HEIGHT;

void GuiOffsettedGrid(Camera2D camera, float spacing);

void DrawVertex(Vertex *vert, bool selected);
void DrawEdge(Vertex *start_vert, Edge *edge, bool selected);
void DrawGraph(Graph *graph);

#endif
