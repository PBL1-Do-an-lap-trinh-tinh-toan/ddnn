#include <gui_interface.h>

GUIState state;

int main(void) {
    // test graph
    Graph *graph = make_graph(4);
    Vertex *v0 = add_vertex(graph);
    Vertex *v1 = add_vertex(graph);
    Vertex *v2 = add_vertex(graph);
    Vertex *v3 = add_vertex(graph);
    make_edge(v0, v3, 300);
    make_edge(v3, v2, 1);
    make_edge(v1, v3, 5);
    make_edge(v2, v0, 2);

    GUIInit(&state, "ddnn");
    GUILoadGraph(&state, graph);

    while(!WindowShouldClose()) {
        GUIUpdate(&state);
        GUIDraw(&state);
    }

    CloseWindow();

    return 0;
}
