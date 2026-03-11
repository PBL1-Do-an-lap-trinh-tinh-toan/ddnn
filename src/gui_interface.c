#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include <gui_interface.h>

#define RAYGUI_STATIC
#define RAYGUI_IMPLEMENTATION
#include <raylib/raygui.h>

#include <graph.h>
#include <constants.h>
#include <string.h>

const float TARGET_DISTANCE = 500;
const float SPRING_STIFFNESS = 0.7;
const float COULOMB_CONSTANT = 0.121 * SPRING_STIFFNESS * TARGET_DISTANCE * TARGET_DISTANCE * TARGET_DISTANCE;

static void GuiOffsettedGrid(Camera2D camera, float spacing) {
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

static void DrawVertex(Vertex *vert, unsigned id, bool selected) {
    Color color = BLUE;
    if(selected) {
        color = RED;
    }

    DrawCircle(vert->position.x, vert->position.y, 20, color);
    const char *text = TextFormat("%d", id);
    Vector2 textOrigin = MeasureTextEx(GetFontDefault(), text, 20, 2);
    textOrigin.x /= -2.0;
    textOrigin.y /= -2.0;
    textOrigin = Vector2Add(textOrigin, vert->position);
    DrawText(TextFormat("%d", id), textOrigin.x, textOrigin.y, 20, RAYWHITE);
}

static void DrawEdge(Vertex *start_vert, Edge *edge, bool selected) {
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

    const char *text = TextFormat("%d", edge->weight);
    Vector2 textOrigin = MeasureTextEx(font, text, 20, 2);
    textOrigin.x /= 2.0;
    textOrigin.y /= 2.0;

    DrawTextPro(
        font,
        text,
        Vector2Add(midPoint, normal),
        textOrigin,
        atan2f(dir.y, dir.x) * RAD2DEG + 180 * (dir.x < 0 ? 1 : 0),
        20,
        2,
        GRAY
    );
}

static void DrawGraph(GUIState *state) {
    Graph *graph = state->graph;
    for(unsigned i = 0; i < graph->vertex_count; i++) {
        Vertex *vert = graph->vertices[i];
        for(unsigned j = 0; j < vert->adjacent_count; j++)
            DrawEdge(vert, vert->adjacents[j], 0);
    }

    for(unsigned i = 0; i < graph->vertex_count; i++) {
        Vertex *vert = graph->vertices[i];
        bool selected = (vert == state->startVertex) || (vert == state->endVertex);
        DrawVertex(vert, i, selected);
    }
}

void GUIInit(GUIState *state, const char *appName) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, appName);
    SetTargetFPS(60);

    state->graph = NULL;
    state->startVertex = NULL;
    state->endVertex = NULL;
    state->selectedVertex = NULL;

    state->bodies = NULL;

    state->camera = (Camera2D){ 0 };
    state->camera.target = Vector2Zero();
    state->camera.offset = (Vector2){ (float)CANVAS_WIDTH / 2, (float)CANVAS_HEIGHT / 2 };
    state->camera.rotation = 0.0f;
    state->camera.zoom = 1.0f;

    state->physicsEnabled = true;

    strcpy(state->statusBar, "ddnn ver 0.1.0");
}

bool GUILoadGraph(GUIState *state, Graph *graph) {
    state->graph = graph;
    state->bodies = (Body*)malloc(sizeof(Body) * graph->max_vertex_count);
    if(!state->bodies) return 0;

    for(unsigned i = 0; i < graph->vertex_count; i++) {
        Vertex *vert = graph->vertices[i];
        BodyInit(state->bodies + i, &vert->position);
        
        vert->position.x = 67.8 * i * cos(i * 0.57);
        vert->position.y = 67.8 * i * sin(i * 0.57);
    }

    return 1;
}

void GUIUnloadGraph(GUIState *state) {
    delete_graph(state->graph);
    free(state->bodies);

    state->graph = NULL;
    state->startVertex = NULL;
    state->endVertex = NULL;
    state->selectedVertex = NULL;

    state->bodies = NULL;

    strcpy(state->statusBar, "Da go do thi");
}

