#include <stdio.h>
#include <stdlib.h>

#define MAXN 100005
#define MAXM 200005 
#define INF 1000000000

typedef struct {
    int to;
    int weight;
    int next;
} Edge;

Edge edges[MAXM];
int head[MAXN];
int edge_cnt = 0;

void add_edge(int u, int v, int w) {
    edge_cnt++;
    edges[edge_cnt].to = v;
    edges[edge_cnt].weight = w;
    edges[edge_cnt].next = head[u];
    head[u] = edge_cnt;
}
typedef struct {
    int u;
    long long dist;
} HeapNode;

HeapNode heap[MAXM * 2];
int heap_size = 0;

void swap_node(HeapNode *a, HeapNode *b) {
    HeapNode temp = *a;
    *a = *b;
    *b = temp;
}

void push(int u, long long dist) {
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

int n, m, s, t;
long long d[MAXN];
int pre[MAXN];

void nhap() {
    printf("Nhap n(so dinh), m(so canh), s(diem cuat phat), t(diem den): ");
    scanf("%d %d %d %d", &n, &m, &s, &t);
    for (int i = 0; i <= n; i++) head[i] = 0; 
    printf("Nhap theo thu tu: u v w (canh tu u toi v co trong so w)\n");
    for (int i = 0; i < m; i++) {
        int u, v, w;
        scanf("%d %d %d", &u, &v, &w);
        add_edge(u, v, w);
    }
}

void dijkstra(int s, int t) {
    for (int i = 1; i <= n; i++) {
        d[i] = INF;
        pre[i] = -1;
    }
    
    d[s] = 0;
    push(s, 0);

    while (!is_empty()) {
        HeapNode top = pop();
        int u = top.u;
        long long kc = top.dist;

        if (kc > d[u]) continue;

        for (int e = head[u]; e != 0; e = edges[e].next) {
            int v = edges[e].to;
            int w = edges[e].weight;

            if (d[v] > d[u] + w) {
                d[v] = d[u] + w;
                pre[v] = u;
                push(v, d[v]);
            }
        }
    }

    printf("Khoang cach ngan nhat la: %lld\n", d[t]); 
    int path[MAXN];
    int path_size = 0;
    int curr = t;

    while (1) {
        path[path_size++] = curr;
        if (curr == s) break;
        curr = pre[curr];
    }
    printf("Duong di ngan nhat tu %d den %d la: ", s, t);
    for (int i = path_size - 1; i >= 0; i--) {
        printf("%d ", path[i]);
    }
    printf("\n");
}

int main() {
    nhap();
    dijkstra(s, t);
    return 0;
}
