#include <gui_interface.h>

GUIState state;

int main(void) {
    GUIInit(&state, "ddnn", "access/IBMPlexMono-Regular.ttf");

Graph *g = make_graph(15 * 2);

    Vertex *V1  = add_vertex(g);
    Vertex *V2  = add_vertex(g);
    Vertex *V3  = add_vertex(g);
    Vertex *V4  = add_vertex(g);
    Vertex *V5  = add_vertex(g);
    Vertex *V6  = add_vertex(g);
    Vertex *V7  = add_vertex(g);
    Vertex *V8  = add_vertex(g);
    Vertex *V9  = add_vertex(g);
    Vertex *V10 = add_vertex(g);
    Vertex *V11 = add_vertex(g);
    Vertex *V12 = add_vertex(g);
    Vertex *V13 = add_vertex(g);
    Vertex *V14 = add_vertex(g);
    Vertex *V15 = add_vertex(g);

    make_edge(V1, V2, 2);
    make_edge(V1, V3, 5);
    make_edge(V1, V4, 10);

    make_edge(V2, V5, 3);
    make_edge(V2, V3, 1);
    make_edge(V3, V2, 2);

    make_edge(V3, V6, 4);
    make_edge(V4, V7, 2);

    make_edge(V5, V8, 7);
    make_edge(V5, V6, 1);
    make_edge(V6, V5, 2);

    make_edge(V6, V8, 2);
    make_edge(V6, V9, 8);
    make_edge(V7, V9, 4);
    make_edge(V7, V10, 12);

    make_edge(V8, V11, 1);
    make_edge(V9, V11, 3);
    make_edge(V9, V12, 5);
    make_edge(V10, V13, 2);

    make_edge(V11, V14, 6);
    make_edge(V12, V14, 2);
    make_edge(V13, V14, 1);
    make_edge(V13, V15, 10);

    make_edge(V14, V15, 3);
    make_edge(V11, V15, 15);

    GUILoadGraph(&state, g);

    while(!WindowShouldClose()) {
        GUIUpdate(&state);
        GUIDraw(&state);
    }

    CloseWindow();

    return 0;
}
