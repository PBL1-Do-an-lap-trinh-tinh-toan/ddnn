#include <raylib/raylib.h>
#include <gui_interface.h>

#define RAYGUI_STATIC
#define RAYGUI_IMPLEMENTATION
#include <raylib/raygui.h>

#include <graph.h>
#include <constants.h>
#include <string.h>

const float MARGIN = 5;
const float ITEM_WIDTH = PANEL_WIDTH - (MARGIN * 2);
const unsigned START_Y = 39;

static Graph *RandomGraph() {
    unsigned vertexCount = GetRandomValue(8, 15);
    Graph *graph = make_graph(vertexCount * 2);
    
    for(unsigned i = 0; i < vertexCount; i++) {
        if(!add_vertex(graph)) {
            delete_graph(graph);
            return NULL;
        }
    }

    for(unsigned i = 1; i < vertexCount; i++) {
        if(!make_edge(graph->vertices[i - 1], graph->vertices[i], GetRandomValue(5, 15))) {
            delete_graph(graph);
            return NULL;
        }
    }

    for(unsigned i = 2; i < vertexCount; i++) {
        if (GetRandomValue(1, 100) <= 50) { 
            unsigned randomPrevious = GetRandomValue(0, i - 2); 
            if(!make_edge(
                graph->vertices[randomPrevious],
                graph->vertices[i],
                GetRandomValue(1, 25) 
            )) {
                delete_graph(graph);
                return NULL;
            }
        }
    }

    unsigned numRetrograde = GetRandomValue(2, 4);
    for(unsigned k = 0; k < numRetrograde; k++) {
        unsigned fromIdx = GetRandomValue(3, vertexCount - 1);
        unsigned toIdx = GetRandomValue(0, fromIdx - 2);

        if(!make_edge(
            graph->vertices[fromIdx],
            graph->vertices[toIdx],
            GetRandomValue(1, 8) 
        )) {
            delete_graph(graph);
            return NULL;
        }
    }

    return graph;
}

