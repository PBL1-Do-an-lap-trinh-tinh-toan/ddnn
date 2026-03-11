#include "raylib/raymath.h"
#include <raylib/raylib.h>
#include <gui_extras.h>
#include <graph.h>
#include <physics.h>

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

    Body bodies[4];
    for(unsigned i = 0; i < graph->vertex_count; i++) {
        Vertex *vert = graph->vertices[i];
        BodyInit(bodies + i, &vert->position);
        
        vert->position.x = 67.8 * i * cos(i * 0.57);
        vert->position.y = 67.8 * i * sin(i * 0.57);
    }

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "ddnn");

    Camera2D camera = { 0 };
    camera.target = Vector2Zero();
    camera.offset = (Vector2){ (float)CANVAS_WIDTH / 2, (float)CANVAS_HEIGHT / 2 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    const float TARGET_DISTANCE = 200;
    const float SPRING_STIFFNESS = 0.7;
    const float COULOMB_CONSTANT = 0.121 * SPRING_STIFFNESS * TARGET_DISTANCE * TARGET_DISTANCE * TARGET_DISTANCE;

    SetTargetFPS(60);

    while(!WindowShouldClose()) {
        BeginDrawing();

        float deltaTime = Clamp(GetFrameTime(), 0.001, 0.5);

        // apply physics
        for(unsigned i = 0; i < graph->vertex_count; i++) {
            Vertex *vert1 = graph->vertices[i];

            // spring force
            for(unsigned j = 0; j < vert1->adjacent_count; j++) {
                Vertex *vert2 = vert1->adjacents[j]->target;

                unsigned id1 = i;
                unsigned id2 = vert2 - graph->vertices[0];

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
            bodies[i].velocity.x *= 0.98;
            bodies[i].velocity.y *= 0.98;
            bodies[i].velocity = Vector2Clamp(
                bodies[i].velocity,
                (Vector2){ -25, -25 },
                (Vector2){ 25, 25 }
            );
            Inertia(&bodies[i], deltaTime);
        }

        ClearBackground(RAYWHITE);

        if(IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
            camera.target = Vector2Subtract(
                camera.target,
                Vector2Divide(GetMouseDelta(), (Vector2){ camera.zoom, camera.zoom })
            );
        }

        float mouse_wheel = GetMouseWheelMove();
        if(mouse_wheel > 0 && camera.zoom < 3) {
            camera.zoom *= 1.2;
        } else if (mouse_wheel < 0 && camera.zoom > 0.1) {
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
