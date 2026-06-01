#define RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT 24
#include <raylib/raygui.h>
#include <gui.h>

#include <graph.h>
#include <stdio.h>
#include <string.h>
#include <constants.h>


static void TogglePhysicsSim(GUIState *state) {
    if(!state->graph) return;

    state->physicsEnabled = !state->physicsEnabled;
    if(!state->physicsEnabled) {
        snprintf(state->statusBar, sizeof(state->statusBar), "%s", "Đã tắt mô phỏng vật lý");
    } else {
        state->currentTemperature = INITIAL_TEMPERATURE;
        snprintf(state->statusBar, sizeof(state->statusBar), "%s", "Đã bật mô phỏng vật lý");
    }
}

static void ShuffleVert(GUIState *state) {
    if(!state->graph) return;
    
    SetRandomSeed((unsigned)GetTime());
    float boundsSize = state->springLength * state->graph->vertex_count / 3.0;
    for(unsigned i = 0; i < state->graph->vertex_count; i++) {
        float theta = GetRandomValue(0, 360) * DEG2RAD;
        Vector2 dir = Vector2Scale((Vector2){cos(theta), sin(theta)}, VELOCITY_CAP);
        Vertex *vert = state->graph->vertices[i];
        vert->velocity.x += dir.x;
        vert->velocity.y += dir.y;
        vert->position.x = (GetRandomValue(-500, 500) / 500.0) * boundsSize;
        vert->position.y = (GetRandomValue(-500, 500) / 500.0) * boundsSize;
    }

    state->currentTemperature = INITIAL_TEMPERATURE;
}

void GUIDrawNormalView(GUIState *state, Rectangle *panelArea) {
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

    if(state->graph || state->fileDialogState.windowActive) GuiDisable();
    if(GuiButton(
        (Rectangle){
            panelArea->x + MARGIN,
            currentY,
            (ITEM_WIDTH - 5) / 2.0,
            30
        },
        "Tải đồ thị"
    )) {
        state->fileDialogState.saveFileMode = false;
        state->fileDialogState.windowActive = true;
    }
    GuiEnable();

    if(!state->graph || state->fileDialogState.windowActive) GuiDisable();
    if(GuiButton(
        (Rectangle){
            panelArea->x + (ITEM_WIDTH - 5) / 2.0 + MARGIN * 2,
            currentY,
            (ITEM_WIDTH - 5) / 2.0,
            30
        },
        "Lưu đồ thị"
    )) {
        state->fileDialogState.saveFileMode = true;
        state->fileDialogState.windowActive = true;
    }
    GuiEnable();
    currentY += 35;

    if(!state->graph) GuiDisable();
    if(GuiButton(
        (Rectangle){
            panelArea->x + MARGIN,
            currentY,
            ITEM_WIDTH,
            30
        },
        "Gỡ đồ thị"
    )) {
        GUIUnloadGraph(state);
    }
    GuiEnable();
    currentY += 35;

    if(GuiButton((Rectangle){
        panelArea->x + MARGIN,
        currentY,
        (ITEM_WIDTH - 5) / 2.0,
        30
    }, "Tạo đỉnh mới")) GUIAddVertex(state);

    if(GuiButton((Rectangle){
        panelArea->x + (ITEM_WIDTH - 5) / 2.0 + MARGIN * 2,
        currentY,
        (ITEM_WIDTH - 5) / 2.0,
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

    if(state->pathPage || state->shortestPathResult == NO_PATH) GuiDisable();
    if(GuiButton((Rectangle){
        panelArea->x + MARGIN,
        currentY,
        ITEM_WIDTH,
        30
    }, "Hiện bảng kết quả")) {
        state->pathPage = true;
    }
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
    )) TogglePhysicsSim(state);
    currentY += 35;

    if(GuiButton(
        (Rectangle){
            panelArea->x + MARGIN,
            currentY,
            ITEM_WIDTH,
            30
        },
        "Xáo đỉnh"
    )) ShuffleVert(state);
    currentY += 35;

    GuiLabel(
        (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 20 },
        "Độ dài lò xo"
    );
    currentY += 20;

    float oldSpringLength = state->springLength;
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

    if(oldSpringLength != state->springLength) {
        state->currentTemperature = INITIAL_TEMPERATURE;
    }

    GuiLabel(
        (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 20 },
        "Hệ số đàn hồi"
    );
    currentY += 20;

    float oldSpringStiffness = state->springStiffness;
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
        5.0
    );
    GuiLabel(
        (Rectangle){ panelArea->x + MARGIN * 2, currentY, ITEM_WIDTH - MARGIN, 30 },
        TextFormat("%.3f", state->springStiffness)
    );
    currentY += 35;

    if(oldSpringStiffness != state->springStiffness) {
        state->currentTemperature = INITIAL_TEMPERATURE;
    }

    GuiLabel(
        (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 20 },
        "Lực đẩy đỉnh"
    );
    currentY += 20;

    float oldCoulombConstant = state->coulombConstant;
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

    if(oldCoulombConstant != state->coulombConstant) {
        state->currentTemperature = INITIAL_TEMPERATURE;
    }
}

