#pragma once
#include "cautruc.h"
#include "dsdocgia.h"

// =================== CÁC HÀM THỐNG KÊ ===================
// Đếm thống kê tổng số bản sao (node) trong DMS của một đầu sách.
inline int dms_count_total(const DauSach* ds) {
    if (ds == NULL) {
        return 0;
    }
    int dem = 0;
    for (const DanhMucSachNode* p = ds->dmsHead; p != NULL; p = p->next) {
        dem++;
    }
    return dem;
}
// Đếm thống kê  bản sao đang được mượn.
inline int dms_count_borrowed(const DauSach* ds) {
    if (ds == NULL) {
        return 0;
    }
    int dem = 0;
    for (const DanhMucSachNode* p = ds->dmsHead; p != NULL; p = p->next) {
        if (p->trangThai == BANSAO_DA_MUON) {
            dem++;
        }
    }
    return dem;
}

// =================== THỐNG KÊ TOP 10 SÁCH MƯỢN NHIỀU NHẤT ===================
inline void thongke_top10_theo_luot_muon(const DanhSachDauSach& dsArr, DauSach* outArr[], int& outN) {
    outN = 0;
    // 1. Lọc sách có lượt mượn > 0
    for (int i = 0; i < dsArr.n; i++) {
        DauSach* ds = dsArr.nodes[i];
        if (ds != NULL && ds->soLuotMuon > 0) {
            outArr[outN++] = ds;
        }
    }
    // 2. Sắp xếp Bubble Sort (Giảm dần theo soLuotMuon)
    for (int i = 0; i < outN - 1; i++) {
        for (int j = i + 1; j < outN; j++) {
            bool doSwap = false;
            if (outArr[j]->soLuotMuon > outArr[i]->soLuotMuon) {
                doSwap = true;
            }
            else if (outArr[j]->soLuotMuon == outArr[i]->soLuotMuon) {
                if (outArr[j]->tenSach < outArr[i]->tenSach) doSwap = true;
            }
            if (doSwap) {
                DauSach* tmp = outArr[i];
                outArr[i] = outArr[j];
                outArr[j] = tmp;
            }
        }
    }
    // 3. Cắt lấy 10 phần tử
    if (outN > 10) outN = 10;
}

// =================== THỐNG KÊ ĐỘC GIẢ QUÁ HẠN ===================
// Hàm phụ: Tìm đầu sách trong mảng tĩnh (const)
inline const DauSach* _tk_find_ds_const(const DanhSachDauSach& dsArr, const std::string& isbn) {
    for (int i = 0; i < dsArr.n; i++) {
        if (dsArr.nodes[i]->ISBN == isbn) return dsArr.nodes[i];
    }
    return NULL;
}
// Hàm phụ: Duyệt cây (DFS) để thu thập dữ liệu vào mảng outRows
inline void _tk_dfs_qua_han(DocGiaNode* root, const DanhSachDauSach& dsArr, const Date& today,
    TKQuaHanRow outRows[], int& count, int maxRow) {
    if (root == NULL || count >= maxRow) { return; }
    // Duyệt trái (L)
    _tk_dfs_qua_han(root->left, dsArr, today, outRows, count, maxRow);
    // Xử lý nút hiện tại (N)
    DocGia* dg = &root->info;
    for (MuonTraNode* p = dg->mtHead; p != NULL; p = p->next) {
        // Chỉ xét sách ĐANG MƯỢN
        if (p->trangThai != MT_DANG_MUON) continue;
        int soNgay = diff_days(today, p->ngayMuon);
        int tre = soNgay - HAN_MUON_NGAY;
        // Nếu quá hạn
        if (tre > 0) {
            if (count >= maxRow) return;// Đầy mảng thì dừng
            std::string isbn = masach_to_isbn(p->maSach);
            const DauSach* ds = _tk_find_ds_const(dsArr, isbn);
            // Điền dữ liệu vào mảng tại vị trí count
            outRows[count].maThe = dg->maThe;
            outRows[count].hoTen = trim(dg->ho + " " + dg->ten);
            outRows[count].maSach = p->maSach;
            outRows[count].isbn = isbn;
            outRows[count].tenSach = (ds ? ds->tenSach : "");
            outRows[count].ngayMuon = p->ngayMuon;
            outRows[count].tre = tre;
            count++; // Tăng biến đếm
        }
    }
    // Duyệt phải (R)
    _tk_dfs_qua_han(root->right, dsArr, today, outRows, count, maxRow);
}
// Hàm chính: Thống kê quá hạn
inline void thongke_qua_han(DocGiaNode* root, const DanhSachDauSach& dsArr, const Date& today,
    TKQuaHanRow outRows[], int& outN, int maxRow = 500) {
    outN = 0;
    // 1. Thu thập dữ liệu
    _tk_dfs_qua_han(root, dsArr, today, outRows, outN, maxRow);
    // 2. Sắp xếp thủ công (Bubble Sort)
    for (int i = 0; i < outN - 1; i++) {
        for (int j = i + 1; j < outN; j++) {
            bool swap = false;
            if (outRows[j].tre > outRows[i].tre) {
                swap = true;
            }
            else if (outRows[j].tre == outRows[i].tre) {
                if (outRows[j].maThe < outRows[i].maThe) {
                    swap = true;
                }
                else if (outRows[j].maThe == outRows[i].maThe) {
                    if (outRows[j].maSach < outRows[i].maSach) {
                        swap = true;
                    }
                }
            }
            if (swap) {
                TKQuaHanRow tmp = outRows[i];
                outRows[i] = outRows[j];
                outRows[j] = tmp;
            }
        }
    }
}