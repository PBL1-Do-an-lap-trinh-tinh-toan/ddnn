#include <raylib/raylib.h>
#include <gui_extras.h>
#include <graph.h>

#define RAYGUI_IMPLEMENTATION
#include <raylib/raygui.h>

static char status[128] = "normal";

int main(void) {
    // test graph
    Graph *graph = make_graph(4);
    Vertex *v0 = add_vertex(graph);
    Vertex *v1 = add_vertex(graph);
    Vertex *v2 = add_vertex(graph);
    Vertex *v3 = add_vertex(graph);
    make_edge(v0, v3, 3);
    make_edge(v3, v2, 1);
    make_edge(v1, v3, 5);
    make_edge(v2, v0, 2);

    v0->position = (Vector2){ 0, 0 };
    v1->position = (Vector2){ 150, 0 };
    v2->position = (Vector2){ 0, 130 };
    v3->position = (Vector2){ 100, 100 };

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "ddnn");

    Camera2D camera = { 0 };
    camera.target = Vector2Zero();
    camera.offset = (Vector2){ (float)CANVAS_WIDTH / 2, (float)CANVAS_HEIGHT / 2 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    SetTargetFPS(60);

    while(!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(RAYWHITE);

        if(IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
            camera.target = Vector2Subtract(
                camera.target,
                Vector2Divide(GetMouseDelta(), (Vector2){ camera.zoom, camera.zoom })
            );
        }

        float mouse_wheel = GetMouseWheelMove();
        if(mouse_wheel > 0) {
            camera.zoom *= 1.2;
        } else if (mouse_wheel < 0) {
            camera.zoom /= 1.2;
        }

        BeginMode2D(camera);
        GuiOffsettedGrid(camera, 20);
        DrawGraph(graph);
        EndMode2D();

        DrawText("abcdxyz", 190, 200, 20, LIGHTGRAY);

        GuiPanel(
            (Rectangle){
                SCREEN_WIDTH - PANEL_WIDTH,
                0,
                PANEL_WIDTH,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT + 1
            },
            "Control"
        );

        GuiStatusBar(
            (Rectangle){
                0,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                SCREEN_WIDTH,
                STATUS_BAR_HEIGHT
            },
            status
        );

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
