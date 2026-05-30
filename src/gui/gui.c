#include <raylib/raylib.h>

#define RAYGUI_IMPLEMENTATION
#include <raylib/raygui.h>

#undef RAYGUI_IMPLEMENTATION
#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include <gui.h>

#include <graph.h>
#include <constants.h>
#include <string.h>

#include <io.h>

static void DrawOffsettedGrid(Camera2D camera, float spacing) {
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

static void GetObjectUnderCursor(GUIState *state, Vector2 worldMouse) {
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

static void UpdatePhysics(GUIState *state) {
    if(!state->physicsEnabled || !state->graph)
        return;

    Graph *graph = state->graph;
    float deltaTime = Clamp(GetFrameTime(), 0, 0.05);

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

        float length = Vector2Length(vert->velocity);
        if(length > state->currentTemperature) {
            vert->velocity = Vector2Scale(vert->velocity, state->currentTemperature / length);
        }

        vert->velocity = Vector2Clamp(
            vert->velocity,
            (Vector2){ -VELOCITY_CAP, -VELOCITY_CAP },
            (Vector2){ VELOCITY_CAP, VELOCITY_CAP }
        );
        Inertia(vert, deltaTime);
    }

    state->currentTemperature *= TEMPERATURE_DECAY;
}

void GUIInit(GUIState *state, const char *appName, const char *fontFile) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, appName);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    state->fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
    state->fileDialogState.windowActive = false;

    state->aboutPage = true;
    state->current_mode = MODE_NORMAL;

    state->graph = NULL;
    state->pathStartVertex = NULL;
    state->pathEndVertex = NULL;
    state->selectedVertex = NULL;
    state->draggingVertex = NULL;

    state->edgeStartVertex = NULL;

    state->camera = (Camera2D){ 0 };
    state->camera.target = Vector2Zero();
    state->camera.offset = (Vector2){
        (float)(GetScreenWidth() - PANEL_WIDTH) / 2,
        (float)(GetScreenHeight() - STATUS_BAR_HEIGHT) / 2
    };
    state->camera.rotation = 0.0f;
    state->camera.zoom = 1.0f;

    state->physicsEnabled = true;
    state->currentTemperature = INITIAL_TEMPERATURE;
    state->springLength = VERTEX_RADIUS * 3;
    state->springStiffness = 1.5;
    state->coulombConstant = 1200000;

    state->shortestPathResult = NO_PATH;

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

