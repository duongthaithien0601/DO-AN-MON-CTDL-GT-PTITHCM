#pragma once
#include <string>
#include <random> 
#include <ctime>  
#include "cautruc.h"
#include "dsdms.h"

// ==================== CÁC HÀM KIỂM TRA & TÌM KIẾM CƠ BẢN ======================
// Kiểm tra danh sách đầu sách đã đầy chưa
inline bool is_full(const DanhSachDauSach& ds) {
    return ds.n >= MAX_DAU_SACH;
}
// Tìm con trỏ Đầu Sách dựa trên mã ISBN (Trả về NULL nếu không thấy)
inline DauSach* tim_dau_sach_theo_isbn(const DanhSachDauSach& ds, const std::string& isbn) {
    for (int i = 0; i < ds.n; ++i) {
        if (ds.nodes[i]->ISBN == isbn) {
            return ds.nodes[i];
        }
    }
    return NULL;
}
// Kiểm tra xem ISBN đã tồn tại trong danh sách chưa (trả về true/false)
inline bool is_isbn_exists(const DanhSachDauSach& ds, const std::string& isbn) {
    return tim_dau_sach_theo_isbn(ds, isbn) != NULL;
}

// ================== NGHIỆP VỤ XỬ LÝ ISBN ====================
// Sinh mã ISBN tự động và duy nhất
// Thuật toán: Random -> Nếu trùng nhiều quá thì chuyển sang tìm Max + 1
inline std::string gen_isbn_unique(const DanhSachDauSach& dsArr) {
    static std::mt19937 rng(static_cast<unsigned int>(std::time(NULL)));
    static std::uniform_int_distribution<int> dist(100000000, 999999999);
    // Cách 1: Thử Random ngẫu nhiên (nhanh nếu dữ liệu ít)
    for (int i = 0; i < 5000; i++) {
        int x = dist(rng);
        std::string cand = std::to_string(x);
        if (!is_isbn_exists(dsArr, cand)) {
            return cand;
        }
    }
    // Cách 2: Nếu Random bị trùng quá nhiều (data đầy), tìm số lớn nhất hiện có + 1
    int maxVal = 100000000;
    for (int i = 0; i < dsArr.n; i++) {
        DauSach* ds = dsArr.nodes[i];
        if (ds != NULL && ds->ISBN.length() == 9 && is_all_digits(ds->ISBN)) {
            try {
                int v = std::stoi(ds->ISBN);
                if (v > maxVal) maxVal = v;
            }
            catch (...) {}
        }
    }    
    if (maxVal < 999999999) {
        return std::to_string(maxVal + 1);
    }
    return "999999999"; 
}

// ===================== THÊM / XÓA ĐẦU SÁCH =====================
// Hàm tiện ích: Chuẩn hóa tên sách để so sánh (In hoa, bỏ dấu, cắt khoảng trắng)
inline std::string key_ten_sach(const DauSach* ds) {
    return to_upper_no_accents(trim(ds->tenSach));
}
// Thêm đầu sách mới vào danh sách (Chèn có sắp xếp - Insertion Sort logic)
// Giúp danh sách luôn tăng dần theo tên ngay khi thêm
inline bool chen_dau_sach_sorted_by_ten(DanhSachDauSach& ds, DauSach* p) {
    if (is_full(ds)) return false;
    // 1. Tìm vị trí (k) cần chèn để đảm bảo thứ tự
    int k = 0;
    std::string keyP = key_ten_sach(p);
    while (k < ds.n) {
        std::string keyCur = key_ten_sach(ds.nodes[k]);
        if (keyCur > keyP) break; // Đã tìm thấy vị trí lớn hơn
        k++;
    }
    // 2. Dời các phần tử từ k về sau sang phải 1 bước
    for (int i = ds.n; i > k; i--) {
        ds.nodes[i] = ds.nodes[i - 1];
    }
    // 3. Chèn phần tử mới vào vị trí k
    ds.nodes[k] = p;
    ds.n++;
    return true;
}
// Xóa đầu sách theo ISBN
// Lưu ý: Chỉ xóa được khi không có độc giả nào đang mượn sách này
inline bool xoa_dau_sach(DanhSachDauSach& ds, const std::string& isbn, std::string* outError = NULL) {
    // 1. Tìm vị trí cần xóa
    int idx = -1;
    for (int i = 0; i < ds.n; i++) {
        if (ds.nodes[i]->ISBN == isbn) {
            idx = i; break;
        }
    }
    if (idx == -1) {
        if (outError) *outError = "Khong tim thay ISBN.";
        return false;
    }
    DauSach* p = ds.nodes[idx];
    // 2. Kiểm tra ràng buộc: Sách đang được mượn thì không được xóa
    if (dms_count_borrowed(p) > 0) {
        if (outError) *outError = "Khong the xoa: dang co doc gia muon.";
        return false;
    }
    // 3. Giải phóng bộ nhớ (Danh sách bản sao + Cấu trúc đầu sách)
    dms_free_all(p->dmsHead);
    delete p;
    // 4. Dời các phần tử phía sau về trước để lấp chỗ trống
    for (int i = idx; i < ds.n - 1; i++) {
        ds.nodes[i] = ds.nodes[i + 1];
    }
    ds.n--; // Giảm số lượng phần tử
    return true;
}

