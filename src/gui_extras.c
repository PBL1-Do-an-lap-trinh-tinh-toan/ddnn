#include "raylib/raylib.h"
#include <gui_extras.h>
#include <raylib/raygui.h>

static Vertex *startVertex = 0;
static Vertex *endVertex = 0;

void GuiOffsettedGrid(Camera2D camera, float spacing) {
    Vector2 topLeft = GetScreenToWorld2D((Vector2){0, 0}, camera);
    Vector2 bottomRight = GetScreenToWorld2D((Vector2){CANVAS_WIDTH, CANVAS_HEIGHT}, camera);

    int startX = (int)floorf(topLeft.x / spacing);
    int endX = (int)ceilf(bottomRight.x / spacing);
    int startY = (int)floorf(topLeft.y / spacing);
    int endY = (int)ceilf(bottomRight.y / spacing);

    Color color = LIGHTGRAY;
    for (int x = startX; x <= endX; x++) {
        DrawLine(x * spacing, startY * spacing, x * spacing, endY * spacing, color);
    }

    for (int y = startY; y <= endY; y++) {
        DrawLine(startX * spacing, y * spacing, endX * spacing, y * spacing, color);
    }
}

void DrawVertex(Vertex *vert, bool selected) {
    DrawCircle(vert->position.x, vert->position.y, 20, BLUE);
}

void DrawEdge(Vertex *start_vert, Edge *edge, bool selected) {
    DrawLineEx(
        start_vert->position,
        edge->target->position,
        5,
        ORANGE
    );
}

void DrawGraph(Graph *graph) {
    for(unsigned i = 0; i < graph->vertex_count; i++) {
        Vertex *vert = graph->vertices[i];
        for(unsigned j = 0; j < vert->adjacent_count; j++)
            DrawEdge(vert, vert->adjacents[j], 0);
    }

    for(unsigned i = 0; i < graph->vertex_count; i++) {
        Vertex *vert = graph->vertices[i];
        bool selected = (vert == startVertex) || (vert == endVertex);
        DrawVertex(vert, selected);
    }
}
