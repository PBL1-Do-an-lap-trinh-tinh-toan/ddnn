#ifndef IO_H
#define IO_H

#include <graph.h>

/**
 * # Quy ước
 * - Đồ thị có n đỉnh thì tên của các đỉnh là các số từ 0 cho đến n.
 *   Ví dụ đồ thị gồm 3 đỉnh thì có các đỉnh là 0, 1  2
 * # Định dạng của file lưu đồ thị
 * - dòng đầu: số đỉnh n và số cạnh m
 * - m dòng tiếp theo: chỉ số đỉnh nguồn, chỉ số đỉnh đích và trọng số của cạnh
 * # Ví dụ
 * `
 *     4 3
 *     1 0 6
 *     0 2 2
 *     1 3 7
 *     2 1 5
 * `
 * File này phải được dịch thành:
 * `
 *     Cho đồ thị gồm 4 đỉnh có các chỉ số trong đoạn [0, 3], trong đó có các cạnh:
 *         - đi từ đỉnh 1 đến đỉnh 0, trọng số là 6
 *         - đi từ đỉnh 0 đến đỉnh 2, trọng số là 2
 *         - đi từ đỉnh 1 đến đỉnh 3, trọng số là 7
 *         - đi từ đỉnh 2 đến đỉnh 1, trọng số là 5
 * `
 */

/**
 * @brief Khởi tạo đồ thị từ file
 * @param[filename] Đường dẫn tới file
 * @param[errcode] Mã lỗi, được gán khi xảy ra lỗi
 * @return Con trỏ trỏ đến đồ thị được tải, NULL nếu không tải được
 */
Graph *load_graph_from_file(char *filename, int *errcode);

/**
 * @brief Lưu đồ thị dưới dạng file văn bản
 * @param[graph] Đồ thị cần lưu
 * @param[errcode] Mã lỗi, được gán khi xảy ra lỗi
 * @return 1 nếu thành công, không thì 0
 */
int save_graph_as_file(Graph *graph, int *errcode);

#endif
