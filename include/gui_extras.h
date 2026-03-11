#ifndef GUI_EXTRAS_H
#define GUI_EXTRAS_H

#include <raylib/raylib.h>

#include <graph.h>

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 600

#define PANEL_WIDTH 200
#define STATUS_BAR_HEIGHT 20

#define CANVAS_WIDTH (SCREEN_WIDTH - PANEL_WIDTH)
#define CANVAS_HEIGHT (SCREEN_HEIGHT - STATUS_BAR_HEIGHT)

void GuiOffsettedGrid(Camera2D camera, float spacing);

void DrawVertex(Vertex *vert, bool selected);
void DrawEdge(Vertex *start_vert, Edge *edge, bool selected);
void DrawGraph(Graph *graph);

#endif