static void DrawVertexInfo(GUIState *state, Rectangle *panelArea, int *currentY) {
    Vertex *vert = state->selectedVertex;

    GuiLabel(
        (Rectangle){ panelArea->x + MARGIN, *currentY, ITEM_WIDTH, 20 },
        TextFormat("Đỉnh %u", vert->id)
    );
    *currentY += 23;

    if(vert->adjacent_count > 0) {
        GuiLabel(
            (Rectangle){ panelArea->x + MARGIN, *currentY, ITEM_WIDTH, 20 },
            "Đỉnh kề"
        );
        *currentY += 25;
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
            "Không có đỉnh kề"
        );
        *currentY += 23;
    }
}

void GUIDrawVertexInspect(GUIState *state, Rectangle *panelArea) {
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

    DrawVertexInfo(state, panelArea, &currentY);

    if(GuiButton(
        (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 30 },
        "Bắt đầu cạnh mới"
    )) {
        state->edgeStartVertex = state->selectedVertex;
        state->current_mode = MODE_CREATE_EDGE;
        state->selectedVertex = NULL;
    }
    currentY += 35;

    if(GuiButton(
        (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 30 },
        "Xóa đỉnh"
    )) GUIDeleteVert(state);
    currentY += 35;
}

void GUIDrawEdgeInspect(GUIState *state, Rectangle *panelArea) {
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
    
    weight_unit_t pWeight = edge->weight;
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
        GUIFindShortestPath(state);
    }
    currentY += 35;

    if(GuiButton(
        (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 30 },
        "Đảo chiều"
    )) GUIReverseEdge(state);
    currentY += 35;

    if(GuiButton(
        (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 30 },
        "Thêm cạnh ngược chiều"
    )) {
        GUIAddEdge(state, state->selectedEdge->target, state->selectedEdge->origin);
    }
    currentY += 35;

    if(GuiButton(
        (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 30 },
        "Xóa cạnh"
    )) GUIDeleteEdge(state);
    currentY += 35;
}

void GUIDrawCreateEdge(GUIState *state, Rectangle *panelArea) {
    int currentY = START_Y - 10;

    if(!state->edgeStartVertex) {
        GuiLabel(
            (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 20 },
            "Chọn đỉnh bắt đầu cạnh"
        );
        currentY += 23;
    } else {
        GuiLabel(
            (Rectangle){ panelArea->x + MARGIN, currentY, ITEM_WIDTH, 20 },
            "Chọn đỉnh kết thúc cạnh"
        );
        currentY += 23;

    }
    if(state->selectedVertex) {
        DrawVertexInfo(state, panelArea, &currentY);
    }
}

static void DrawTextCenter(Vector2 position, float panelWidth, const char* text, Font font, unsigned fontSize) {
    Vector2 textOrigin = MeasureTextEx(font, text, fontSize, 2);
    textOrigin.x /= -2.0;
    textOrigin.y /= -2.0;
    position.x += panelWidth / 2.0f;
    textOrigin = Vector2Add(textOrigin, position);
    DrawTextEx(font, text, textOrigin, fontSize, 2, GRAY);
}

