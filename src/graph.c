#include <graph.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    Vertex *u;
    weight_unit_t dist;
} HeapNode;

HeapNode *heap = NULL;
int heap_size = 0;
int head_capacity = 0;

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
    memset(g->vertices, 0, n * sizeof(Vertex*));
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
    memset(new_vertex->adjacents, 0, new_vertex->max_adjacent_count * sizeof(Edge*));
    new_vertex->path_prev = NULL;

    graph->vertices[graph->vertex_count] = new_vertex;
    new_vertex->idx = graph->vertex_count;
    graph->vertex_count++;

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
                remove_edge(vertex->adjacents[j]);
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
        graph->vertices[i - 1]->idx = i - 1;
    }
    graph->vertex_count--;

    return 1;
}

Edge *make_edge(Vertex *from, Vertex *to, unsigned weight) {
    if(from->adjacent_count == from->max_adjacent_count)
        return NULL;

    Edge *t = find_edge(from, to);
    if(t) {
        t->weight = weight;
        return t;
    }

    Edge *e = (Edge*)malloc(sizeof(Edge));
    if(!e) return NULL;

    e->origin = from;
    e->target = to;
    e->weight = weight;
    e->idx = from->adjacent_count;

    from->adjacents[from->adjacent_count++] = e;

    return e;
}

int remove_edge(Edge *edge) {
    unsigned edge_idx = edge->idx;
    Vertex *owner = edge->origin;
    free(owner->adjacents[edge->idx]);

    for(unsigned i = edge_idx + 1; i < owner->adjacent_count; i++) {
        owner->adjacents[i - 1] = owner->adjacents[i];
        owner->adjacents[i - 1]->idx = i - 1;
    }
    owner->adjacent_count--;

    return 1;
}

Edge *find_edge(Vertex *a, Vertex *b) {
    for(unsigned i = 0; i < a->adjacent_count; i++)
        if(a->adjacents[i]->target == b) return a->adjacents[i];
    return NULL;
}

void heap_swap_node(HeapNode *a, HeapNode *b) {
    HeapNode temp = *a;
    *a = *b;
    *b = temp;
}
void heap_push(Vertex *u, weight_unit_t dist) {
    if(heap_size + 1 >= head_capacity){
        head_capacity = (head_capacity == 0) ? 256 : head_capacity * 2;
        HeapNode *temp = (HeapNode*)realloc(heap, head_capacity * sizeof(HeapNode));
        if (!temp) return;
        heap = temp;
    }
    heap[++heap_size] = (HeapNode){u, dist};
    int i = heap_size;
    while (i > 1 && heap[i].dist < heap[i / 2].dist) {
        heap_swap_node(&heap[i], &heap[i / 2]);
        i /= 2;
    }
}

HeapNode heap_pop() {
    HeapNode top = heap[1];
    heap[1] = heap[heap_size--];
    int i = 1;
    while (i * 2 <= heap_size) {
        int child = i * 2;
        if (child + 1 <= heap_size && heap[child + 1].dist < heap[child].dist) {
            child++;
        }
        if (heap[i].dist <= heap[child].dist) break;
        heap_swap_node(&heap[i], &heap[child]);
        i = child;
    }
    return top;
}

int is_heap_empty() {
    return heap_size == 0;
}

void heap_clear() {
    if(heap != NULL){
        free(heap);
        heap = NULL;
    }
    heap_size = 0;
    head_capacity = 0;
}

weight_unit_t shortest_path(Graph *graph, Vertex *start, Vertex *end) {
    weight_unit_t *d = (weight_unit_t*)malloc(graph->unique_id*sizeof(weight_unit_t));
    if(!d) return -1;
    for(unsigned i = 0; i<graph->unique_id; i++) {
        d[i] = NO_PATH;
    }

    heap_clear();
    d[start->id] = 0;
    start->path_prev = NULL;
    heap_push(start, 0);

    while(!is_heap_empty()){
        HeapNode current = heap_pop();
        Vertex *u = current.u;
        weight_unit_t dist_u = current.dist;
        if(dist_u > d[u->id]) continue;
        if(u == end) break;
        for(unsigned i=0; i < u->adjacent_count; i++) {
            Edge *edge = u->adjacents[i];
            Vertex *v = edge->target;
            weight_unit_t new_dist = dist_u + edge->weight;
            if(new_dist < d[v->id]){
                d[v->id] = new_dist;
                v->path_prev = u;
                heap_push(v, new_dist);
            }
        }
    }

    weight_unit_t result = d[end->id];
    free(d);
    heap_clear();
    return result;
}
