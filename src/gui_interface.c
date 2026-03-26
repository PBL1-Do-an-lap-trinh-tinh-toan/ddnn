#include <raylib/raylib.h>
#include <gui_interface.h>

#define RAYGUI_STATIC
#define RAYGUI_IMPLEMENTATION
#include <raylib/raygui.h>

#include <graph.h>
#include <constants.h>
#include <string.h>

static void GuiOffsettedGrid(Camera2D camera, float spacing) {
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

static void DrawVertex(Vertex *vert, Color color, Font font) {
    DrawCircleV(*vert->position, VERTEX_RADIUS, GRAY);
    DrawCircleV(*vert->position, VERTEX_RADIUS - 3, color);
    const char *text = TextFormat("%d", vert->id);
    Vector2 textOrigin = MeasureTextEx(font, text, 25, 2);
    textOrigin.x /= -2.0;
    textOrigin.y /= -2.0;
    textOrigin = Vector2Add(textOrigin, *vert->position);
    DrawTextEx(font, TextFormat("%d", vert->id), textOrigin, 25, 2, BLACK);
}

static void DrawEdge(Vertex *start_vert, Edge *edge, Color color, Font font) {
    DrawLineEx(
        *start_vert->position,
        *edge->target->position,
        EDGE_WIDTH,
        color
    );

    Vector2 midPoint = Vector2Add(*start_vert->position, *edge->target->position);
    midPoint.x /= 2.0;
    midPoint.y /= 2.0;

    Vector2 dir = Vector2Subtract(*edge->target->position, *start_vert->position);

    Vector2 dirNormalized = Vector2Normalize(dir);
    Vector2 normal = dirNormalized;
    float t = normal.x;
    normal.x = -normal.y * 5;
    normal.y = t * 5;

    Vector2 arrowTarget = Vector2Subtract(
        *edge->target->position,
        (Vector2){ dirNormalized.x * VERTEX_RADIUS, dirNormalized.y * VERTEX_RADIUS }
    );
    Vector2 arrowOrigin = Vector2Subtract(
        arrowTarget,
        (Vector2){ dirNormalized.x * 25, dirNormalized.y * 25 }
    );

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
        GRAY
    );
}

static void DrawGraph(GUIState *state) {
    Graph *graph = state->graph;
    for(unsigned i = 0; i < graph->vertex_count; i++) {
        Vertex *vert = graph->vertices[i];
        for(unsigned j = 0; j < vert->adjacent_count; j++)
            DrawEdge(vert, vert->adjacents[j], GRAY, state->font);
    }

    for(unsigned i = 0; i < graph->vertex_count; i++) {
        Vertex *vert = graph->vertices[i];
        DrawVertex(vert, RAYWHITE, state->font);
    }

    if(state->startVertex)
        DrawVertex(state->startVertex, BLUE, state->font);

    if(state->endVertex)
        DrawVertex(state->endVertex, BLUE, state->font);
}

static Graph *RandomGraph() {
    unsigned vertexCount = GetRandomValue(4, 20);
    Graph *graph = make_graph(vertexCount);
    for(unsigned i = 0; i < vertexCount; i++) {
        add_vertex(graph);
    }

    for(unsigned i = 1; i < vertexCount; i++) {
        unsigned randomPrevious = GetRandomValue(0, i - 1);
        make_edge(
            graph->vertices[i], 
            graph->vertices[randomPrevious], 
            GetRandomValue(1, 20)
        );
    }

    unsigned extraEdges = GetRandomValue(1, vertexCount / 2); 
    for(unsigned i = 0; i < extraEdges; i++) {
        unsigned u = GetRandomValue(0, vertexCount - 1);
        unsigned v = GetRandomValue(0, vertexCount - 1);

        if(u != v) {
            Edge *edge_uv = find_edge(graph->vertices[u], graph->vertices[v]);
            Edge *edge_vu = find_edge(graph->vertices[v], graph->vertices[u]);

            if(!edge_uv) {
                // swap
                unsigned t = u;
                u = v;
                v = t;

                Edge *t_edge = edge_uv;
                edge_uv = edge_vu;
                edge_vu = t_edge;
            }

            if(!edge_uv) {
                if(!edge_vu) {
                    int weight = GetRandomValue(1, 20);
                    make_edge(graph->vertices[u], graph->vertices[v], weight);
                    if(GetRandomValue(0, 100) < 10)
                        make_edge(graph->vertices[v], graph->vertices[u], weight);
                } else {
                    make_edge(graph->vertices[u], graph->vertices[v], edge_vu->weight);
                }
            }
        }
    }

    return graph;
}