static void GUIDrawOffsettedGrid(Camera2D camera, float spacing) {
    Vector2 topLeft = GetScreenToWorld2D((Vector2){0, 0}, camera);
    Vector2 bottomRight = GetScreenToWorld2D(
        (Vector2){GetScreenWidth() - PANEL_WIDTH, GetScreenHeight() - STATUS_BAR_HEIGHT},
        camera
    );

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

static void GUIDrawVertex(Vertex *vert, Color color, Font font) {
    DrawCircleV(vert->position, VERTEX_RADIUS, GRAY);
    DrawCircleV(vert->position, VERTEX_RADIUS - 3, color);
    const char *text = TextFormat("%d", vert->id);
    Vector2 textOrigin = MeasureTextEx(font, text, 25, 2);
    textOrigin.x /= -2.0;
    textOrigin.y /= -2.0;
    textOrigin = Vector2Add(textOrigin, vert->position);
    DrawTextEx(font, TextFormat("%d", vert->id), textOrigin, 25, 2, BLACK);
}

static void GUIDrawEdge(Vertex *start_vert, Edge *edge, Color color, Font font, bool mutual_adjacency) {
    Vector2 dir = Vector2Subtract(edge->target->position, start_vert->position);

    Vector2 dirNormalized = Vector2Normalize(dir);
    Vector2 normal = dirNormalized;
    float t = normal.x;
    normal.x = -normal.y * 5;
    normal.y = t * 5;

    Vector2 offset = Vector2Scale(normal, 1.3f * mutual_adjacency);

    Vector2 midPoint = Vector2Add(start_vert->position, edge->target->position);
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
        Vector2Add(start_vert->position, offset),
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

    DrawTextPro(
        font,
        text,
        Vector2Add(midPoint, normal),
        textOrigin,
        atan2f(dir.y, dir.x) * RAD2DEG + 180 * (dir.x < 0 ? 1 : 0),
        25,
        2,
        color
    );
}

static void GUIDrawGraph(GUIState *state) {
    Graph *graph = state->graph;
    for(unsigned i = 0; i < graph->vertex_count; i++) {
        Vertex *vert = graph->vertices[i];
        for(unsigned j = 0; j < vert->adjacent_count; j++) {
            Vertex *target = vert->adjacents[j]->target;
            bool mutual_adjacency = false;
            mutual_adjacency = find_edge(target, vert);
            GUIDrawEdge(vert, vert->adjacents[j], GRAY, state->font, mutual_adjacency);
        }
    }

    // highlight shortest path
    if(state->pathStartVertex && state->pathEndVertex && (state->shortestPathResult >= 0)) {
        Vertex *current = state->pathEndVertex;
        while(current != state->pathStartVertex) {
            bool mutual_adjacency = false;
            mutual_adjacency = find_edge(current, current->path_prev);
            GUIDrawEdge(
                current->path_prev,
                find_edge(current->path_prev, current),
                ORANGE,
                state->font,
                mutual_adjacency
            );
            current = current->path_prev; 
        }
    }

    if(state->selectedEdge)
        GUIDrawEdge(
            state->selectedEdge->origin,
            state->selectedEdge,
            MAROON,
            state->font,
            find_edge(state->selectedEdge->target, state->selectedEdge->origin) != NULL
        );

    for(unsigned i = 0; i < graph->vertex_count; i++) {
        Vertex *vert = graph->vertices[i];
        GUIDrawVertex(vert, RAYWHITE, state->font);
    }

    if(state->pathStartVertex)
        GUIDrawVertex(state->pathStartVertex, GREEN, state->font);

    if(state->pathEndVertex)
        GUIDrawVertex(state->pathEndVertex, BLUE, state->font);

    if(state->selectedVertex)
        GUIDrawVertex(state->selectedVertex, MAROON, state->font);

    if(state->edgeStartVertex)
        GUIDrawVertex(state->edgeStartVertex, PURPLE, state->font);
}

static void GUIGetObjectUnderCursor(GUIState *state, Vector2 worldMouse) {
    state->selectedVertex = NULL;
    state->selectedEdge = NULL;

    // check edges
    float minDistSqr = EDGE_WIDTH * EDGE_WIDTH * 4;
    for(unsigned i = 0; i < state->graph->vertex_count; i++) {
        Vertex *vert = state->graph->vertices[i];
        for(unsigned j = 0; j < vert->adjacent_count; j++) {
            Edge *edge = vert->adjacents[j];
            Vertex *target = edge->target;
            bool mutualAdjacency = find_edge(target, vert) != NULL;

            Vector2 edge_dir = Vector2Subtract(edge->target->position, vert->position);
            float edgeLenSqr = Vector2LengthSqr(edge_dir);
            if(edgeLenSqr < 40.0 * 40.0) continue;

            Vector2 dirNormalized = Vector2Normalize(edge_dir);
            Vector2 normal = { -dirNormalized.y * 5, dirNormalized.x * 5 };
            Vector2 offset = Vector2Scale(normal, 1.3f * mutualAdjacency);

            Vector2 vertPosOffsetted = Vector2Add(vert->position, offset);

            Vector2 event_dir = Vector2Subtract(worldMouse, vertPosOffsetted);
            float t = Vector2DotProduct(event_dir, edge_dir) / edgeLenSqr;
            t = Clamp(t, 0.0, 1.0);
            Vector2 proj = Vector2Add(vertPosOffsetted, Vector2Scale(edge_dir, t));
            float distance = Vector2LengthSqr(Vector2Subtract(worldMouse, proj));

            if(distance < minDistSqr) {
                state->selectedEdge = edge;
                minDistSqr = distance;
            }
        }
    }

    // prioritise vertices over edges
    for(unsigned i = 0; i < state->graph->vertex_count; i++) {
        Vertex *vert = state->graph->vertices[i];
        Vector2 dir = Vector2Subtract(worldMouse, vert->position);
        if(Vector2LengthSqr(dir) <= VERTEX_RADIUS * VERTEX_RADIUS) {
            state->selectedVertex = vert;
            state->selectedEdge = NULL;
            break;
        }
    }
}

static void GUIUpdatePhysics(GUIState *state) {
    if(!state->physicsEnabled || !state->graph)
        return;

    Graph *graph = state->graph;
    float deltaTime = Clamp(GetFrameTime(), 0, 0.5);

    for(unsigned i = 0; i < graph->vertex_count; i++) {
        Vertex *vert1 = graph->vertices[i];

        // spring force
        for(unsigned j = 0; j < vert1->adjacent_count; j++) {
            Vertex *vert2 = vert1->adjacents[j]->target;

            Vector2 p1 = vert1->position;
            Vector2 p2 = vert2->position;
            Vector2 dir = Vector2Subtract(p1, p2);

            float forceMag = state->springStiffness * (state->springLength - Vector2Length(dir));
            Vector2 force = Vector2Scale(Vector2Normalize(dir), forceMag);

            ApplyForce(vert1, force, deltaTime);
            ApplyForce(vert2, Vector2Scale(force, -1.0f), deltaTime);
        }

        // coulomb force
        for(unsigned j = i + 1; j < graph->vertex_count; j++) {
            Vertex *vert2 = graph->vertices[j];
            Vector2 p1 = vert1->position;
            Vector2 p2 = vert2->position;
            Vector2 dir = Vector2Subtract(p1, p2);

            float forceMag = state->coulombConstant / (Vector2LengthSqr(dir) + 0.1f);
            Vector2 force = Vector2Scale(Vector2Normalize(dir), forceMag);

            ApplyForce(vert1, force, deltaTime);
            ApplyForce(vert2, Vector2Scale(force, -1.0f), deltaTime);
        }

        // avoid drifting
        Vector2 anchorForce = Vector2Scale(vert1->position, -ANCHOR_FORCE_MAG);
        ApplyForce(vert1, anchorForce, deltaTime);
    }

    for(unsigned i = 0; i < graph->vertex_count; i++) {
        Vertex *vert = graph->vertices[i];
        if(isnan(vert->velocity.x) || isnan(vert->velocity.y)) {
            vert->velocity.x = 0;
            vert->velocity.y = 0;
        }

        vert->velocity.x *= VELOCITY_DAMPENING;
        vert->velocity.y *= VELOCITY_DAMPENING;

        vert->velocity = Vector2Clamp(
            vert->velocity,
            (Vector2){ -VELOCITY_CAP, -VELOCITY_CAP },
            (Vector2){ VELOCITY_CAP, VELOCITY_CAP }
        );
        Inertia(vert, deltaTime);
    }
}

static void GUIFindShortestPath(GUIState *state) {
    if(!state->pathStartVertex || !state->pathEndVertex)
        return;

    state->shortestPathResult = shortest_path(state->graph, state->pathStartVertex, state->pathEndVertex);
    if(state->shortestPathResult>= 0) {
        strcpy(
            state->statusBar,
            TextFormat(
                "Tìm được đường đi từ đỉnh %d đến đỉnh %d với chi phí tối thiểu là %lld",
                state->pathStartVertex->id,
                state->pathEndVertex->id,
                state->shortestPathResult
            )
        );
    } else {
        strcpy(
            state->statusBar,
            TextFormat(
                "Không tồn tại đường đi từ đỉnh %d đến đỉnh %d",
                state->pathStartVertex->id,
                state->pathEndVertex->id
            )
        );
    }
}

static void GUIAddVertex(GUIState *state) {
    if(!state->graph) {
        state->graph = make_graph(100);
        if(!state->graph) {
            snprintf(state->statusBar, sizeof(state->statusBar), "%s", "Tạo đồ thị không thành công");
        }
    }

    if(state->graph) {
        Vertex *new_vert = add_vertex(state->graph);
        if(!new_vert) {
            snprintf(state->statusBar, sizeof(state->statusBar), "%s", "Đã đạt số lượng đỉnh tối đa");
        } else {
            BodyInit(new_vert);
            float randAngle = GetRandomValue(-10, 10) / 10.0 * 2 * M_PI;
            ApplyForce(
                new_vert,
                (Vector2){
                    10.0f * cos(randAngle),
                    10.0f * sin(randAngle),
                },
                1.0f
            );
            state->selectedVertex = new_vert;
            snprintf(state->statusBar, sizeof(state->statusBar), "%s", TextFormat("Đã thêm đỉnh %d", new_vert->idx));
        }
    }
}

static void GUISwapStartEndVert(GUIState *state) {
    Vertex *t = state->pathStartVertex;
    state->pathStartVertex = state->pathEndVertex;
    state->pathEndVertex = t;
    state->shortestPathResult = -1;
    snprintf(state->statusBar, sizeof(state->statusBar), "%s","Đã hoán đổi đỉnh bắt đầu và đỉnh kết thúc");
}

static void GUITogglePhysicsSim(GUIState *state) {
    if(!state->graph) return;
    
    state->physicsEnabled = !state->physicsEnabled;
    if(!state->physicsEnabled) {
        snprintf(state->statusBar, sizeof(state->statusBar), "%s", "Đã tắt mô phỏng vật lý");
    } else {
        snprintf(state->statusBar, sizeof(state->statusBar), "%s", "Đã bật mô phỏng vật lý");
    }
}

static void GUIJiggleVert(GUIState *state) {
    if(!state->graph) return;
    
    SetRandomSeed((unsigned)GetTime());
    for(unsigned i = 0; i < state->graph->vertex_count; i++) {
        float theta = GetRandomValue(0, 360) * DEG2RAD;
        Vector2 dir = Vector2Scale((Vector2){cos(theta), sin(theta)}, VELOCITY_CAP);
        state->graph->vertices[i]->velocity.x += dir.x;
        state->graph->vertices[i]->velocity.y += dir.y;
    }
}

static void GUISetStartVert(GUIState *state) {
    Vertex *sel_vert = state->selectedVertex;
    if(!sel_vert) return;

    if(state->pathStartVertex != sel_vert) {
        state->pathStartVertex = sel_vert;
        if(state->pathEndVertex == sel_vert)
            state->pathEndVertex = NULL;
        snprintf(state->statusBar, sizeof(state->statusBar), "%s", TextFormat("Đã chọn đỉnh %d làm đỉnh bắt đầu", sel_vert->id));
    } else {
        state->pathStartVertex = NULL;
        snprintf(state->statusBar, sizeof(state->statusBar), "%s", TextFormat("Đã bỏ chọn đỉnh %d làm đỉnh bắt đầu", sel_vert->id));
    }

    if(state->pathStartVertex && state->pathEndVertex) GUIFindShortestPath(state);
}

static void GUISetEndVert(GUIState *state) {
    Vertex *sel_vert = state->selectedVertex;
    if(!sel_vert) return;

    if(state->pathEndVertex != sel_vert) {
        state->pathEndVertex = sel_vert;
        if(state->pathStartVertex == sel_vert)
            state->pathStartVertex = NULL;
        snprintf(state->statusBar, sizeof(state->statusBar), "%s", TextFormat("Đã chọn đỉnh %d làm đỉnh kết thúc", sel_vert->id));
    } else {
        state->pathEndVertex = NULL;
        snprintf(state->statusBar, sizeof(state->statusBar), "%s", TextFormat("Đã bỏ chọn đỉnh %d làm đỉnh kết thúc", sel_vert->id));
    }

    if(state->pathStartVertex && state->pathEndVertex) GUIFindShortestPath(state);
}

static void GUIDeleteEdge(GUIState *state) {
    if(!state->selectedEdge) return;
    
    unsigned from_id = state->selectedEdge->origin->id;
    unsigned to_id = state->selectedEdge->target->id;
    remove_edge(state->selectedEdge);

    state->selectedEdge = NULL;

    snprintf(state->statusBar, sizeof(state->statusBar), "%s", TextFormat("Đã xóa cạnh %u -> %u", from_id, to_id));
}

static void GUIReverseEdge(GUIState *state) {
    if(!state->selectedEdge || !state->graph) return;
    
    Vertex *from = state->selectedEdge->origin;
    Vertex *to = state->selectedEdge->target;
    unsigned from_id = from->id;
    unsigned to_id = to->id;
    unsigned long long weight = state->selectedEdge->weight;

    Edge *new_edge = make_edge(to, from, weight);

    if(new_edge) {
        remove_edge(state->selectedEdge);
        state->selectedEdge = new_edge;
        snprintf(state->statusBar, sizeof(state->statusBar), "%s", TextFormat(
            "Đã đảo chiều cạnh %u -> %u thành %u -> %u",
            from_id,
            to_id,
            to_id,
            from_id
        ));
    } else {
        snprintf(state->statusBar, sizeof(state->statusBar), "%s", TextFormat(
            "Đảo chiều cạnh %u -> %u không thành công, đã đạt số lượng cạnh tối đa hoặc hết bộ nhớ",
            from_id,
            to_id,
            to_id,
            from_id
        ));
    }

}

static void GUIDeleteVert(GUIState *state) {
    Vertex *sel_vert = state->selectedVertex;
    if(!state->graph || !sel_vert) return;

    unsigned vert_id = sel_vert->id;
    remove_vertex(state->graph, sel_vert->id);
    state->selectedVertex = NULL;
    sel_vert = NULL;
    state->shortestPathResult = -1;

    snprintf(state->statusBar, sizeof(state->statusBar), "%s", TextFormat("Đã xóa đỉnh %u", vert_id));
}

static void GUIDrawNormalView(GUIState *state, Rectangle *panelArea) {
    int currentY = START_Y;

    GuiLine(
        (Rectangle){
            panelArea->x + MARGIN,
            currentY,
            ITEM_WIDTH,
            3
        },
        "Đồ thị"
    );
    currentY += 23;

    if(state->graph) GuiDisable();
    GuiButton(
        (Rectangle){
            panelArea->x + MARGIN,
            currentY,
            (ITEM_WIDTH - 5) / 2,
            30
        },
        "Tải đồ thị"
    );
    GuiEnable();
    if(!state->graph) GuiDisable();
    if(GuiButton(
        (Rectangle){ panelArea->x + (
            ITEM_WIDTH - 5
        ) / 2 + MARGIN * 2, currentY, (ITEM_WIDTH - 5) / 2, 30 },
        "Gỡ đồ thị"
    )) {
        GUIUnloadGraph(state);
    }
    GuiEnable();
    currentY += 35;

    if(state->graph) GuiDisable();
    if(GuiButton(
        (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 30 },
        "Tạo đồ thị ngẫu nhiên"
    )) {
        SetRandomSeed((unsigned)GetTime());
        Graph *rGraph = RandomGraph();
        if(rGraph) {
            GUILoadGraph(state, rGraph);
        } else {
            snprintf(state->statusBar, sizeof(state->statusBar), "%s", TextFormat(
                "Tạo đồ thị ngẫu nhiên không thành công (hết bộ nhớ?)"
            ));
        }
    }
    GuiEnable();
    currentY += 35;

    if(GuiButton((Rectangle){
        panelArea->x + MARGIN,
        currentY,
        ITEM_WIDTH,
        30
    }, "Tạo đỉnh mới")) GUIAddVertex(state);
    currentY += 35;

    if(GuiButton((Rectangle){
        panelArea->x + MARGIN,
        currentY,
        ITEM_WIDTH,
        30
    }, "Tạo cạnh mới")) {
        state->current_mode = MODE_CREATE_EDGE;
    }
    currentY += 35;

    if(GuiButton((Rectangle){
        panelArea->x + MARGIN,
        currentY,
        ITEM_WIDTH,
        30
    }, "Đảo đỉnh bắt đầu/kết thúc")) GUISwapStartEndVert(state);
    currentY += 35;


    if(!state->pathStartVertex || !state->pathEndVertex)
        GuiDisable();
    if(GuiButton((Rectangle){
        panelArea->x + MARGIN,
        currentY,
        ITEM_WIDTH,
        30
    }, "Tìm đường đi ngắn nhất")) GUIFindShortestPath(state);
    currentY += 35;
    GuiEnable();

    currentY += 15;
    GuiLine(
        (Rectangle){
            panelArea->x + MARGIN,
            currentY,
            ITEM_WIDTH,
            3
        },
        "Mô phỏng vật lý"
    );
    currentY += 23;

    if(GuiButton(
        (Rectangle){
            panelArea->x + MARGIN,
            currentY,
            ITEM_WIDTH,
            30
        },
        state->physicsEnabled ? "Dừng mô phỏng vật lý" : "Bắt đầu mô phỏng vật lý"
    )) GUITogglePhysicsSim(state);
    currentY += 35;

    if(GuiButton(
        (Rectangle){
            panelArea->x + MARGIN,
            currentY,
            ITEM_WIDTH,
            30
        },
        "Rung đỉnh"
    )) GUIJiggleVert(state);
    currentY += 35;

    GuiLabel(
        (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 20 },
        "Độ dài lò xo"
    );
    currentY += 20;

    GuiSlider(
        (Rectangle){
            panelArea->x + MARGIN,
            currentY,
            ITEM_WIDTH,
            30
        },
        NULL, NULL,
        &state->springLength,
        20.0,
        300.0
    );
    GuiLabel(
        (Rectangle){ panelArea->x + MARGIN * 2, currentY, ITEM_WIDTH - MARGIN, 30 },
        TextFormat("%.1fpx", state->springLength)
    );
    currentY += 35;

    GuiLabel(
        (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 20 },
        "Hệ số đàn hồi"
    );
    currentY += 20;

    GuiSlider(
        (Rectangle){
            panelArea->x + MARGIN,
            currentY,
            ITEM_WIDTH,
            30
        },
        NULL, NULL,
        &state->springStiffness,
        0.01,
        2.0
    );
    GuiLabel(
        (Rectangle){ panelArea->x + MARGIN * 2, currentY, ITEM_WIDTH - MARGIN, 30 },
        TextFormat("%.3f", state->springStiffness)
    );
    currentY += 35;

    GuiLabel(
        (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 20 },
        "Lực đẩy đỉnh"
    );
    currentY += 20;

    GuiSlider(
        (Rectangle){
            panelArea->x + MARGIN,
            currentY,
            ITEM_WIDTH,
            30
        },
        NULL, NULL,
        &state->coulombConstant,
        1000000.0,
        5000000.0
    );
    GuiLabel(
        (Rectangle){ panelArea->x + MARGIN * 2, currentY, ITEM_WIDTH - MARGIN, 30 },
        TextFormat("%.0f", state->coulombConstant)
    );
    currentY += 35;
}

static void GUIDrawVertexInfo(GUIState *state, Rectangle *panelArea, int *currentY) {
    Vertex *vert = state->selectedVertex;

    GuiLabel(
        (Rectangle){ panelArea->x + MARGIN, *currentY, ITEM_WIDTH, 20 },
        TextFormat("Đỉnh %u", vert->id)
    );
    *currentY += 23;

    GuiLabel(
        (Rectangle){ panelArea->x + MARGIN, *currentY, ITEM_WIDTH, 20 },
        "Đỉnh kề"
    );
    *currentY += 25;
    if(vert->adjacent_count) {
        for(unsigned i = 0; i < vert->adjacent_count; i++) {
            GuiLabel((Rectangle){ panelArea->x + MARGIN + 10, *currentY, ITEM_WIDTH, 20 }, 
                TextFormat(
                "- ID %u, w %u",
                vert->adjacents[i]->target->id,
                vert->adjacents[i]->weight
            ));
            *currentY += 23;
            if (*currentY > SCREEN_HEIGHT - 100) break;
        }
    } else {
        GuiLabel(
            (Rectangle){ panelArea->x + MARGIN, *currentY, ITEM_WIDTH, 20 },
            "- Không có đỉnh kề"
        );
        currentY += 23;
    }
}

static void GUIDrawVertexInspect(GUIState *state, Rectangle *panelArea) {
    int currentY = START_Y - 10;
    Vertex *sel_vert = state->selectedVertex;

    if(GuiButton(
        (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 30 },
        sel_vert != state->pathStartVertex ? "Chọn làm đỉnh bắt đầu" : "Bỏ chọn làm đỉnh bắt đầu"
    )) GUISetStartVert(state);
    currentY += 35;

    if(GuiButton(
        (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 30 },
        sel_vert != state->pathEndVertex ? "Chọn làm đỉnh kết thúc" : "Bỏ chọn làm đỉnh kết thúc"
    )) GUISetEndVert(state);
    currentY += 40;

    GUIDrawVertexInfo(state, panelArea, &currentY);

    if(GuiButton(
        (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 30 },
        "Bắt đầu cạnh mới"
    )) {
        state->edgeStartVertex = state->selectedVertex;
        state->current_mode = MODE_CREATE_EDGE;
    }
    currentY += 35;

    if(GuiButton(
        (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 30 },
        "Xóa đỉnh"
    )) GUIDeleteVert(state);
    currentY += 35;
}

static void GUIDrawEdgeInspect(GUIState *state, Rectangle *panelArea) {
    int currentY = START_Y - 10;
    Edge *edge = state->selectedEdge;

    GuiLabel(
        (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 20 },
        TextFormat("Cạnh %u->%u", edge->origin->id, edge->target->id)
    );
    currentY += 23;

    GuiLabel(
        (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 20 },
        "Trọng số"
    );
    currentY += 25;
    
    long long int pWeight = edge->weight;
    GuiValueBox(
        (Rectangle){
            panelArea->x + MARGIN,
            currentY,
            ITEM_WIDTH,
            30
        },
        NULL,
        (int*)&edge->weight,
        0, 20,
        true
    );
    if(pWeight != edge->weight) {
        state->shortestPathResult = -1;
    }
    currentY += 35;

    if(GuiButton(
        (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 30 },
        "Đảo chiều"
    )) GUIReverseEdge(state);
    currentY += 35;

    if(GuiButton(
        (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 30 },
        "Xóa cạnh"
    )) GUIDeleteEdge(state);
    currentY += 35;
}

static void GUIDrawCreateEdge(GUIState *state, Rectangle *panelArea) {
    int currentY = START_Y - 10;

    if(!state->edgeStartVertex) {
        GuiLabel(
            (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 20 },
            "Chọn đỉnh bắt đầu cạnh"
        );
        currentY += 23;

        if(state->selectedVertex) {
            GUIDrawVertexInfo(state, panelArea, &currentY);
        }
    } else {
        GuiLabel(
            (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 20 },
            "Chọn đỉnh kết thúc cạnh"
        );
        currentY += 23;
    }
}

void GUIInit(GUIState *state, const char *appName, const char *fontFile) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, appName);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    state->current_mode = MODE_NORMAL;

    state->graph = NULL;
    state->pathStartVertex = NULL;
    state->pathEndVertex = NULL;
    state->selectedVertex = NULL;
    state->draggingVertex = NULL;

    state->edgeStartVertex = NULL;

    state->physicsEnabled = true;

    state->camera = (Camera2D){ 0 };
    state->camera.target = Vector2Zero();
    state->camera.offset = (Vector2){
        (float)(GetScreenWidth() - PANEL_WIDTH) / 2,
        (float)(GetScreenHeight() - STATUS_BAR_HEIGHT) / 2
    };
    state->camera.rotation = 0.0f;
    state->camera.zoom = 1.0f;

    state->springLength = VERTEX_RADIUS * 3;
    state->springStiffness = 0.7;
    state->coulombConstant = 1200000;

    state->shortestPathResult = -1;

    int codepoints[1024];
    int count = 0;
    for (int i = 32; i <= 126; i++) codepoints[count++] = i;
    for (int i = 160; i <= 255; i++) codepoints[count++] = i;
    for (int i = 256; i <= 591; i++) codepoints[count++] = i;
    for (int i = 7680; i <= 7935; i++) codepoints[count++] = i;
    state->font = LoadFontEx(fontFile, 35, codepoints, count);
    SetTextureFilter(state->font.texture, TEXTURE_FILTER_BILINEAR);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

    snprintf(state->statusBar, sizeof(state->statusBar), "%s", "");
}

