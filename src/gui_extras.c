#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include <gui_extras.h>
#include <raylib/raygui.h>
#include <stdio.h>
#include <constants.h>

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
    Color color = BLUE;
    if(selected) {
        color = RED;
    }

    DrawCircle(vert->position.x, vert->position.y, 20, color);
}

void DrawEdge(Vertex *start_vert, Edge *edge, bool selected) {
    Color color = ORANGE;
    if(selected) {
        color = RED;
    }

    DrawLineEx(
        start_vert->position,
        edge->target->position,
        5,
        color
    );

    Vector2 midPoint = Vector2Add(start_vert->position, edge->target->position);
    midPoint.x /= 2.0;
    midPoint.y /= 2.0;

    Vector2 dir = Vector2Subtract(edge->target->position, start_vert->position);

    Vector2 dirNormalized = Vector2Normalize(dir);
    Vector2 normal = dirNormalized;
    float t = normal.x;
    normal.x = -normal.y * 5;
    normal.y = t * 5;

    Vector2 arrowTarget = Vector2Subtract(
        edge->target->position,
        (Vector2){ dirNormalized.x * VERTEX_RADIUS, dirNormalized.y * VERTEX_RADIUS }
    );
    Vector2 arrowOrigin = Vector2Subtract(
        arrowTarget,
        (Vector2){ dirNormalized.x * 25, dirNormalized.y * 25 }
    );

    DrawLineEx(
        Vector2Add(arrowOrigin, normal),
        arrowTarget,
        5,
        color
    );

    DrawLineEx(
        Vector2Add(arrowOrigin, (Vector2){ -normal.x, -normal.y }),
        arrowTarget,
        5,
        color
    );

    normal.x *= 3;
    normal.y *= 3;

    Font font = GetFontDefault();

    char buff[32];
    snprintf(buff, 32, "%d", edge->weight);

    Vector2 textOrigin = MeasureTextEx(font, buff, 20, 2);
    textOrigin.x /= 2.0;
    textOrigin.y /= 2.0;

    DrawTextPro(
        font,
        buff,
        Vector2Add(midPoint, normal),
        textOrigin,
        atan2f(dir.y, dir.x) * RAD2DEG + 180 * (dir.x < 0 ? 1 : 0),
        20,
        2,
        GRAY
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