void GUIInit(GUIState *state, const char *appName, const char *fontFile) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, appName);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    state->graph = NULL;
    state->startVertex = NULL;
    state->endVertex = NULL;
    state->selectedVertex = NULL;
    state->draggingVertex = NULL;

    state->bodies = NULL;
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

    int codepoints[1024];
    int count = 0;
    for (int i = 32; i <= 126; i++) codepoints[count++] = i;
    for (int i = 160; i <= 255; i++) codepoints[count++] = i;
    for (int i = 256; i <= 591; i++) codepoints[count++] = i;
    for (int i = 7680; i <= 7935; i++) codepoints[count++] = i;
    state->font = LoadFontEx(fontFile, 35, codepoints, count);
    SetTextureFilter(state->font.texture, TEXTURE_FILTER_BILINEAR);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

    state->currentTab = TAB_FILES;
    strcpy(state->statusBar, "");
}

bool GUILoadGraph(GUIState *state, Graph *graph) {
    state->graph = graph;
    state->bodies = (Body*)malloc(sizeof(Body) * graph->max_vertex_count);
    if(!state->bodies) {
        strcpy(state->statusBar, "Tải đồ thị không thành công");
        return 0;
    }

    memset(state->bodies, 0, sizeof(Body) * graph->max_vertex_count);

    float boundsSize = state->springLength * graph->vertex_count / 3.0;
    for(unsigned i = 0; i < graph->vertex_count; i++) {
        Vertex *vert = graph->vertices[i];
        BodyInit(&state->bodies[i], &vert->position);
        vert->position->x = (GetRandomValue(-500, 500) / 500.0) * boundsSize;
        vert->position->y = (GetRandomValue(-500, 500) / 500.0) * boundsSize;
    }
    strcpy(state->statusBar, "Đã tải đồ thị");

    return 1;
}

void GUIUnloadGraph(GUIState *state) {
    delete_graph(state->graph);
    free(state->bodies);

    state->graph = NULL;
    state->startVertex = NULL;
    state->endVertex = NULL;
    state->selectedVertex = NULL;
    state->selectedEdge = NULL;
    state->draggingVertex = NULL;

    state->bodies = NULL;

    strcpy(state->statusBar, "Đã gỡ đồ thị");
}

