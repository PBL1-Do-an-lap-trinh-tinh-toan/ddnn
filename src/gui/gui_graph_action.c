#include <raylib/raygui.h>
#include <gui.h>

#include <graph.h>
#include <stdio.h>
#include <string.h>
#include <constants.h>

bool GUILoadGraph(GUIState *state, Graph *graph) {
    state->graph = graph;

    float boundsSize = state->springLength * graph->vertex_count / 3.0;
    for(unsigned i = 0; i < graph->vertex_count; i++) {
        Vertex *vert = graph->vertices[i];
        BodyInit(state->graph->vertices[i]);
        vert->position.x = (GetRandomValue(-500, 500) / 500.0) * boundsSize;
        vert->position.y = (GetRandomValue(-500, 500) / 500.0) * boundsSize;
    }
    state->currentTemperature = INITIAL_TEMPERATURE;

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

void GUIFindShortestPath(GUIState *state) {
    if(!state->pathStartVertex || !state->pathEndVertex)
        return;

    state->shortestPathResult = shortest_path(state->graph, state->pathStartVertex, state->pathEndVertex);
    if(state->shortestPathResult != NO_PATH) {
        strcpy(
            state->statusBar,
            TextFormat(
                "Tìm được đường đi từ đỉnh %d đến đỉnh %d với chi phí tối thiểu là %lld",
                state->pathStartVertex->id,
                state->pathEndVertex->id,
                state->shortestPathResult
            )
        );
        state->pathPage = true;
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

void GUISetStartVert(GUIState *state) {
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

void GUISetEndVert(GUIState *state) {
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

void GUISwapStartEndVert(GUIState *state) {
    Vertex *t = state->pathStartVertex;
    state->pathStartVertex = state->pathEndVertex;
    state->pathEndVertex = t;
    snprintf(
        state->statusBar,
        sizeof(state->statusBar),
        "%s",
        "Đã hoán đổi đỉnh bắt đầu và đỉnh kết thúc"
    );
    GUIFindShortestPath(state);
}

void GUIAddEdge(GUIState *state, Vertex *startVert, Vertex *endVert) {
    Edge *edge;
    if(!(edge = find_edge(startVert, endVert)))
        edge = make_edge(startVert, endVert, 0);

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
        GUIFindShortestPath(state);
        snprintf(
            state->statusBar,
            sizeof(state->statusBar),
            "%s",
            TextFormat(
                "Đã thêm cạnh %d -> %d",
                startVert->id,
                endVert->id
            )
        );
    }

    state->edgeStartVertex = NULL;
    state->current_mode = MODE_EDGE_INSPECT;
}

void GUIDeleteEdge(GUIState *state) {
    if(!state->selectedEdge) return;
    
    unsigned from_id = state->selectedEdge->origin->id;
    unsigned to_id = state->selectedEdge->target->id;
    remove_edge(state->selectedEdge);

    state->selectedEdge = NULL;
    GUIFindShortestPath(state);

    snprintf(state->statusBar, sizeof(state->statusBar), "%s", TextFormat("Đã xóa cạnh %u -> %u", from_id, to_id));
}

void GUIReverseEdge(GUIState *state) {
    if(!state->selectedEdge || !state->graph) return;
    
    Vertex *from = state->selectedEdge->origin;
    Vertex *to = state->selectedEdge->target;
    unsigned from_id = from->id;
    unsigned to_id = to->id;
    weight_unit_t weight = state->selectedEdge->weight;

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

void GUIAddVertex(GUIState *state) {
    if(!state->graph) {
        state->graph = make_graph(100);
        if(!state->graph) {
            snprintf(state->statusBar, sizeof(state->statusBar), "%s", "Tạo đồ thị không thành công");
        }
    }

    if(state->graph) {
        Vertex *new_vert = add_vertex(state->graph, -1);
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

void GUIDeleteVert(GUIState *state) {
    Vertex *sel_vert = state->selectedVertex;
    if(!state->graph || !sel_vert) return;

    unsigned vert_id = sel_vert->id;
    remove_vertex(state->graph, sel_vert->id);
    state->selectedVertex = NULL;
    sel_vert = NULL;
    GUIFindShortestPath(state);

    snprintf(state->statusBar, sizeof(state->statusBar), "%s", TextFormat("Đã xóa đỉnh %u", vert_id));
}
