#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <graph.h>

void test0() {
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

    long long dist = shortest_path(g, A, D);

    // expected path: A -> C -> B -> D
    // cost: 3 + 1 + 2 = 6
    assert(dist != -1); 
    assert(dist == 6);

    assert(D->path_prev == B);
    assert(B->path_prev == C);
    assert(C->path_prev == A);
    assert(A->path_prev == NULL);

    delete_graph(g);
    puts("dijkstra: test 0 passed");
}

void test1() {
    Graph *g = make_graph(10);
    
    Vertex *V[10];
    for (int i = 0; i < 10; i++) {
        V[i] = add_vertex(g);
    }

    make_edge(V[0], V[1], 4);
    make_edge(V[0], V[2], 2);
    make_edge(V[1], V[2], 5);
    
    make_edge(V[1], V[3], 6); 
    
    make_edge(V[2], V[4], 3);
    make_edge(V[4], V[3], 4); 
    
    make_edge(V[3], V[5], 2);
    make_edge(V[4], V[6], 8);
    make_edge(V[6], V[5], 2); 
    make_edge(V[5], V[7], 1);
    make_edge(V[5], V[8], 3);
    make_edge(V[6], V[7], 6);
    make_edge(V[7], V[8], 2);
    
    make_edge(V[7], V[9], 4); 
    
    make_edge(V[8], V[9], 7); 

    long long dist = shortest_path(g, V[0], V[9]);

    // expected path: 0 -> 2 -> 4 -> 3 -> 5 -> 7 -> 9
    // cost: 2 + 3 + 4 + 2 + 1 + 4 = 16
    assert(dist == 16);

    assert(V[9]->path_prev == V[7]);
    assert(V[7]->path_prev == V[5]);
    assert(V[5]->path_prev == V[3]);
    assert(V[3]->path_prev == V[4]);
    assert(V[4]->path_prev == V[2]);
    assert(V[2]->path_prev == V[0]);
    assert(V[0]->path_prev == NULL);

    delete_graph(g);
    puts("dijkstra: test 1 passed");
}

int main() {
    test0();
    test1();

    return 0;
}
