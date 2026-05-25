#include <io.h>
#include <stdio.h>
#include <stdlib.h>

#define ERR_NONE 0
#define ERR_FILE_OPEN 1
#define ERR_INP_FORMAT 2
#define ERR_MEMORY 3
#define ERR_INVALID_GRAPH 4
#define ERR_GRAPH_OPEN 5
Graph *load_graph_from_file(char *filename, int *errcode){
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
Graph *graph = make_graph(n);
if(graph == NULL){
    if(errcode != NULL){
        *errcode = ERR_MEMORY;
    }
    fclose(file);
    return NULL;
}
for (int i = 0; i < n; i++) {
        add_vertex(graph);
    }
for(unsigned i=0; i<m; i++){
    unsigned u, v, w;
    if(fscanf(file, "%u %u %u", &u, &v, &w) != 3){
        if(errcode != NULL){
            *errcode = ERR_INP_FORMAT;
        }
        delete_graph(graph);
        fclose(file);
        return NULL;
    }
    make_edge(graph->vertices[u], graph->vertices[v], w);
}
fclose(file);
if(errcode != NULL){
    *errcode = ERR_NONE;
}
return graph;
}
int save_graph_as_file(Graph *graph, int *errcode){
    if(graph == NULL){
        if(errcode != NULL){
            *errcode = ERR_GRAPH_OPEN;
        }
        return 0;
    }
    const char *filename = "doThi.txt";
    FILE *file = fopen(filename, "w");
    if(file == NULL){
        if(errcode != NULL){
            *errcode = ERR_FILE_OPEN;
        }
        return 0;
    }
    unsigned n = graph->vertex_count;
    unsigned m = 0;
    for(unsigned i=0; i<n; i++){
        m += graph->vertices[i]->adjacent_count;
    }
    fprintf(file, "%u %u\n", n, m);
    for(unsigned i=0; i<n; i++){
        Vertex *vertex = graph->vertices[i];
        for(unsigned j=0; j<vertex->adjacent_count; j++){
            Edge *edge = vertex->adjacents[j];
            fprintf(file, "%u %u %llu\n", edge->origin->id, edge->target->id, edge->weight);
        }
    }
    fclose(file);
    if(errcode != NULL){
        *errcode = ERR_NONE;
    }
    return 1;
}