// ====================== QUẢN LÝ BẢN SAO ======================
// Tự động tạo 'n' bản sao cho đầu sách (kèm vị trí Kệ - Hàng)
inline void tao_ban_sao_tu_dong(DauSach* ds, int soLuong, const std::string& ke, const std::string& hang) {
    if (ds == NULL || soLuong <= 0) return;
    int startIdx = ds->soLuongBanSao + 1; // Đánh số tiếp theo
    for (int i = 0; i < soLuong; i++) {
        DanhMucSachNode* node = new DanhMucSachNode();
        // Mã sách dạng: ISBN-1, ISBN-2...
        node->maSach = make_masach(ds->ISBN, startIdx + i);
        node->trangThai = BANSAO_CHO_MUON;
        node->viTri = std::string("Ke ") + trim(ke) + std::string(" - Hang ") + trim(hang);
        // Thêm vào cuối DSLK đơn
        dms_append_tail(ds, node);
    }
}
// Lấy vị trí chung của các bản sao (nếu tất cả bản sao cùng chỗ)
inline std::string lay_vi_tri_chung(const DauSach* ds) {
    if (ds == NULL || ds->dmsHead == NULL) return "(chua tao ban sao)";
    const std::string first = ds->dmsHead->viTri;
    // Duyệt xem có bản sao nào khác vị trí đầu tiên không
    for (const DanhMucSachNode* p = ds->dmsHead; p != NULL; p = p->next) {
        if (p->viTri != first) return ""; // Khác nhau -> trả về rỗng
    }
    return first;
}
// Cập nhật vị trí kho cho TOÀN BỘ bản sao
inline void doi_vi_tri_tat_ca_ban_sao(DauSach* ds, const std::string& ke, const std::string& hang) {
    if (ds == NULL) return;
    std::string label = std::string("Ke ") + trim(ke) + std::string(" - Hang ") + trim(hang);
    for (DanhMucSachNode* p = ds->dmsHead; p != NULL; p = p->next) {
        p->viTri = label;
    }
}
// Xóa bớt bản sao (ưu tiên xóa từ cuối lên, bỏ qua sách đang mượn)
inline bool giam_ban_sao_tu_cuoi(DauSach* ds, int soCanXoa) {
    if (ds == NULL || soCanXoa <= 0) return true;
    // 1. Đưa DSLK vào mảng tạm để dễ duyệt ngược 
    DanhMucSachNode* nodes[5000]; // Giới hạn 5000 bản sao
    int count = 0;
    for (DanhMucSachNode* p = ds->dmsHead; p != NULL; p = p->next) {
        if (count < 5000) nodes[count++] = p;
    }
    int daXoa = 0;
    // 2. Duyệt từ cuối về đầu
    for (int i = count - 1; i >= 0 && daXoa < soCanXoa; --i) {
        DanhMucSachNode* cur = nodes[i];
        // Không xóa sách đang được mượn
        if (cur->trangThai == BANSAO_DA_MUON) continue;
        // Xử lý nối dây trước khi xóa
        if (cur == ds->dmsHead) {
            ds->dmsHead = cur->next; // Xóa đầu
        }
        else {
            // Nối node trước đó (i-1) với node sau (next)
            if (i > 0) {
                nodes[i - 1]->next = cur->next;
            }
        }
        delete cur;
        ds->soLuongBanSao--;
        daXoa++;
    }
    return (daXoa == soCanXoa); // Trả về true nếu xóa đủ số lượng yêu cầu
}

