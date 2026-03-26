#include <graph.h>
#include <stdlib.h>
#define INF 999999999999999LL
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

typedef struct {
    Vertex *u;
    long long dist;
} HeapNode;
HeapNode *heap = NULL;
int heap_size = 0;
int head_capacity = 0;
void swap_node(HeapNode *a, HeapNode *b) {
    HeapNode temp = *a;
    *a = *b;
    *b = temp;
}
void push(Vertex *u, long long dist) {
    if(heap_size + 1 >= head_capacity){
        head_capacity = (head_capacity == 0) ? 256 : head_capacity * 2;
        HeapNode *temp = (HeapNode*)realloc(heap, head_capacity * sizeof(HeapNode));
        if (!temp) return;
        heap = temp;
    }
    heap[++heap_size] = (HeapNode){u, dist};
    int i = heap_size;
    while (i > 1 && heap[i].dist < heap[i / 2].dist) {
        swap_node(&heap[i], &heap[i / 2]);
        i /= 2;
    }
}

HeapNode pop() {
    HeapNode top = heap[1];
    heap[1] = heap[heap_size--];
    int i = 1;
    while (i * 2 <= heap_size) {
        int child = i * 2;
        if (child + 1 <= heap_size && heap[child + 1].dist < heap[child].dist) {
            child++;
        }
        if (heap[i].dist <= heap[child].dist) break;
        swap_node(&heap[i], &heap[child]);
        i = child;
    }
    return top;
}

int is_empty() {
    return heap_size == 0;
}
void free_heap() {
    if(heap != NULL){
        free(heap);
        heap = NULL;
    }
    heap_size = 0;
    head_capacity = 0;
}

long long shortest_path(Graph *graph, Vertex *start, Vertex *end) {
    /**
     * Lưu thông tin truy hồi ở mỗi đỉnh
     * vd nếu tìm được đường đi là A -> B -> C
     * thì A->path_prev = NULL
     * B->path_prev = A
     * C->path_prev = B
     *
     * nên làm bằng thuật Dijkstra
     */
    long long *d = (long long*)malloc(graph->unique_id*sizeof(long long));
    if(!d) return -1;
    for(unsigned i = 0; i<graph->unique_id; i++) {
        d[i] = INF;
    }
    free_heap();
    d[start->id] = 0;
    start->path_prev = NULL;
    push(start, 0);
    while(!is_empty()){
        HeapNode current = pop();
        Vertex *u = current.u;
        long long dist_u = current.dist;
        if(dist_u > d[u->id]) continue;
        if(u == end) break; 
        for(unsigned i=0; i < u->adjacent_count; i++) {
            Edge *edge = u->adjacents[i];
            Vertex *v = edge->target;
            long long new_dist = dist_u + edge->weight;
            if(new_dist < d[v->id]){
                d[v->id] = new_dist;
                v->path_prev = u;
                push(v, new_dist);
            }
        }
    }
long long result = d[end->id];
free(d);
free_heap();
if(result == INF) result = -1;
return result;
}
