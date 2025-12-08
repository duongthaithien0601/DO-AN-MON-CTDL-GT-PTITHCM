#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include "cautruc.h"
#include "dsdocgia.h"
#include "dsdausach.h"

// ========================= THONG KE =========================
inline std::vector<const DauSach*> thongke_top10_theo_luot_muon(const std::vector<DauSach*>& dsArr) {
    std::vector<const DauSach*> a;
    a.reserve(dsArr.size());
    for (const DauSach* ds : dsArr) {
        if (ds != NULL && ds->soLuotMuon > 0) {
            a.push_back(ds);
        }
    }
    std::sort(a.begin(), a.end(), [](const DauSach* L, const DauSach* R) {
        if (L->soLuotMuon != R->soLuotMuon) { return L->soLuotMuon > R->soLuotMuon; }
        if (L->tenSach != R->tenSach) { return L->tenSach < R->tenSach; }
        return L->ISBN < R->ISBN;
        });
    if (a.size() > 10u) { a.resize(10u); }
    return a;
}
inline const DauSach* _tk_find_ds_by_isbn_const(const std::vector<DauSach*>& a, const std::string& isbn) {
    for (auto* ds : a) { if (ds != NULL && ds->ISBN == isbn) { return ds; } }
    return NULL;
}
inline void _tk_collect_qua_han_from_dg(DocGia* dg, const std::vector<DauSach*>& dsArr, const Date& today, std::vector<TKQuaHanRow>& out) {
    for (MuonTraNode* p = dg->mtHead; p != NULL; p = p->next) {
        if (p->trangThai != MT_DANG_MUON) { continue; }
        int soNgay = diff_days(today, p->ngayMuon);
        int tre = soNgay - HAN_MUON_NGAY;
        if (tre <= 0) { continue; }
        std::string isbn = masach_to_isbn(p->maSach);
        const DauSach* ds = _tk_find_ds_by_isbn_const(dsArr, isbn);
        TKQuaHanRow r;
        r.maThe = dg->maThe;
        r.hoTen = trim(dg->ho + " " + dg->ten);
        r.maSach = p->maSach;
        r.isbn = isbn;
        r.tenSach = (ds ? ds->tenSach : "");
        r.ngayMuon = p->ngayMuon;
        r.tre = tre;
        out.push_back(r);
    }
}
inline void _tk_dfs_qua_han(DocGiaNode* root, const std::vector<DauSach*>& dsArr, const Date& today, std::vector<TKQuaHanRow>& out) {
    if (root == NULL) { return; }
    _tk_dfs_qua_han(root->left, dsArr, today, out);
    _tk_collect_qua_han_from_dg(&root->info, dsArr, today, out);
    _tk_dfs_qua_han(root->right, dsArr, today, out);
}
inline std::vector<TKQuaHanRow> thongke_qua_han(DocGiaNode* root, const std::vector<DauSach*>& dsArr, const Date& today) {
    std::vector<TKQuaHanRow> rows;
    rows.reserve(128);
    _tk_dfs_qua_han(root, dsArr, today, rows);
    std::sort(rows.begin(), rows.end(), [](const TKQuaHanRow& a, const TKQuaHanRow& b) {
        if (a.tre != b.tre) { return a.tre > b.tre; }
        if (a.maThe != b.maThe) { return a.maThe < b.maThe; }
        return a.maSach < b.maSach;
        });
    return rows;
}


