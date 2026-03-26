#ifndef GRAPH_H
#define GRAPH_H

struct Vector2;

/**
 * @brief Cạnh của một đỉnh, khởi tạo khi thêm 1 đỉnh liền kể
 */
typedef struct Edge {
    struct Vertex *origin; /**< Đỉnh xuất phát */
    struct Vertex *target; /**< Đích của cạnh */
    unsigned long long weight; /**< Trọng số/chi phí */
    unsigned idx; /**< Chỉ số của cạnh trong mảng kề của đỉnh xuất phát */
} Edge;


/**
 * @brief Đỉnh của một đồ thị
 */
typedef struct Vertex {
    unsigned id; /**< Chỉ số độc nhất của mỗi đỉnh */
    unsigned idx; /**< Chỉ số của đỉnh trong danh sách đỉnh */

    struct Edge **adjacents; /**< Danh sách (list) các con trỏ trỏ đến cạnh liền kể */
    unsigned adjacent_count; /**< Số lượng đỉnh liền kể */
    unsigned max_adjacent_count; /**< Số lượng tối đa đỉnh liền kề */

    /** Đỉnh phải đi qua để đến được đỉnh hiện tại,
     * dùng trong hàm quy hồi sau khi chạy thuật đường đi ngắn nhất */
    struct Vertex *path_prev;

    struct Vector2 *position; /**< Vị trí của đỉnh trong canvas */
} Vertex;

typedef struct Graph {
    Vertex **vertices; /**< Danh sách con trỏ của các đỉnh */
    unsigned vertex_count; /**< Số đỉnh hiện có */
    unsigned max_vertex_count; /**< Số đỉnh tối đa */

    unsigned unique_id; /**< id để dùng cho đỉnh mới, tăng khi tạo 1 đỉnh */
} Graph;

/**
 * @brief Tạo một đồ thị mới
 * @param[n] Số lượng đỉnh tối đa
 * @return Con trỏ trỏ đến đồ thị mới tạo, NULL nếu không tạo được
 */
Graph *make_graph(unsigned n);

/**
 * @brief Xóa đồ thị, giải phóng dữ liệu
 * @param[graph] Đồ thị cần xóa
 */
void delete_graph(Graph *graph);

/**
 * @brief Thêm một đỉnh mới vào đồ thị
 * @param[graph] Đồ thị cần thêm điểm
 * @return Con trỏ trỏ đến đỉnh mới, NULL nếu không tạo được
 */
Vertex *add_vertex(Graph *graph);

/**
 * @brief Tìm 1 đỉnh bằng id
 * @param[graph] Đồ thị chứa đỉnh cần tìm
 * @param[id] id của đỉnh cần tìm
 * @return Chỉ số của đỉnh nếu có, không tìm thấy trả -1
 */
int find_vertex(Graph *graph, unsigned id);

/**
 * @brief Xóa một đỉnh ra khỏi đồ thị bằng id
 * @param[graph] Đồ thị chứa đỉnh cần xóa
 * @param[id] id của đỉnh cần xóa
 * @return 1 nếu xóa thành công, 0 nếu không tìm thấy
 */
int remove_vertex(Graph *graph, unsigned id);

/**
 * @brief Tìm 1 cạnh dựa vào đỉnh xuất phát và đinh kết thúc
 * @param[a] Đỉnh xuất phát
 * @param[b] Đỉnh xuất phát
 * @return địa chỉ của cạnh nếu tìm thấy, NULL nếu không tìm thấy
 */
Edge *find_edge(Vertex *a, Vertex *b);

/**
 * @brief Tạo cạnh có hướng nối 2 điểm
 * @param[from] Đỉnh nguồn
 * @param[to] Đỉnh đích
 * @param[weight] Trọng số
 * @return Con trỏ trỏ đến cạnh mới tạo, NULL nếu không tạo được
 */
Edge *make_edge(Vertex *from, Vertex *to, unsigned weight);

/**
 * @brief Xóa một cạnh của đỉnh gốc
 * @param[edge] Con trỏ đến cạnh cần xóa
 * @return 1 nếu thành công, không thì 0
 */
int remove_edge(Edge *edge);

/**
 * @brief Tìm đường đi ngắn nhất, thông tin truy hồi được lưu ở mỗi đỉnh
 * @param[start] Đỉnh bắt đầu
 * @param[end] Đỉnh kết thúc
 * @return 1 nếu tồn tại đường đi giữa start và end, 0 nếu không có đường đi
 */
int shortest_path(Vertex *start, Vertex *end);

#endif
