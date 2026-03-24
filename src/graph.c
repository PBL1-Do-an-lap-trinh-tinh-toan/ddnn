#include <graph.h>
#include <stdlib.h>

Graph *make_graph(unsigned n) {
    Graph *g = (Graph*)malloc(sizeof(Graph));
    if(!g) return NULL;

    g->max_vertex_count = n;
    g->vertex_count = 0;
    g->vertices = (Vertex**)malloc(n * sizeof(Vertex*));
    if(!g->vertices) {
        free(g);
        return NULL;
    }
    g->unique_id = 0;

    return g;
}

void delete_graph(Graph *graph) {
    for(unsigned i = 0; i < graph->vertex_count; i++) {
        Vertex *vertex = graph->vertices[i];
        for(unsigned j = 0; j < vertex->adjacent_count; j++) {
            free(vertex->adjacents[j]);
        }
        free(vertex->adjacents);
        free(vertex);
    }

    free(graph->vertices);
    free(graph);
}

Vertex *add_vertex(Graph *graph) {
    if(graph->vertex_count == graph->max_vertex_count)
        return NULL;

    Vertex *new_vertex = (Vertex*)malloc(sizeof(Vertex));
    if(!new_vertex)
        return NULL;

    // khoi tao cac gia tri ban dau
    new_vertex->id = graph->unique_id++;
    new_vertex->adjacent_count = 0;
    new_vertex->max_adjacent_count = graph->max_vertex_count - 1;
    new_vertex->adjacents = (Edge**)malloc(new_vertex->max_adjacent_count * sizeof(Edge*));
    if(!new_vertex->adjacents) {
        free(new_vertex);
        return NULL;
    }
    new_vertex->path_prev = NULL;
    new_vertex->position.x = 0;
    new_vertex->position.y = 0;

    graph->vertices[graph->vertex_count++] = new_vertex;

    return new_vertex;
}

int find_vertex(Graph *graph, unsigned id) {
    for(unsigned i = 0; i < graph->vertex_count; i++) {
        if(graph->vertices[i]->id == id)
            return i;
    }
     return -1;
}

int remove_vertex(Graph *graph, unsigned id) {
    int idx = find_vertex(graph, id);
    if(idx < 0) return 0;

    Vertex *del_vertex = graph->vertices[idx];

    // xoa cac canh di den dinh
    for(unsigned i = 0; i < graph->vertex_count; i++) {
        Vertex *vertex = graph->vertices[i];
        for(int j = vertex->adjacent_count - 1; j >= 0; j--) {
            if(vertex->adjacents[j]->target == del_vertex) {
                remove_edge(vertex, j);
            }
        }
    }

    // xoa cac canh cua dinh
    for(unsigned i = 0; i < del_vertex->adjacent_count; i++) {
        free(del_vertex->adjacents[i]);
    }
    free(del_vertex->adjacents);
    free(del_vertex);

    for(unsigned i = idx + 1; i < graph->vertex_count; i++) {
        graph->vertices[i - 1] = graph->vertices[i];
    }
    graph->vertex_count--;

    return 1;
}

Edge *make_edge(Vertex *from, Vertex *to, unsigned weight) {
    if(from->adjacent_count == from->max_adjacent_count)
        return NULL;

    for(unsigned i = 0; i < from->adjacent_count; i++) {
        Edge *e = from->adjacents[i];
        if(e->target == to) {
            e->weight = weight;
            return e;
        }
    }

    Edge *e = (Edge*)malloc(sizeof(Edge));
    if(!e) return NULL;

    e->target = to;
    e->weight = weight;

    from->adjacents[from->adjacent_count++] = e;

    return e;
}

int remove_edge(Vertex *owner, unsigned edge_idx) {
    if(edge_idx >= owner->adjacent_count)
        return 0;

    free(owner->adjacents[edge_idx]);

    for(unsigned i = edge_idx + 1; i < owner->adjacent_count; i++) {
        owner->adjacents[i - 1] = owner->adjacents[i];
    }
    owner->adjacent_count--;

    return 1;
}

int shortest_path(Vertex *start, Vertex *end) {
    /**
     * Lưu thông tin truy hồi ở mỗi đỉnh
     * vd nếu tìm được đường đi là A -> B -> C
     * thì A->path_prev = NULL
     * B->path_prev = A
     * C->path_prev = B
     *
     * nên làm bằng thuật Dijkstra
     */
    return 0;
}