void GUIUpdate(GUIState *state) {
    if(IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        state->camera.target = Vector2Subtract(
            state->camera.target,
            Vector2Divide(GetMouseDelta(), (Vector2){ state->camera.zoom, state->camera.zoom })
        );
    }

    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && state->graph) {
        Vector2 mousePos = GetMousePosition();
        if(
            mousePos.x < SCREEN_WIDTH - PANEL_WIDTH
            && mousePos.y < SCREEN_HEIGHT - STATUS_BAR_HEIGHT
        ) {
            state->selectedVertex = NULL;

            Vector2 worldMouse = GetScreenToWorld2D(mousePos, state->camera);

            for(unsigned i = 0; i < state->graph->vertex_count; i++) {
                Vertex *vert = state->graph->vertices[i];
                float dx = worldMouse.x - vert->position.x;
                float dy = worldMouse.y - vert->position.y;
                if((dx * dx + dy * dy) <= VERTEX_RADIUS * VERTEX_RADIUS) {
                    state->selectedVertex = vert;
                    break;
                }
            }
        }
    }

    float mouse_wheel = GetMouseWheelMove();
    if(mouse_wheel > 0 && state->camera.zoom < 3) {
        state->camera.zoom *= 1.2;
    } else if (mouse_wheel < 0 && state->camera.zoom > 0.1) {
        state->camera.zoom /= 1.2;
    }

    if(state->physsimButton && state->bodies) {
        state->physicsEnabled = !state->physicsEnabled;
        if(!state->physicsEnabled) {
            for(unsigned i = 0; i < state->graph->vertex_count; i++) {
                state->bodies[i].velocity.x = 0;
                state->bodies[i].velocity.y = 0;
            }
            strcpy(state->statusBar, "Da tat gia lap vat ly");
        } else {
            strcpy(state->statusBar, "Da bat gia lap vat ly");
        }
    }

    if(state->physicsEnabled && state->graph && state->bodies) {
        Graph *graph = state->graph;
        Body *bodies = state->bodies;
        float deltaTime = Clamp(GetFrameTime(), 0, 0.5);

        for(unsigned i = 0; i < graph->vertex_count; i++) {
            Vertex *vert1 = graph->vertices[i];

            // spring force
            for(unsigned j = 0; j < vert1->adjacent_count; j++) {
                unsigned id1 = i;
                unsigned id2 = vert1->adjacents[j]->target->id;

                Vector2 p1 = *bodies[id1].position;
                Vector2 p2 = *bodies[id2].position;
                Vector2 dir = Vector2Subtract(p1, p2);

                float forceMag = SPRING_STIFFNESS * (TARGET_DISTANCE - Vector2Length(dir));
                Vector2 force = Vector2Normalize(dir);
                force.x *= forceMag;
                force.y *= forceMag;

                ApplyForce(&bodies[id1], force, deltaTime);
                ApplyForce(&bodies[id2], (Vector2){ -force.x, -force.y }, deltaTime);
            }

            // coulomb force
            for(unsigned j = i + 1; j < graph->vertex_count; j++) {
                Vector2 p1 = *bodies[i].position;
                Vector2 p2 = *bodies[j].position;
                Vector2 dir = Vector2Subtract(p1, p2);

                float forceMag = COULOMB_CONSTANT / Vector2LengthSqr(dir);
                Vector2 force = Vector2Normalize(dir);
                force.x *= forceMag;
                force.y *= forceMag;

                ApplyForce(&bodies[i], force, deltaTime);
                ApplyForce(&bodies[j], (Vector2){ -force.x, -force.y }, deltaTime);
            }

            // avoid drifting
            Vector2 anchorForce = vert1->position;
            anchorForce.x *= -1.3;
            anchorForce.y *= -1.3;
            ApplyForce(
                &bodies[i],
                anchorForce,
                deltaTime
            );
        }

        for(unsigned i = 0; i < graph->vertex_count; i++) {
            if(isnan(bodies[i].velocity.x) || isnan(bodies[i].velocity.y)) {
                bodies[i].velocity.x = 0;
                bodies[i].velocity.y = 0;
            }

            bodies[i].velocity.x *= 0.99;
            bodies[i].velocity.y *= 0.99;

            bodies[i].velocity = Vector2Clamp(
                bodies[i].velocity,
                (Vector2){ -25, -25 },
                (Vector2){ 25, 25 }
            );
            Inertia(&bodies[i], deltaTime);
        }
    }
}

