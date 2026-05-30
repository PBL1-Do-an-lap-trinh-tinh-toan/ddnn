#include <raylib/raygui.h>
#include <gui_interface.h>

#include <graph.h>
#include <string.h>
#include <constants.h>

static void drawVertex(Vertex *vert, Color color, Font font) {
    DrawCircleV(vert->position, VERTEX_RADIUS, GRAY);
    DrawCircleV(vert->position, VERTEX_RADIUS - 3, color);
    const char *text = TextFormat("%d", vert->id);
    Vector2 textOrigin = MeasureTextEx(font, text, 25, 2);
    textOrigin.x /= -2.0;
    textOrigin.y /= -2.0;
    textOrigin = Vector2Add(textOrigin, vert->position);
    DrawTextEx(font, TextFormat("%d", vert->id), textOrigin, 25, 2, BLACK);
}

static void drawEdge(Edge *edge, Color color, Font font, bool mutual_adjacency) {
    Vertex *startVert = edge->origin;
    Vector2 dir = Vector2Subtract(edge->target->position, startVert->position);

    Vector2 dirNormalized = Vector2Normalize(dir);
    Vector2 normal = dirNormalized;
    float t = normal.x;
    normal.x = -normal.y * 5;
    normal.y = t * 5;

    Vector2 offset = Vector2Scale(normal, 1.3f * mutual_adjacency);

    Vector2 midPoint = Vector2Add(startVert->position, edge->target->position);
    midPoint.x /= 2.0;
    midPoint.y /= 2.0;
    midPoint = Vector2Add(midPoint, offset);

    Vector2 arrowTarget = Vector2Subtract(
        edge->target->position,
        (Vector2){ dirNormalized.x * VERTEX_RADIUS, dirNormalized.y * VERTEX_RADIUS }
    );
    Vector2 arrowOrigin = Vector2Subtract(
        arrowTarget,
        (Vector2){ dirNormalized.x * 25, dirNormalized.y * 25 }
    );

    arrowOrigin = Vector2Add(arrowOrigin, offset);
    arrowTarget = Vector2Add(arrowTarget, offset);

    DrawLineEx(
        Vector2Add(arrowOrigin, normal),
        arrowTarget,
        EDGE_WIDTH,
        color
    );

    DrawLineEx(
        Vector2Add(arrowOrigin, (Vector2){ -normal.x, -normal.y }),
        arrowTarget,
        EDGE_WIDTH,
        color
    );

    DrawLineEx(
        Vector2Add(startVert->position, offset),
        Vector2Add(edge->target->position, offset),
        EDGE_WIDTH,
        color
    );

    normal.x *= 3;
    normal.y *= 3;

    const char *text = TextFormat("%d", edge->weight);
    Vector2 textOrigin = MeasureTextEx(font, text, 25, 2);
    textOrigin.x /= 2.0;
    textOrigin.y /= 2.0;

    float angle = atan2f(dir.y, dir.x) * RAD2DEG + 180 * (dir.x < 0 ? 1 : 0);

    // rim
    DrawTextPro(
        font,
        text,
        Vector2Add(midPoint, normal),
        textOrigin,
        angle,
        26,
        2,
        RAYWHITE
    );

    // fg
    DrawTextPro(
        font,
        text,
        Vector2Add(midPoint, normal),
        textOrigin,
        angle,
        25,
        2,
        color
    );
}

void GUIDrawGraph(GUIState *state) {
    Graph *graph = state->graph;
    for(unsigned i = 0; i < graph->vertex_count; i++) {
        Vertex *vert = graph->vertices[i];
        for(unsigned j = 0; j < vert->adjacent_count; j++) {
            Vertex *target = vert->adjacents[j]->target;
            bool mutual_adjacency = false;
            mutual_adjacency = find_edge(target, vert);
            drawEdge(vert->adjacents[j], GRAY, state->font, mutual_adjacency);
        }
    }

    // highlight shortest path
    if(
        state->pathStartVertex
        && state->pathEndVertex
        && state->shortestPathResult != NO_PATH
    ) {
        Vertex *current = state->pathEndVertex;
        while(current != state->pathStartVertex) {
            bool mutual_adjacency = false;
            mutual_adjacency = find_edge(current, current->path_prev);
            drawEdge(
                find_edge(current->path_prev, current),
                ORANGE,
                state->font,
                mutual_adjacency
            );
            current = current->path_prev; 
        }
    }

    if(state->selectedEdge)
        drawEdge(
            state->selectedEdge,
            MAROON,
            state->font,
            find_edge(state->selectedEdge->target, state->selectedEdge->origin) != NULL
        );

    for(unsigned i = 0; i < graph->vertex_count; i++) {
        Vertex *vert = graph->vertices[i];
        drawVertex(vert, RAYWHITE, state->font);
    }

    if(state->pathStartVertex)
        drawVertex(state->pathStartVertex, GREEN, state->font);

    if(state->pathEndVertex)
        drawVertex(state->pathEndVertex, BLUE, state->font);

    if(state->selectedVertex)
        drawVertex(state->selectedVertex, MAROON, state->font);

    if(state->edgeStartVertex) {
        Vertex previewTarget;
        if(!state->selectedVertex)
            previewTarget.position = GetScreenToWorld2D(GetMousePosition(), state->camera);
        else
            previewTarget.position = state->selectedVertex->position;
        Edge previewEdge;
        previewEdge.origin = state->edgeStartVertex;
        previewEdge.target = &previewTarget;
        previewEdge.weight = 0;
        drawEdge(&previewEdge, MAROON, state->font, false);

        drawVertex(state->edgeStartVertex, PURPLE, state->font);
    }
}

