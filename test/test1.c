#include <stdio.h>
#include <stdlib.h>
#include "graph.h"

int main() {
    Graph *g = make_graph(5);
    Vertex *A = add_vertex(g);
    Vertex *B = add_vertex(g);
    Vertex *C = add_vertex(g);
    Vertex *D = add_vertex(g);
    Vertex *E = add_vertex(g);
    make_edge(A, B, 10);
    make_edge(A, C, 3);
    make_edge(C, B, 1);
    make_edge(B, D, 2);
    make_edge(C, D, 8);
    make_edge(C, E, 2);
    make_edge(E, D, 9);
    long long kc = shortest_path(g, A, D);
    if (kc == -1) {
        printf("-> Khong co duong di nao tu A den D!\n");
    } else {
        printf("-> Khoang cach ngan nhat tu A den D la: %lld (Ky vong: 6)\n", distance);
        printf("-> Lo trinh di nguoc tu dich ve nguon: ");
        Vertex *temp = D;
        while (temp != NULL) {
            // In ID của đỉnh
            printf("%d ", temp->id);
            if (temp->path_prev != NULL) {
                printf("<- ");
            }
            temp = temp->path_prev; 
        }
        printf("\n(Ky vong in ra: 3 <- 1 <- 2 <- 0)\n");
    }
    delete_graph(g);
    return 0;
}
