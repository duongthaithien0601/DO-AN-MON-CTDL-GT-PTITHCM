#pragma once
#include "cautruc.h"
#include "dsdms.h"
#include <vector>
#include <string>
#include <algorithm>

//================== KIỂM TRA TỒN TẠI THEO ISBN ==================
inline bool is_isbn_exists(const std::vector<DauSach*>& arr, const std::string& isbn) {
    for (size_t i = 0; i < arr.size(); ++i) {
        if (arr[i] != NULL && arr[i]->ISBN == isbn) {
            return true;
        }
    }
    return false;
}

//================== TÌM ĐẦU SÁCH THEO ISBN ==================
inline DauSach* tim_dau_sach_theo_isbn(const std::vector<DauSach*>& arr, const std::string& isbn) {
    for (size_t i = 0; i < arr.size(); i++) {
        if (arr[i] != NULL && arr[i]->ISBN == isbn) {
            return arr[i];
        }
    }
    return NULL;
}
// ================== CHÈN ĐẦU SÁCH THEO TÊN SÁCH ==================
inline std::string key_ten_sach(const DauSach* ds) {
    return to_upper_no_accents(trim(ds->tenSach));
}
//Chèn dau sach p vào arr sao cho arr vẫn được sắp xếp theo tên sách tăng dần
inline void chen_dau_sach_sorted_by_ten(std::vector<DauSach*>& arr, DauSach* p) {
    std::string keyP = key_ten_sach(p);
    std::vector<DauSach*>::iterator it = arr.begin();
    for (; it != arr.end(); ++it) {
        if (*it == NULL) {
            continue;
        }
        if (key_ten_sach(*it) > keyP) {
            break;
        }
    }
    arr.insert(it, p);
}

// ================== TẠO BẢN SAO TỰ ĐỘNG ==================
inline void tao_ban_sao_tu_dong(DauSach* ds, int soLuong, const std::string& ke, const std::string& hang) {
    if (ds == NULL || soLuong <= 0) {
        return;
    }
    int startIdx = ds->soLuongBanSao + 1;
    for (int i = 0; i < soLuong; i++) {
        DanhMucSachNode* node = new DanhMucSachNode();
        node->maSach = make_masach(ds->ISBN, startIdx + i);
        node->trangThai = BANSAO_CHO_MUON;
        node->viTri = std::string("Ke ") + trim(ke) + std::string(" - Hang ") + trim(hang);
        dms_append_tail(ds, node);
    }
}

// ====== VỊ TRÍ CHUNG & ĐỔI VỊ TRÍ TOÀN BỘ ======
inline std::string lay_vi_tri_chung(const DauSach* ds) {
    if (ds == NULL || ds->dmsHead == NULL) {
        return "(chua tao ban sao)";
    }
    const std::string first = ds->dmsHead->viTri;
    for (const DanhMucSachNode* p = ds->dmsHead; p != NULL; p = p->next) {
        if (p->viTri != first) {
            return ""; 
        }
    }
    return first;
}
//Đặt vị trí cho tất cả bản sao.
inline void doi_vi_tri_tat_ca_ban_sao(DauSach* ds, const std::string& ke, const std::string& hang) {
    if (ds == NULL) {
        return;
    }
    std::string label = std::string("Ke ") + trim(ke) + std::string(" - Hang ") + trim(hang);
    for (DanhMucSachNode* p = ds->dmsHead; p != NULL; p = p->next) {
        p->viTri = label;
    }
}

// ====== GIẢM SỐ LƯỢNG ======
inline bool giam_ban_sao_tu_cuoi(DauSach* ds, int soCanXoa) {
    if (ds == NULL || soCanXoa <= 0) {
        return true;
    }
    std::vector<DanhMucSachNode*> nodes;
    for (DanhMucSachNode* p = ds->dmsHead; p != NULL; p = p->next) {
        nodes.push_back(p);
    }
    int daXoa = 0;
    for (int i = static_cast<int>(nodes.size()) - 1; i >= 0 && daXoa < soCanXoa; --i) {
        DanhMucSachNode* cur = nodes[static_cast<size_t>(i)];
        if (cur->trangThai == BANSAO_DA_MUON) {
            continue; // khong duoc xoa ban sao dang muon
        }
        DanhMucSachNode* prev = NULL;
        if (i > 0) {
            prev = nodes[static_cast<size_t>(i - 1)];
        }
        if (prev == NULL) {
            ds->dmsHead = cur->next;
        }
        else {
            prev->next = cur->next;
        }
        delete cur;
        ds->soLuongBanSao--;
        daXoa++;
    }
    return (daXoa == soCanXoa);
}

// =================== XOA DAU SACH ===================
// Xoa dau sach theo ISBN neu khong con ban sao dang cho muon.
inline bool xoa_dau_sach(std::vector<DauSach*>& arr, const std::string& isbn, std::string* outError = NULL) {
    DauSach* ds = tim_dau_sach_theo_isbn(arr, isbn);
    if (ds == NULL) {
        if (outError != NULL) {
            *outError = "Khong tim thay dau sach voi ISBN da nhap.";
        }
        return false;
    }
    // Khong duoc xoa neu co ban sao dang cho muon
    int borrowed = dms_count_borrowed(ds);
    if (borrowed > 0) {
        if (outError != NULL) {
            *outError = std::string("Khong the xoa: con ") + std::to_string(borrowed) + std::string(" ban sao dang cho muon.");
        }
        return false;
    }
    // Giai phong DSLK danh muc sach
    dms_free_all(ds->dmsHead);
    // Xoa khoi arr
    for (size_t i = 0; i < arr.size(); ++i) {
        if (arr[i] == ds) {
            delete arr[i];
            arr.erase(arr.begin() + static_cast<long long>(i));
            return true;
        }
    }
    if (outError != NULL) {
        *outError = "Loi noi bo: khong the xoa khoi danh sach.";
    }
    return false;
}

// ================== TIM DAU SACH THEO TEN ==================
inline std::vector<DauSach*> tim_dau_sach_theo_ten(const std::vector<DauSach*>& arr, const std::string& qRaw) {
    std::vector<DauSach*> res;
    std::string q = to_upper_no_accents(trim(qRaw));
    if (q.empty()) {
        return res;
    }
    for (DauSach* ds : arr) {
        if (ds == NULL) {
            continue;
        }
        std::string key = key_ten_sach(ds);
        if (key.find(q) != std::string::npos) {
            res.push_back(ds);
        }
    }
    std::sort(res.begin(), res.end(), [](DauSach* a, DauSach* b) {
        if (a->tenSach != b->tenSach) {
            return a->tenSach < b->tenSach;
        }
        return a->ISBN < b->ISBN;
        });
    return res;
}