static void _DrawPathPage(Vector2 position, GUIState *state) {
    float currentX = position.x + MARGIN;
    float currentY = position.y + MARGIN + RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT;

    if(!state->pathStartVertex || !state->pathEndVertex) {
        GuiLabel(
            (Rectangle){ currentX, currentY, 550, 20 },
            "Chưa chọn đỉnh bắt đầu/kết thúc"
        );
        return;
    }

    if(state->shortestPathResult == NO_PATH) {
        GuiLabel(
            (Rectangle){ currentX, currentY, 550, 20 },
            TextFormat(
                "Không tồn tại đường đi từ đỉnh %d đến đỉnh %d",
                state->pathStartVertex->id,
                state->pathEndVertex->id
            )
        );
        return;
    }

    GuiLabel(
        (Rectangle){ currentX, currentY, 550, 20 },
        TextFormat(
            "Chi phí tối thiểu để đi từ đỉnh %d đến đỉnh %d là %d",
            state->pathStartVertex->id,
            state->pathEndVertex->id,
            state->shortestPathResult
        )
    );
    currentY += 40;

    GuiLabel(
        (Rectangle){ currentX, currentY, ITEM_WIDTH, 20 },
        "Đường đi:"
    );
    currentY += 35;

    Vertex *vertStack[128];
    unsigned stkTop = 0;
    Vertex *current = state->pathEndVertex;

    while(current != state->pathStartVertex) {
        vertStack[stkTop++] = current;
        current = current->path_prev; 
    }
    vertStack[stkTop++] = state->pathStartVertex;

    const unsigned MAX_VERT_PER_LINE = 10;

    unsigned vertCount = 0;
    char buffer[32];
    currentX = position.x + MARGIN;
    while(stkTop > 0) {
        Vertex *vert = vertStack[stkTop - 1];
        vertCount++;

        snprintf(buffer, 32, "%d", vert->id);

        DrawTextCenter(
            (Vector2){ currentX, currentY },
            45,
            buffer,
            state->font,
            20
        );

        currentX += 24;

        if(vert != state->pathEndVertex) {
            DrawTextCenter(
                (Vector2){ currentX, currentY },
                45,
                "->",
                state->font,
                20
            );
            currentX += 24;
        }

        if(vertCount % MAX_VERT_PER_LINE == 0) {
            currentY += 24;
            currentX = position.x + MARGIN;
        }

        stkTop--;
    }
    currentX = position.x + MARGIN;

    const unsigned BUTTON_WIDTH = 140;
    const unsigned BUTTON_HEIGHT = 30;

    if(GuiButton(
        (Rectangle){
            550 - MARGIN - BUTTON_WIDTH + position.x,
            180 - MARGIN - BUTTON_HEIGHT + position.y,
            BUTTON_WIDTH,
            BUTTON_HEIGHT
        },
        "Xuất kết quả"
    )) {
        state->resultFileDialogState.windowActive = true;
    }
}


// taken from
// https://github.com/raysan5/raygui/blob/master/examples/floating_window/floating_window.c
static void GuiWindowFloating(
    Vector2 *position,
    Vector2 size,
    bool *minimized,
    bool *moving,
    void (*draw_content)(Vector2, GUIState*),
    const char* title,
    GUIState* state
) {
    #if !defined(RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT)
        #define RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT 24
    #endif

    #if !defined(RAYGUI_WINDOW_CLOSEBUTTON_SIZE)
        #define RAYGUI_WINDOW_CLOSEBUTTON_SIZE 18
    #endif

    int close_title_size_delta_half = (RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT - RAYGUI_WINDOW_CLOSEBUTTON_SIZE) / 2;

    // window movement and resize input and collision check
    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !*moving) {
        Vector2 mouse_position = GetMousePosition();

        Rectangle title_collision_rect = {
            position->x,
            position->y,
            size.x - (RAYGUI_WINDOW_CLOSEBUTTON_SIZE + close_title_size_delta_half),
            RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT
        };

        if(CheckCollisionPointRec(mouse_position, title_collision_rect)) {
            *moving = true;
        }
    }

    // window movement and resize update
    if(*moving) {
        Vector2 mouse_delta = GetMouseDelta();
        position->x += mouse_delta.x;
        position->y += mouse_delta.y;

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            *moving = false;

            // clamp window position keep it inside the application area
            if(position->x < 0) position->x = 0;
            else if(position->x > GetScreenWidth() - size.x) position->x = GetScreenWidth() - size.x;
            if(position->y < 0) position->y = 0;
            else if(position->y > GetScreenHeight()) position->y = GetScreenHeight() - RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT;
        }

    }

    // window and content drawing with scissor and scroll area
    if(*minimized) {
        GuiStatusBar((Rectangle){ position->x, position->y, size.x, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT }, title);

        if (GuiButton((Rectangle){ position->x + size.x - RAYGUI_WINDOW_CLOSEBUTTON_SIZE - close_title_size_delta_half,
                                   position->y + close_title_size_delta_half,
                                   RAYGUI_WINDOW_CLOSEBUTTON_SIZE,
                                   RAYGUI_WINDOW_CLOSEBUTTON_SIZE },
                                   "#120#")) {
            *minimized = false;
        }

    } else {
        *minimized = GuiWindowBox((Rectangle) { position->x, position->y, size.x, size.y }, title);

        // scissor and draw content within a scroll panel
        if(draw_content != NULL) {
            GuiPanel(
                (Rectangle) {
                    position->x,
                    position->y + RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT,
                    size.x,
                    size.y - RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT
                },
                NULL
            );

            draw_content(*position, state);
        }
    }
}

