#include <io.h>
#include <stdio.h>
#include <stdlib.h>

Graph *load_graph_from_file(const char *filename, int *errcode){
    FILE *file = fopen(filename, "r");
    if(file == NULL){
        if(errcode != NULL){
            *errcode = ERR_FILE_OPEN;
        }
        return NULL;
    }

    unsigned n, m;
    if(fscanf(file, "%u %u", &n, &m) != 2){
        if(errcode != NULL){
            *errcode = ERR_INP_FORMAT;
        }
        fclose(file);
        return NULL;
    }

    unsigned id[n];
    for(int i = 0; i < n; i++) {
        fscanf(file, "%u", id + i);
    }

    Graph *graph = make_graph(n + 100);
    if(graph == NULL){
        if(errcode != NULL){
            *errcode = ERR_MEMORY;
        }
        fclose(file);
        return NULL;
    }

    for (int i = 0; i < n; i++) {
        add_vertex(graph, id[i]);
    }

    for(unsigned i=0; i<m; i++){
        unsigned u, v;
        weight_unit_t w;
        if(fscanf(file, "%u %u %llu", &u, &v, &w) != 3){
            if(errcode != NULL){
                *errcode = ERR_INP_FORMAT;
            }
            delete_graph(graph);
            fclose(file);
            return NULL;
        }
        int u_idx = find_vertex(graph, u);
        int v_idx = find_vertex(graph, v);
        if(u_idx >= 0 && v_idx >= 0) {
            make_edge(graph->vertices[u_idx], graph->vertices[v_idx], w);
        } else {
            if(errcode != NULL){
                *errcode = ERR_INP_FORMAT;
            }
            delete_graph(graph);
            fclose(file);
            return NULL;
        }
    }

    fclose(file);
    if(errcode != NULL){
        *errcode = ERR_NONE;
    }
    return graph;
}

void save_graph_as_file(Graph *graph, const char *filename, int *errcode){
    if(graph->vertex_count == 0) {
        *errcode = ERR_INVALID_GRAPH;
        return;
    }

    FILE *file = fopen(filename, "w");
    if(file == NULL){
        *errcode = ERR_FILE_OPEN;
        return;
    }

    unsigned n = graph->vertex_count;
    unsigned m = 0;
    unsigned id[n];
    for(unsigned i=0; i<n; i++){
        m += graph->vertices[i]->adjacent_count;
        id[i] = graph->vertices[i]->id;
    }

    fprintf(file, "%u %u\n", n, m);
    for(int i = 0; i < n; i++) {
        fprintf(file, "%u ", id[i]);
    }
    fprintf(file, "\n");

    for(unsigned i=0; i<n; i++){
        Vertex *vertex = graph->vertices[i];
        for(unsigned j=0; j<vertex->adjacent_count; j++){
            Edge *edge = vertex->adjacents[j];
            fprintf(file, "%u %u %llu\n", edge->origin->id, edge->target->id, edge->weight);
        }
    }

    fclose(file);
    *errcode = ERR_NONE;
}

void save_result_to_file(Vertex *start, Vertex *end, weight_unit_t total_cost, const char *filepath, int *err) {
    FILE *file = fopen(filepath, "w");

    if(file == NULL){
        *err = ERR_FILE_OPEN;
        return;
    }

    fprintf(
        file,
        "Chi phí tối thiểu để đi từ đỉnh %d đến đỉnh %d là %llu\n",
        start->id,
        end->id,
        total_cost
    );

    fprintf(file, "Đường đi:\n");

    Vertex *vertStack[128];
    unsigned stkTop = 0;
    Vertex *current = end;

    while(current != start) {
        vertStack[stkTop++] = current;
        current = current->path_prev; 
    }
    vertStack[stkTop++] = start;

    while(stkTop > 0) {
        Vertex *vert = vertStack[stkTop - 1];

        fprintf(file, "%d", vert->id);

        if(vert != end) {
            fprintf(file, "->");
        }

        stkTop--;
    }

    fclose(file);
    *err = ERR_NONE;
}

