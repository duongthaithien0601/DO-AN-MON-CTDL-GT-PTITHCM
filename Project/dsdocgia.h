#pragma once
#include <string>
#include <random>
#include <ctime>
#include "cautruc.h"

// =================== CÁC HÀM TÌM KIẾM TRÊN CÂY ===================
// Tìm kiếm độc giả theo Mã thẻ (Binary Search)
inline DocGiaNode* tim_node_doc_gia(DocGiaNode* root, int maThe) {
    DocGiaNode* p = root;
    while (p != NULL) {
        if (maThe < p->info.maThe) {
            p = p->left;  // Nhỏ hơn -> Sang trái
        }
        else if (maThe > p->info.maThe) {
            p = p->right; // Lớn hơn -> Sang phải
        }
        else {
            return p;     // Tìm thấy
        }
    }
    return NULL; // Không tìm thấy
}
// Kiểm tra xem mã thẻ đã tồn tại hay chưa
inline bool _exists_ma_the(DocGiaNode* root, int maThe) {
    return tim_node_doc_gia(root, maThe) != NULL;
}
// Tìm node có giá trị nhỏ nhất trong cây con (Dùng khi xóa node có 2 con)
inline DocGiaNode* _find_min(DocGiaNode* root) {
    if (root == NULL) return NULL;
    while (root->left != NULL) {
        root = root->left;
    }
    return root;
}

// =================== THAY ĐỔI CẤU TRÚC CÂY (THÊM / XÓA NODE) =====================
// Chèn một độc giả mới vào cây (BST Insertion)
inline void insert_doc_gia(DocGiaNode*& root, const DocGia& v) {
    if (root == NULL) {
        root = new DocGiaNode(v);
        return;
    }
    DocGiaNode* p = root;
    DocGiaNode* parent = NULL;
    // Tìm vị trí lá (leaf) phù hợp
    while (p != NULL) {
        parent = p;
        if (v.maThe < p->info.maThe) {
            p = p->left;
        }
        else if (v.maThe > p->info.maThe) {
            p = p->right;
        }
        else {
            // Trùng mã thẻ -> Không thêm để bảo toàn dữ liệu cũ
            return;
        }
    }
    // Gắn node mới vào cha
    if (v.maThe < parent->info.maThe) {
        parent->left = new DocGiaNode(v);
    }
    else {
        parent->right = new DocGiaNode(v);
    }
}
// Hàm đệ quy xóa node theo Mã thẻ (BST Deletion)
inline void _delete_node(DocGiaNode*& root, int maThe) {
    if (root == NULL) return;

    if (maThe < root->info.maThe) {
        _delete_node(root->left, maThe);
    }
    else if (maThe > root->info.maThe) {
        _delete_node(root->right, maThe);
    }
    else {
        // Đã tìm thấy node cần xóa (root hiện tại)
        DocGiaNode* del = root;

        // TH1: Node có 1 con hoặc không có con
        if (root->left == NULL) {
            root = root->right;
            delete del;
        }
        else if (root->right == NULL) {
            root = root->left;
            delete del;
        }
        // TH2: Node có 2 con
        else {
            // Tìm phần tử thay thế (nhỏ nhất bên phải)
            DocGiaNode* succ = _find_min(root->right);
            // Sao chép giá trị
            root->info = succ->info;
            // Đệ quy xóa phần tử thay thế đó
            _delete_node(root->right, succ->info.maThe);
        }
    }
}