void GUIDrawPathPage(GUIState *state) {
    if(!state->pathPage) return;

    static Vector2 pagePosition = (Vector2){ 10, 10 };
    static bool isMoving = false;

    bool isMinimized = !state->pathPage;

    GuiWindowFloating(
        &pagePosition,
        (Vector2){550, 180},
        &isMinimized,
        &isMoving,
        _DrawPathPage,
        "Kết quả",
        state
    );

    state->pathPage = !isMinimized;
}

void GUIDrawAboutPage(GUIState *state) {
    if(!state->aboutPage) return;

    const unsigned ABOUTPAGE_WIDTH = 550;
    const unsigned ABOUTPAGE_HEIGHT = 380;
    const unsigned _MARGIN = 20;

    unsigned currentX = (GetScreenWidth() - ABOUTPAGE_WIDTH) / 2;
    unsigned currentY = (GetScreenHeight() - ABOUTPAGE_HEIGHT) / 2.0f;
    state->aboutPage = !GuiWindowBox(
        (Rectangle) {
            currentX,
            currentY,
            ABOUTPAGE_WIDTH,
            ABOUTPAGE_HEIGHT
        },
        NULL
    );

    currentY += 58;
    currentX += _MARGIN;

    DrawTextCenter(
        (Vector2){ currentX, currentY },
        ABOUTPAGE_WIDTH - _MARGIN * 2,
        "PBL1: Đồ án lập trình tính toán",
        state->font,
        28
    );
    currentY += 32;

    DrawTextCenter(
        (Vector2){ currentX, currentY },
        ABOUTPAGE_WIDTH - _MARGIN * 2,
        "Đề tài 914",
        state->font,
        20
    );
    currentY += 24;

    DrawTextCenter(
        (Vector2){ currentX, currentY },
        ABOUTPAGE_WIDTH - _MARGIN * 2,
        "Tìm đường đi ngắn nhất",
        state->font,
        26
    );
    currentY += 14;

    GuiLabel(
        (Rectangle) {currentX, currentY, ABOUTPAGE_WIDTH, 100},
        "Giáo viên hướng dẫn:"
    );
    currentY += 24;

    GuiLabel(
        (Rectangle) {currentX + 15, currentY, ABOUTPAGE_WIDTH, 100},
        "Đỗ Thị Tuyết Hoa"
    );
    currentY += 35;

    GuiLabel(
        (Rectangle) {currentX, currentY, ABOUTPAGE_WIDTH, 100},
        "Sinh viên thực hiện:"
    );
    currentY += 24;

    GuiLabel(
        (Rectangle) {currentX + 15, currentY, ABOUTPAGE_WIDTH, 100},
        "Trần Đức Minh Nhật"
    );

    GuiLabel(
        (Rectangle) {currentX + 215, currentY, ABOUTPAGE_WIDTH, 100},
        "25T_DT1"
    );

    currentY += 24;

    GuiLabel(
        (Rectangle) {currentX + 15, currentY, ABOUTPAGE_WIDTH, 100},
        "Trần Vi Diệu"
    );

    GuiLabel(
        (Rectangle) {currentX + 215, currentY, ABOUTPAGE_WIDTH, 100},
        "25T_DT4"
    );
}