bool GUILoadGraph(GUIState *state, Graph *graph) {
    state->graph = graph;

    float boundsSize = state->springLength * graph->vertex_count / 3.0;
    for(unsigned i = 0; i < graph->vertex_count; i++) {
        Vertex *vert = graph->vertices[i];
        BodyInit(state->graph->vertices[i]);
        vert->position.x = (GetRandomValue(-500, 500) / 500.0) * boundsSize;
        vert->position.y = (GetRandomValue(-500, 500) / 500.0) * boundsSize;
    }
    snprintf(state->statusBar, sizeof(state->statusBar), "%s", "Đã tải đồ thị");

    return 1;
}

void GUIUnloadGraph(GUIState *state) {
    delete_graph(state->graph);

    state->graph = NULL;
    state->pathStartVertex = NULL;
    state->pathEndVertex = NULL;
    state->selectedVertex = NULL;
    state->selectedEdge = NULL;
    state->draggingVertex = NULL;

    snprintf(state->statusBar, sizeof(state->statusBar), "%s", "Đã gỡ đồ thị");
}

void GUIUpdate(GUIState *state) {
    if(IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        state->camera.target = Vector2Subtract(
            state->camera.target,
            Vector2Scale(GetMouseDelta(), 1.0f/state->camera.zoom)
        );
    }

    if(state->graph) {
        Vector2 mousePos = GetMousePosition();
        if(
            mousePos.x < GetScreenWidth() - PANEL_WIDTH
            && mousePos.y < GetScreenHeight() - STATUS_BAR_HEIGHT
        ) {
            Vector2 worldMouse = GetScreenToWorld2D(mousePos, state->camera);

            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                GUIGetObjectUnderCursor(state, worldMouse);
            } else if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                if(state->selectedVertex) {
                    Vector2 dir = Vector2Subtract(worldMouse, state->selectedVertex->position);
                    if(
                        IsMouseButtonDown(MOUSE_LEFT_BUTTON)
                        && Vector2LengthSqr(dir) <= VERTEX_RADIUS * VERTEX_RADIUS
                    ) {
                        state->draggingVertex = state->selectedVertex;
                    }
                }
            } else {
                state->draggingVertex = NULL;
            }

            if(state->draggingVertex) {
                state->draggingVertex->velocity = Vector2Zero();
                state->draggingVertex->position = worldMouse;
            }
        }
    }

    float mouse_wheel = GetMouseWheelMove();
    if(mouse_wheel > 0 && state->camera.zoom < 3) {
        state->camera.zoom *= 1.2;
    } else if (mouse_wheel < 0 && state->camera.zoom > 0.1) {
        state->camera.zoom /= 1.2;
    }

    GUIUpdatePhysics(state);

    // per mode update
    switch(state->current_mode) {
        case MODE_NORMAL:
            if(IsKeyPressed(KEY_ESCAPE)) {
                state->pathStartVertex = NULL;
                state->pathEndVertex = NULL;
                state->shortestPathResult = -1;
            }
            break;

        case MODE_VERTEX_INSPECT:
            if(IsKeyPressed(KEY_SPACE)) {
                if(!state->pathStartVertex && !state->pathEndVertex)
                    GUISetStartVert(state);
                else if(state->pathStartVertex && !state->pathEndVertex)
                    GUISetEndVert(state);
                else if(!state->pathStartVertex && state->pathEndVertex)
                    GUISetStartVert(state);
                else {
                    state->pathEndVertex = NULL;
                    GUISetStartVert(state);
                }
            }
            break;

        case MODE_CREATE_EDGE:
            if(IsKeyPressed(KEY_ESCAPE)) {
                state->edgeStartVertex = NULL;
                state->current_mode = MODE_NORMAL;
                break;
            }

            if(!state->edgeStartVertex) {
                if(state->selectedVertex) {
                    if(IsKeyPressed(KEY_SPACE)) {
                        state->edgeStartVertex = state->selectedVertex;
                        state->selectedVertex = NULL;
                    }
                }
            } else {
                if(state->selectedVertex && IsKeyPressed(KEY_SPACE)) {
                    Vertex *endVert = state->selectedVertex;
                    state->selectedVertex = NULL;

                    Edge *edge = make_edge(state->edgeStartVertex, endVert, 0);
                    if(!edge) {
                        snprintf(
                            state->statusBar,
                            sizeof(state->statusBar),
                            "%s",
                            "Đã đạt số lượng cạnh tối đa"
                        );
                    } else {
                        state->selectedVertex = NULL;
                        state->selectedEdge = edge;
                        snprintf(
                            state->statusBar,
                            sizeof(state->statusBar),
                            "%s",
                            TextFormat(
                                "Đã thêm cạnh %d -> %d",
                                state->edgeStartVertex->id,
                                endVert->id
                            )
                        );
                    }

                    state->edgeStartVertex = NULL;
                    state->current_mode = MODE_EDGE_INSPECT;
                }
            }

            break;

        default:
            break;
    }
    
    if(state->current_mode != MODE_CREATE_EDGE) {
        if(!state->selectedVertex && !state->selectedEdge) {
            state->current_mode = MODE_NORMAL;
        } else if(state->selectedVertex) {
            state->current_mode = MODE_VERTEX_INSPECT;
        } else {
            state->current_mode = MODE_EDGE_INSPECT;
        }
    }
}

void GUIDraw(GUIState *state) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode2D(state->camera);
    GUIDrawOffsettedGrid(state->camera, 100);
    if(state->graph)
        GUIDrawGraph(state);
    EndMode2D();

    GuiSetFont(state->font);

    Rectangle panelArea = (Rectangle){
        GetScreenWidth() - PANEL_WIDTH,
        0,
        GetScreenWidth(),
        GetScreenHeight() - STATUS_BAR_HEIGHT + 1
    };
    GuiPanel(panelArea, "Điều khiển");

    switch(state->current_mode) {
        case MODE_CREATE_EDGE:
            GUIDrawCreateEdge(state, &panelArea);
            break;

        case MODE_VERTEX_INSPECT:
            GUIDrawVertexInspect(state, &panelArea);
            break;

        case MODE_EDGE_INSPECT:
            GUIDrawEdgeInspect(state, &panelArea);
            break;

        default:
        case MODE_NORMAL:
            GUIDrawNormalView(state, &panelArea);
            break;
    }

    GuiStatusBar(
        (Rectangle){
            0,
            GetScreenHeight() - STATUS_BAR_HEIGHT,
            GetScreenWidth(),
            STATUS_BAR_HEIGHT
        },
        state->statusBar
    );

    EndDrawing();
}