// ==================== TIỆN ÍCH & LOGIC NGHIỆP VỤ ====================
// Đếm số lượng sách độc giả đang mượn (để kiểm tra điều kiện xóa)
inline int dem_mt_dang_muon(const DocGia& dg) {
    int cnt = 0;
    for (MuonTraNode* p = dg.mtHead; p != NULL; p = p->next) {
        if (p->trangThai == MT_DANG_MUON) {
            cnt++;
        }
    }
    return cnt;
}
// Xóa độc giả an toàn (Chỉ xóa nếu không còn giữ sách thư viện)
inline bool xoa_doc_gia_if_no_borrowing(DocGiaNode*& root, int maThe) {
    DocGiaNode* p = tim_node_doc_gia(root, maThe);
    if (p == NULL) return false; // Không tồn tại
    // Nếu đang mượn sách -> Từ chối xóa
    if (dem_mt_dang_muon(p->info) > 0) {
        return false;
    }
    // Thực hiện xóa
    _delete_node(root, maThe);
    return true;
}
// Sinh mã thẻ ngẫu nhiên và đảm bảo duy nhất
inline int gen_ma_the_unique(DocGiaNode* root) {
    static std::mt19937 rng(static_cast<unsigned>(std::time(NULL)));
    std::uniform_int_distribution<int> dist(100000, 999999);
    // Thử random 2048 lần
    for (int i = 0; i < 2048; i++) {
        int cand = dist(rng);
        if (!_exists_ma_the(root, cand)) {
            return cand;
        }
    }
    for (int cand = 100000; cand <= 999999; cand++) {
        if (!_exists_ma_the(root, cand)) {
            return cand;
        }
    }
    return 999999; 
}
// Đếm tổng số node (số lượng độc giả) trên cây
inline int dem_tong_doc_gia(DocGiaNode* root) {
    if (root == NULL) return 0;
    return 1 + dem_tong_doc_gia(root->left) + dem_tong_doc_gia(root->right);
}

// ===================== CHUYỂN ĐỔI DỮ LIỆU CÂY -> MẢNG ===========================
// Duyệt cây theo thứ tự LNR (Left-Node-Right) để lấy danh sách đã sắp xếp sơ bộ theo Mã thẻ
// Kết quả lưu vào mảng tĩnh arr[] để phục vụ in ấn hoặc sắp xếp lại theo tên
inline void duyet_LNR_to_array(DocGiaNode* root, DocGia* arr[], int& n) {
    if (root == NULL) return;
    duyet_LNR_to_array(root->left, arr, n);
    if (n < MAX_DOC_GIA) {
        arr[n] = &root->info;
        n++;
    }
    duyet_LNR_to_array(root->right, arr, n);
}

// ================== THUẬT TOÁN SẮP XẾP ==================
// Hoán đổi 2 con trỏ DocGia*
inline void swap_dg(DocGia*& a, DocGia*& b) {
    DocGia* temp = a; a = b; b = temp;
}
// --- 1. Sắp xếp Độc giả theo MÃ THẺ (Tăng dần) ---
inline int partition_dg_mathe(DocGia* arr[], int low, int high) {
    int pivot = arr[high]->maThe;
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (arr[j]->maThe < pivot) {
            i++;
            swap_dg(arr[i], arr[j]);
        }
    }
    swap_dg(arr[i + 1], arr[high]);
    return i + 1;
}
// Hàm chính Quick Sort theo mã thẻ
inline void quick_sort_dg_mathe(DocGia* arr[], int low, int high) {
    if (low < high) {
        int pi = partition_dg_mathe(arr, low, high);
        quick_sort_dg_mathe(arr, low, pi - 1);
        quick_sort_dg_mathe(arr, pi + 1, high);
    }
}
// --- 2. Sắp xếp Độc giả theo TÊN + HỌ (A -> Z) ---
// Ưu tiên: Tên -> Họ -> Mã thẻ
inline int partition_dg_tenho(DocGia* arr[], int low, int high) {
    DocGia* pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; j++) {
        bool condition = false;
        // So sánh Tên trước
        if (arr[j]->ten < pivot->ten) {
            condition = true;        }
        else if (arr[j]->ten == pivot->ten) {
            // Tên trùng thì so sánh Họ
            if (arr[j]->ho < pivot->ho) {
                condition = true;
            }
            // Nếu Tên và Họ đều trùng, so sánh Mã thẻ để ổn định vị trí
            else if (arr[j]->ho == pivot->ho && arr[j]->maThe < pivot->maThe) {
                condition = true;
            }
        }
        if (condition) {
            i++;
            swap_dg(arr[i], arr[j]);
        }
    }
    swap_dg(arr[i + 1], arr[high]);
    return i + 1;
}
// Hàm chính Quick Sort theo TÊN + HỌ
inline void quick_sort_dg_tenho(DocGia* arr[], int low, int high) {
    if (low < high) {
        int pi = partition_dg_tenho(arr, low, high);
        quick_sort_dg_tenho(arr, low, pi - 1);
        quick_sort_dg_tenho(arr, pi + 1, high);
    }
}