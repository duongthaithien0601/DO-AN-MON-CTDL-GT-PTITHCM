#pragma once
#include <string>
#include "cautruc.h"
#include "thongke.h" 

// =================== TIỆN ÍCH DSLK ===================
// Tìm bản sao rảnh (trangThai == BANSAO_CHO_MUON) đầu tiên.
inline DanhMucSachNode* dms_find_first_available(DauSach* ds) {
    if (ds == NULL) {
        return NULL;
    }
    for (DanhMucSachNode* p = ds->dmsHead; p != NULL; p = p->next) {
        if (p->trangThai == BANSAO_CHO_MUON) {
            return p;
        }
    }
    return NULL;
}
// Tìm bản sao theo mã sách.
inline DanhMucSachNode* dms_find_by_masach(DauSach* ds, const std::string& maSach) {
    if (ds == NULL) {
        return NULL;
    }
    for (DanhMucSachNode* p = ds->dmsHead; p != NULL; p = p->next) {
        if (p->maSach == maSach) {
            return p;
        }
    }
    return NULL;
}

// =================== THÊM / GỠ BẢN SAO ===================
//Thêm một node vào cuối DSLK, tự tăng soLuongBanSao.
inline void dms_append_tail(DauSach* ds, DanhMucSachNode* node) {
    if (ds == NULL || node == NULL) {
        return;
    }
    node->next = NULL;
    if (ds->dmsHead == NULL) {
        ds->dmsHead = node;
    }
    else {
        DanhMucSachNode* p = ds->dmsHead;
        while (p->next != NULL) {
            p = p->next;
        }
        p->next = node;
    }
    ds->soLuongBanSao++;
}
// Tách (gỡ) một node khỏi DSLK (không giải phóng bộ nhớ).
inline bool dms_detach_node(DauSach* ds, DanhMucSachNode* target) {
    if (ds == NULL || target == NULL) {
        return false;
    }
    DanhMucSachNode* prev = NULL;
    DanhMucSachNode* p = ds->dmsHead;
    while (p != NULL) {
        if (p == target) {
            if (prev == NULL) {
                ds->dmsHead = p->next;
            }
            else {
                prev->next = p->next;
            }
            ds->soLuongBanSao--;
            return true;
        }
        prev = p;
        p = p->next;
    }
    return false;
}
// Giải phóng toàn bộ DSLK. 
inline void dms_free_all(DanhMucSachNode*& head) {
    DanhMucSachNode* p = head;
    while (p != NULL) {
        DanhMucSachNode* nxt = p->next;
        delete p;
        p = nxt;
    }
    head = NULL;
}
// Lấy danh sách mã sách (phục vụ lưu trữ/in bảng).
inline void dms_list_masach(const DauSach* ds, std::string outArr[], int& outN, int maxE = 1000) {
    outN = 0;
    if (ds == NULL) {
        return;
    }
    for (const DanhMucSachNode* p = ds->dmsHead; p != NULL; p = p->next) {
        if (outN < maxE) {
            outArr[outN++] = p->maSach;
        }
    }
}

// =================== ĐÁNH DẤU MƯỢN / TRẢ ===================
// Đánh dấu mượn sách.
inline bool dms_mark_borrowed(DanhMucSachNode* node) {
    if (node == NULL) {
        return false;
    }
    if (node->trangThai != BANSAO_CHO_MUON) {
        return false;
    }
    node->trangThai = BANSAO_DA_MUON;
    return true;
}
//  Đánh dấu trả sách.
inline bool dms_mark_returned(DanhMucSachNode* node) {
    if (node == NULL) {
        return false;
    }
    if (node->trangThai != BANSAO_DA_MUON) {
        return false;
    }
    node->trangThai = BANSAO_CHO_MUON;
    return true;
}

// =================== RÀNG BUỘC TÍNH NHẤT QUÁN ===================
// Đồng bộ lại soLuongBanSao theo DSLK (gọi khi cần).
inline void dms_recount_update(DauSach* ds) {
    if (ds == NULL) {
        return;
    }
    ds->soLuongBanSao = dms_count_total(ds); 
}