// ====================  TÌM KIẾM THEO TÊN ====================
// Tìm kiếm tuyến tính theo tên sách 
inline void tim_dau_sach_theo_ten(const DanhSachDauSach& ds, const std::string& qRaw, DauSach* resultArr[], int& count) {
    count = 0;
    std::string q = to_upper_no_accents(trim(qRaw)); // Chuẩn hóa từ khóa
    if (q.empty()) return;
    for (int i = 0; i < ds.n; i++) {
        std::string key = key_ten_sach(ds.nodes[i]);
        // Kiểm tra chuỗi con (Substring search)
        if (key.find(q) != std::string::npos) {
            resultArr[count++] = ds.nodes[i];
        }
    }
}

// ===================== DỌN DẸP BỘ NHỚ =========================
// Giải phóng toàn bộ mảng đầu sách và các bản sao đi kèm
inline void giai_phong_danh_sach_dausach(DanhSachDauSach& ds) {
    for (int i = 0; i < ds.n; i++) {
        if (ds.nodes[i] != NULL) {
            // Xóa DSLK bản sao trước
            dms_free_all(ds.nodes[i]->dmsHead);
            // Xóa cấu trúc đầu sách
            delete ds.nodes[i];
            ds.nodes[i] = NULL;
        }
    }
    ds.n = 0;
}

// ================ THUẬT TOÁN SẮP XẾP (QUICK SORT) ==================
// Hàm đổi chỗ hai con trỏ đầu sách
inline void swap_ds(DauSach*& a, DauSach*& b) {
    DauSach* temp = a; a = b; b = temp;
}
// --- 1. Sắp xếp theo TÊN SÁCH (A -> Z) ---
inline int partition_ten(DauSach* arr[], int low, int high) {
    std::string pivot = to_upper_no_accents(trim(arr[high]->tenSach));
    int i = low - 1;
    for (int j = low; j < high; j++) {
        // So sánh chuỗi
        if (to_upper_no_accents(trim(arr[j]->tenSach)) < pivot) {
            i++;
            swap_ds(arr[i], arr[j]);
        }
    }
    swap_ds(arr[i + 1], arr[high]);
    return i + 1;
}
// Hàm chính Quick Sort theo TÊN SÁCH
inline void quick_sort_ten(DauSach* arr[], int low, int high) {
    if (low < high) {
        int pi = partition_ten(arr, low, high);
        quick_sort_ten(arr, low, pi - 1);
        quick_sort_ten(arr, pi + 1, high);
    }
}
// --- 2. Sắp xếp theo THỂ LOẠI (Ưu tiên Thể loại -> Tên sách) ---
inline int partition_theloai(DauSach* arr[], int low, int high) {
    DauSach* p = arr[high]; 
    int i = low - 1;
    for (int j = low; j < high; j++) {
        bool condition = false;
        if (arr[j]->theLoai < p->theLoai) condition = true;
        else if (arr[j]->theLoai == p->theLoai && arr[j]->tenSach < p->tenSach) condition = true;
        if (condition) {
            i++;
            swap_ds(arr[i], arr[j]);
        }
    }
    swap_ds(arr[i + 1], arr[high]);
    return i + 1;
}
// Hàm chính Quick Sort theo THỂ LOẠI
inline void quick_sort_theloai(DauSach* arr[], int low, int high) {
    if (low < high) {
        int pi = partition_theloai(arr, low, high);
        quick_sort_theloai(arr, low, pi - 1);
        quick_sort_theloai(arr, pi + 1, high);
    }
}
//Sao chép dữ liệu từ danh sách chính sang mảng tạm rồi sắp xếp
inline void get_sorted_by_theloai(const DanhSachDauSach& src, DauSach* dest[], int& n) {
    n = src.n;
    // Copy con trỏ
    for (int i = 0; i < n; i++) {
        dest[i] = src.nodes[i];
    }
    // Sắp xếp trên mảng copy
    if (n > 0) {
        quick_sort_theloai(dest, 0, n - 1);
    }
}