#include <io.h>

Graph *load_graph_from_file(char *filename, int *errcode);

int save_graph_as_file(Graph *graph, int *errcode);