void GUIUpdate(GUIState *state) {
    if(!state->graph || !state->bodies) return;

    if(IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        state->camera.target = Vector2Subtract(
            state->camera.target,
            Vector2Divide(GetMouseDelta(), (Vector2){ state->camera.zoom, state->camera.zoom })
        );
    }

    Vector2 mousePos = GetMousePosition();
    if(
        mousePos.x < GetScreenWidth() - PANEL_WIDTH
        && mousePos.y < GetScreenHeight() - STATUS_BAR_HEIGHT
    ) {
        Vector2 worldMouse = GetScreenToWorld2D(mousePos, state->camera);

        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            state->selectedVertex = NULL;
            state->selectedEdge = NULL;

            // check edges
            float minDistSqr = EDGE_WIDTH * EDGE_WIDTH * 9;
            for(unsigned i = 0; i < state->graph->vertex_count; i++) {
                Vertex *vert = state->graph->vertices[i];
                for(unsigned j = 0; j < vert->adjacent_count; j++) {
                    Edge *edge = vert->adjacents[j];
                    Vector2 edge_dir = Vector2Subtract(*edge->target->position, *vert->position);
                    float edgeLenSqr = Vector2LengthSqr(edge_dir);
                    if(edgeLenSqr < 40.0 * 40.0) continue;

                    Vector2 event_dir = Vector2Subtract(worldMouse, *vert->position);
                    float t = Vector2DotProduct(event_dir, edge_dir) / edgeLenSqr;
                    t = Clamp(t, 0.0, 1.0);
                    Vector2 proj = Vector2Add(*vert->position, Vector2Scale(edge_dir, t));
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
                Vector2 dir = Vector2Subtract(worldMouse, *vert->position);
                if(Vector2LengthSqr(dir) <= VERTEX_RADIUS * VERTEX_RADIUS) {
                    state->selectedVertex = vert;
                    state->selectedEdge = NULL;
                    break;
                }
            }
        } else if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            if(state->selectedVertex) {
                Vector2 dir = Vector2Subtract(worldMouse, *state->selectedVertex->position);
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
            state->bodies[state->draggingVertex->id].velocity = Vector2Zero();
            *state->draggingVertex->position = worldMouse;
        }
    }

    float mouse_wheel = GetMouseWheelMove();
    if(mouse_wheel > 0 && state->camera.zoom < 3) {
        state->camera.zoom *= 1.2;
    } else if (mouse_wheel < 0 && state->camera.zoom > 0.1) {
        state->camera.zoom /= 1.2;
    }

    if(state->physsimRequest) {
        state->physicsEnabled = !state->physicsEnabled;
        if(!state->physicsEnabled) {
            strcpy(state->statusBar, "Đã tắt mô phỏng vật lý");
        } else {
            strcpy(state->statusBar, "Đã bật mô phỏng vật lý");
        }
    }

    if(state->jiggleRequest) {
        SetRandomSeed((unsigned)GetTime());
        for(unsigned i = 0; i < state->graph->vertex_count; i++) {
            float theta = GetRandomValue(0, 360) * DEG2RAD;
            Vector2 dir = Vector2Scale((Vector2){cos(theta), sin(theta)}, VELOCITY_CAP);
            state->bodies[i].velocity.x += dir.x;
            state->bodies[i].velocity.y += dir.y;
        }
    }

    Vertex *vert = state->selectedVertex;

    if(state->startVertSetRequest) {
        if(state->startVertex != vert) {
            state->startVertex = vert;
            if(state->endVertex == vert)
                state->endVertex = NULL;
            strcpy(state->statusBar, TextFormat("Đã chọn đỉnh %d làm đỉnh bắt đầu", vert->id));
        } else {
            state->startVertex = NULL;
            strcpy(state->statusBar, TextFormat("Đã bỏ chọn đỉnh %d làm đỉnh bắt đầu", vert->id));
        }
    }

    if(state->endVertSetRequest) {
        if(state->endVertex != vert) {
            state->endVertex = vert;
            if(state->startVertex == vert)
                state->startVertex = NULL;
            strcpy(state->statusBar, TextFormat("Đã chọn đỉnh %d làm đỉnh kết thúc", vert->id));
        } else {
            state->endVertex = NULL;
            strcpy(state->statusBar, TextFormat("Đã bỏ chọn đỉnh %d làm đỉnh kết thúc", vert->id));
        }
    }

    if(state->deleteEdgeRequest) {
        unsigned from_id = state->selectedEdge->origin->id;
        unsigned to_id = state->selectedEdge->target->id;
        remove_edge(state->selectedEdge);
        state->selectedEdge = NULL;
        state->deleteEdgeRequest = false;
        strcpy(state->statusBar, TextFormat("Đã xóa cạnh %u -> %u", from_id, to_id));
    }

    if(state->deleteVertexRequest) {
        // delete physics body
        // body idx is also vertex idx
        for(unsigned i = vert->idx + 1; i < state->graph->vertex_count; i++) {
            state->bodies[i - 1] = state->bodies[i];
            state->graph->vertices[i]->position = &state->bodies[i - 1].position;
        }

        // delete graph vertex
        remove_vertex(state->graph, vert->id);
        state->selectedVertex = NULL;
        vert = NULL;
        state->deleteVertexRequest = false;
    }

    if(state->physicsEnabled) {
        Graph *graph = state->graph;
        Body *bodies = state->bodies;
        float deltaTime = Clamp(GetFrameTime(), 0, 0.5);

        for(unsigned i = 0; i < graph->vertex_count; i++) {
            Vertex *vert1 = graph->vertices[i];

            // spring force
            for(unsigned j = 0; j < vert1->adjacent_count; j++) {
                unsigned id1 = i;
                unsigned id2 = vert1->adjacents[j]->target->idx;

                Vector2 p1 = bodies[id1].position;
                Vector2 p2 = bodies[id2].position;
                Vector2 dir = Vector2Subtract(p1, p2);

                float forceMag = state->springStiffness * (state->springLength - Vector2Length(dir));
                Vector2 force = Vector2Scale(Vector2Normalize(dir), forceMag);

                ApplyForce(&bodies[id1], force, deltaTime);
                ApplyForce(&bodies[id2], Vector2Scale(force, -1.0f), deltaTime);
            }

            // coulomb force
            for(unsigned j = i + 1; j < graph->vertex_count; j++) {
                Vector2 p1 = bodies[i].position;
                Vector2 p2 = bodies[j].position;
                Vector2 dir = Vector2Subtract(p1, p2);

                float forceMag = state->coulombConstant / Vector2LengthSqr(dir);
                Vector2 force = Vector2Scale(Vector2Normalize(dir), forceMag);

                ApplyForce(&bodies[i], force, deltaTime);
                ApplyForce(&bodies[j], Vector2Scale(force, -1.0f), deltaTime);
            }

            // avoid drifting
            Vector2 anchorForce = Vector2Scale(*vert1->position, -ANCHOR_FORCE_MAG);
            ApplyForce(&bodies[i], anchorForce, deltaTime);
        }

        for(unsigned i = 0; i < graph->vertex_count; i++) {
            if(isnan(bodies[i].velocity.x) || isnan(bodies[i].velocity.y)) {
                bodies[i].velocity.x = 0;
                bodies[i].velocity.y = 0;
            }

            bodies[i].velocity.x *= VELOCITY_DAMPENING;
            bodies[i].velocity.y *= VELOCITY_DAMPENING;

            bodies[i].velocity = Vector2Clamp(
                bodies[i].velocity,
                (Vector2){ -VELOCITY_CAP, -VELOCITY_CAP },
                (Vector2){ VELOCITY_CAP, VELOCITY_CAP }
            );
            Inertia(&bodies[i], deltaTime);
        }
    }
}

