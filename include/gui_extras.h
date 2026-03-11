#ifndef GUI_EXTRAS_H
#define GUI_EXTRAS_H

#include <raylib/raylib.h>
#include <graph.h>

void GuiOffsettedGrid(Camera2D camera, float spacing);

void DrawVertex(Vertex *vert, bool selected);
void DrawEdge(Vertex *start_vert, Edge *edge, bool selected);
void DrawGraph(Graph *graph);

#endif