void GUIDraw(GUIState *state) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode2D(state->camera);
    GuiOffsettedGrid(state->camera, 20);
    DrawGraph(state);
    EndMode2D();

    Rectangle panelArea = (Rectangle){
        SCREEN_WIDTH - PANEL_WIDTH,
        0,
        PANEL_WIDTH,
        SCREEN_HEIGHT - STATUS_BAR_HEIGHT + 1
    };
    GuiPanel(panelArea, "Dieu khien");

    float currentY = 29;
    float margin = 5;
    float itemWidth = PANEL_WIDTH - (margin * 2);

    if(!state->selectedVertex) {
        if(state->graph) GuiDisable();
        GuiButton((Rectangle){ panelArea.x + margin, currentY, (itemWidth - 5) / 2, 30 }, "Tai do thi");
        GuiEnable();
        if(!state->graph) GuiDisable();
        GuiButton((Rectangle){ panelArea.x + (itemWidth - 5) / 2 + margin * 2, currentY, (itemWidth - 5) / 2, 30 }, "Go do thi");
        GuiEnable();
        currentY += 35;

        state->physsimButton = GuiButton(
            (Rectangle){
                SCREEN_WIDTH - PANEL_WIDTH + 5,
                currentY,
                itemWidth,
                30
            },
            state->physicsEnabled ? "Dung gia lap vat ly" : "Bat dau gia lap vat ly"
        );
        currentY += 35;

        // if(state->startVertex && state->endVertex) {
        //
        // }
    } else {
        Vertex *vert = state->selectedVertex;

        GuiLabel(
            (Rectangle){ panelArea.x + margin, currentY, itemWidth, 20 },
            TextFormat("ID: %u", vert->id)
        );
        currentY += 15;

        GuiDisable();
        GuiLabel((Rectangle){ panelArea.x + margin, currentY, itemWidth, 20 }, "Vi tri va van toc");
        currentY += 10;
        GuiLabel(
            (Rectangle){ panelArea.x + margin, currentY, itemWidth, 30 },
            TextFormat(
                "P: %.1f, %.1f | V: %.1f",
                vert->position.x,
                vert->position.y,
                state->bodies[vert->id].velocity.x
            )
        );
        GuiEnable();
        currentY += 25;

        GuiButton((Rectangle){ panelArea.x + margin, currentY, itemWidth, 30 }, "Dat lam diem bat dau");
        currentY += 35;

        GuiButton((Rectangle){ panelArea.x + margin, currentY, itemWidth, 30 }, "Dat lam diem ket thuc");
        currentY += 35;

        GuiLabel((Rectangle){ panelArea.x + margin, currentY, itemWidth, 20 }, "Dinh ke");
        currentY += 15;
        for (unsigned i = 0; i < vert->adjacent_count; i++) {
            GuiLabel((Rectangle){ panelArea.x + margin + 10, currentY, itemWidth, 20 }, 
                     TextFormat("-> ID: %u (w: %u)", vert->adjacents[i]->target->id, vert->adjacents[i]->weight));
            currentY += 10;
            if (currentY > SCREEN_HEIGHT - 100) break;
        }

    }

    GuiStatusBar(
        (Rectangle){
            0,
            SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
            SCREEN_WIDTH,
            STATUS_BAR_HEIGHT
        },
        state->statusBar
    );

    EndDrawing();
}