void GUIDraw(GUIState *state) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode2D(state->camera);
    GuiOffsettedGrid(state->camera, 100);
    if(state->graph)
        DrawGraph(state);
    EndMode2D();

    GuiSetFont(state->font);

    Rectangle panelArea = (Rectangle){
        GetScreenWidth() - PANEL_WIDTH,
        0,
        GetScreenWidth(),
        GetScreenHeight() - STATUS_BAR_HEIGHT + 1
    };
    GuiPanel(panelArea, "Điều khiển");

    float currentY = 39;
    float margin = 5;
    float itemWidth = PANEL_WIDTH - (margin * 2);

    if(!state->selectedVertex && !state->selectedEdge) {
        GuiLine(
            (Rectangle){
                panelArea.x + margin,
                currentY,
                itemWidth,
                3
            },
            "Đồ thị"
        );
        currentY += 23;

        if(state->graph) GuiDisable();
        GuiButton(
            (Rectangle){
                panelArea.x + margin,
                currentY,
                (itemWidth - 5) / 2,
                30
            },
            "Tải đồ thị"
        );
        GuiEnable();
        if(!state->graph) GuiDisable();
        if(GuiButton(
            (Rectangle){ panelArea.x + (
                itemWidth - 5
            ) / 2 + margin * 2, currentY, (itemWidth - 5) / 2, 30 },
            "Gỡ đồ thị"
        )) {
            GUIUnloadGraph(state);
        }
        GuiEnable();
        currentY += 35;

        if(state->graph) GuiDisable();
        if(GuiButton(
            (Rectangle){ panelArea.x + margin, currentY, itemWidth, 30 },
            "Tạo đồ thị ngẫu nhiên"
        )) {
            SetRandomSeed((unsigned)GetTime());
            GUILoadGraph(state, RandomGraph());
        }
        GuiEnable();
        currentY += 35;

        currentY += 15;
        GuiLine(
            (Rectangle){
                panelArea.x + margin,
                currentY,
                itemWidth,
                3
            },
            "Mô phỏng vật lý"
        );
        currentY += 23;

        state->physsimRequest = GuiButton(
            (Rectangle){
                panelArea.x + margin,
                currentY,
                itemWidth,
                30
            },
            state->physicsEnabled ? "Dừng mô phỏng vật lý" : "Bắt đầu mô phỏng vật lý"
        );
        currentY += 35;

        state->jiggleRequest = GuiButton(
            (Rectangle){
                panelArea.x + margin,
                currentY,
                itemWidth,
                30
            },
            "Rung đỉnh"
        );
        currentY += 35;

        GuiLabel(
            (Rectangle){ panelArea.x + margin, currentY, itemWidth, 20 },
            "Độ dài lò xo"
        );
        currentY += 20;

        GuiSlider(
            (Rectangle){
                panelArea.x + margin,
                currentY,
                itemWidth,
                30
            },
            NULL, NULL,
            &state->springLength,
            20.0,
            300.0
        );
        GuiLabel(
            (Rectangle){ panelArea.x + margin * 2, currentY, itemWidth - margin, 30 },
            TextFormat("%.1fpx", state->springLength)
        );
        currentY += 35;

        GuiLabel(
            (Rectangle){ panelArea.x + margin, currentY, itemWidth, 20 },
            "Hệ số đàn hồi"
        );
        currentY += 20;

        GuiSlider(
            (Rectangle){
                panelArea.x + margin,
                currentY,
                itemWidth,
                30
            },
            NULL, NULL,
            &state->springStiffness,
            0.01,
            2.0
        );
        GuiLabel(
            (Rectangle){ panelArea.x + margin * 2, currentY, itemWidth - margin, 30 },
            TextFormat("%.3f", state->springStiffness)
        );
        currentY += 35;

        GuiLabel(
            (Rectangle){ panelArea.x + margin, currentY, itemWidth, 20 },
            "Lực đẩy đỉnh"
        );
        currentY += 20;

        GuiSlider(
            (Rectangle){
                panelArea.x + margin,
                currentY,
                itemWidth,
                30
            },
            NULL, NULL,
            &state->coulombConstant,
            1000000.0,
            5000000.0
        );
        GuiLabel(
            (Rectangle){ panelArea.x + margin * 2, currentY, itemWidth - margin, 30 },
            TextFormat("%.0f", state->coulombConstant)
        );
        currentY += 35;

        // if(state->startVertex && state->endVertex) {
        //
        // }
    } else if(state->selectedVertex) {
        currentY -= 10;
        Vertex *vert = state->selectedVertex;

        GuiLabel(
            (Rectangle){ panelArea.x + margin, currentY, itemWidth, 20 },
            TextFormat("ID %u", vert->id)
        );
        currentY += 23;

        state->startVertSetRequest = GuiButton(
            (Rectangle){ panelArea.x + margin, currentY, itemWidth, 30 },
            vert != state->startVertex ? "Chọn làm đỉnh bắt đầu" : "Bỏ chọn làm đỉnh bắt đầu"
        );
        currentY += 35;

        state->endVertSetRequest = GuiButton(
            (Rectangle){ panelArea.x + margin, currentY, itemWidth, 30 },
            vert != state->endVertex ? "Chọn làm đỉnh kết thúc" : "Bỏ chọn làm đỉnh kết thúc"
        );
        currentY += 40;

        GuiLabel(
            (Rectangle){ panelArea.x + margin, currentY, itemWidth, 20 },
            "Đỉnh kề"
        );
        currentY += 25;
        if(vert->adjacent_count) {
            for(unsigned i = 0; i < vert->adjacent_count; i++) {
                GuiLabel((Rectangle){ panelArea.x + margin + 10, currentY, itemWidth, 20 }, 
                         TextFormat(
                             "- ID %u, w %u",
                             vert->adjacents[i]->target->id,
                             vert->adjacents[i]->weight
                         ));
                currentY += 23;
                if (currentY > SCREEN_HEIGHT - 100) break;
            }
        } else {
            GuiLabel(
                (Rectangle){ panelArea.x + margin, currentY, itemWidth, 20 },
                "- Không có đỉnh kề"
            );
            currentY += 23;
        }

        state->deleteVertexRequest = GuiButton(
            (Rectangle){ panelArea.x + margin, currentY, itemWidth, 30 },
            "Xóa đỉnh"
        );
        currentY += 35;
    } else {
        currentY -= 10;
        Edge *edge = state->selectedEdge;

        GuiLabel(
            (Rectangle){ panelArea.x + margin, currentY, itemWidth, 20 },
            TextFormat("Cạnh %u->%u", edge->origin->id, edge->target->id)
        );
        currentY += 23;

        GuiLabel(
            (Rectangle){ panelArea.x + margin, currentY, itemWidth, 20 },
            "Trọng số"
        );
        currentY += 25;
        
        GuiValueBox(
            (Rectangle){
                panelArea.x + margin,
                currentY,
                itemWidth,
                30
            },
            NULL,
            (int*)&edge->weight,
            0, 20,
            true
        );
        currentY += 35;

        state->deleteEdgeRequest = GuiButton(
            (Rectangle){ panelArea.x + margin, currentY, itemWidth, 30 },
            "Xóa cạnh"
        );
        currentY += 35;
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