void GUIUpdate(GUIState *state) {
    if(state->fileDialogState.SelectFilePressed) {
        if(state->fileDialogState.saveFileMode) {
            const char *ext = GetFileExtension(state->fileDialogState.fileNameText);
            if(!ext) {
                strcat(state->fileDialogState.fileNameText, ".txt");
            }
        }

        if(!IsFileExtension(state->fileDialogState.fileNameText, ".txt")) {
            snprintf(
                state->statusBar,
                sizeof(state->statusBar),
                "%s",
                "Tên file phải có đuôi .txt"
            );
        } else {
            char buff[512];
            strcpy(buff, TextFormat("%s" PATH_SEPERATOR "%s", state->fileDialogState.dirPathText, state->fileDialogState.fileNameText));
            int err;
            bool saving = state->fileDialogState.saveFileMode;
            if(!saving) {
                Graph *graph = load_graph_from_file(buff, &err);
                if(err == ERR_NONE) {
                    GUILoadGraph(state, graph);
                    snprintf(
                        state->statusBar,
                        sizeof(state->statusBar),
                        "%s \"%s\"",
                        "Đã tải file",
                        buff
                    );
                }
            } else {
                save_graph_as_file(state->graph, buff, &err);
                if(err == ERR_NONE)
                    snprintf(
                        state->statusBar,
                        sizeof(state->statusBar),
                        "%s \"%s\"",
                        "Lưu thành công đồ thị vào file",
                        buff
                    );
            }
            switch(err) {
                case ERR_NONE:
                    break;
                case ERR_FILE_OPEN:
                    snprintf(
                        state->statusBar,
                        sizeof(state->statusBar),
                        "%s",
                        (!saving) ? "Không thể tải đồ thị, file không tồn tại" : "Không thể mở file để lưu"
                    );
                    state->fileDialogState.windowActive = true;
                    break;
                case ERR_INP_FORMAT:
                    snprintf(
                        state->statusBar,
                        sizeof(state->statusBar),
                        "%s",
                        "Không thể tải đồ thị, định dạng file không hợp lệ"
                    );
                    break;
                case ERR_MEMORY:
                    snprintf(
                        state->statusBar,
                        sizeof(state->statusBar),
                        "%s",
                        "Không thể tải đồ thị, không đủ bộ nhớ"
                    );
                    break;
                case ERR_INVALID_GRAPH:
                    snprintf(
                        state->statusBar,
                        sizeof(state->statusBar),
                        "%s",
                        (!saving) ? "Không thể tải đồ thị, đồ thị trống" : "Không thể lưu đồ thị, đồ thị trống"
                    );
                    break;
            }
        }

        state->fileDialogState.SelectFilePressed = false;
    }

    if(state->fileDialogState.windowActive)
        return;

    if(IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) && !GuiIsLocked()) {
        state->camera.target = Vector2Subtract(
            state->camera.target,
            Vector2Scale(GetMouseDelta(), 1.0f/state->camera.zoom)
        );
    }

    if(state->graph && !GuiIsLocked()) {
        Vector2 mousePos = GetMousePosition();
        if(
            mousePos.x < GetScreenWidth() - PANEL_WIDTH
            && mousePos.y < GetScreenHeight() - STATUS_BAR_HEIGHT
        ) {
            Vector2 worldMouse = GetScreenToWorld2D(mousePos, state->camera);

            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                GetObjectUnderCursor(state, worldMouse);
            } else if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                if(state->selectedVertex) {
                    Vector2 dir = Vector2Subtract(worldMouse, state->selectedVertex->position);
                    if(
                        IsMouseButtonDown(MOUSE_LEFT_BUTTON)
                        && Vector2LengthSqr(dir) <= VERTEX_RADIUS * VERTEX_RADIUS
                    ) {
                        state->draggingVertex = state->selectedVertex;
                        state->currentTemperature = INITIAL_TEMPERATURE;
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
    if(mouse_wheel != 0 && !GuiIsLocked()) {
        Vector2 prevMousePos = GetScreenToWorld2D(GetMousePosition(), state->camera);

        if(mouse_wheel > 0 && state->camera.zoom < 3) {
            state->camera.zoom *= 1.2;
        } else if (mouse_wheel < 0 && state->camera.zoom > 0.1) {
            state->camera.zoom /= 1.2;
        }

        Vector2 newMousePos = GetScreenToWorld2D(GetMousePosition(), state->camera);
        state->camera.target = Vector2Add(
            state->camera.target,
            Vector2Subtract(prevMousePos, newMousePos)
        );
    }

    UpdatePhysics(state);

    if(GuiIsLocked())
        return;

    // per mode update
    switch(state->current_mode) {
        case MODE_NORMAL:
            if(IsKeyPressed(KEY_BACKSPACE) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                state->pathStartVertex = NULL;
                state->pathEndVertex = NULL;
                GUIFindShortestPath(state);
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
                    state->shortestPathResult = NO_PATH;
                }
            }
            break;

        case MODE_CREATE_EDGE:
            if(IsKeyPressed(KEY_BACKSPACE) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
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
                if(state->selectedVertex == state->edgeStartVertex) {
                    state->selectedVertex = NULL;
                }
                if(state->selectedVertex && IsKeyPressed(KEY_SPACE)) {
                    Vertex *endVert = state->selectedVertex;
                    state->selectedVertex = NULL;
                    GUIAddEdge(state, state->edgeStartVertex, endVert);
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
    DrawOffsettedGrid(state->camera, 100);
    if(state->graph)
        GUIDrawGraph(state);
    EndMode2D();

    GuiSetFont(state->font);

    if(state->fileDialogState.windowActive) {
        GuiLock();
    }

    Rectangle panelArea = (Rectangle){
        GetScreenWidth() - PANEL_WIDTH,
        0,
        GetScreenWidth(),
        GetScreenHeight() - STATUS_BAR_HEIGHT + 1
    };
    GuiPanel(panelArea, "Điều khiển");

    const char *contextHelper = "";
    switch(state->current_mode) {
        case MODE_CREATE_EDGE:
            if(!state->edgeStartVertex) {
                contextHelper = "[Chuột trái]: Chọn đỉnh bắt đầu   |  [Space]: Xác nhận  |  [Esc]: Hủy";
            } else {
                contextHelper = "[Chuột trái]: Chọn đỉnh kết thúc  |  [Space]: Xác nhận  |  [Esc]: Hủy";
            }
            GUIDrawCreateEdge(state, &panelArea);
            break;

        case MODE_VERTEX_INSPECT:
            contextHelper = "[Space]: Đặt đỉnh Bắt đầu/Kết thúc  |  [Chuột trái]: Kéo thả đỉnh";
            GUIDrawVertexInspect(state, &panelArea);
            break;

        case MODE_EDGE_INSPECT:
            contextHelper = "[Chuột giữa]: Kéo thả Camera  |  [Lăn chuột]: Thu phóng";
            GUIDrawEdgeInspect(state, &panelArea);
            break;

        default:
        case MODE_NORMAL:
            contextHelper = "[Chuột giữa]: Kéo thả Camera  |  [Lăn chuột]: Thu phóng  |  [Chuột trái]: Chọn";
            GUIDrawNormalView(state, &panelArea);
            break;
    }

    Vector2 helperTextSize = MeasureTextEx(state->font, contextHelper, 20, 1);
    DrawTextEx(
        state->font, 
        contextHelper, 
        (Vector2){ 10, GetScreenHeight() - STATUS_BAR_HEIGHT - helperTextSize.y - 10 }, 
        20, 
        1, 
        GRAY
    );

    GuiStatusBar(
        (Rectangle){
            0,
            GetScreenHeight() - STATUS_BAR_HEIGHT,
            GetScreenWidth(),
            STATUS_BAR_HEIGHT
        },
        state->statusBar
    );

    GUIDrawPathPage(state);
    GUIDrawAboutPage(state);

    if(state->fileDialogState.windowActive) {
        GuiUnlock();
        GuiWindowFileDialog(&state->fileDialogState);
    }

    EndDrawing();
}
