#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include "cautruc.h"
#include "dsdms.h"
#include "dsdocgia.h"
#include "dsdausach.h"

// Tra ve ten sach theo ISBN; tra rong neu khong tim thay.
inline std::string ten_sach_theo_isbn(const std::vector<DauSach*>& dsArr, const std::string& isbn) {
    const DauSach* p = tim_dau_sach_theo_isbn(dsArr, isbn);
    if (p == NULL) {
        return std::string();
    }
    return p->tenSach;
}
// Tao & gan mot phieu muon moi vao danh sach cua doc gia (dau danh sach)
inline void them_phieu_muon_cho_doc_gia(DocGia& dg, const std::string& maSach, const Date& ngayMuon) {
    MuonTraNode* node = new MuonTraNode();
    node->maSach = maSach;
    node->trangThai = MT_DANG_MUON;
    node->ngayMuon = ngayMuon;
    node->next = dg.mtHead;
    dg.mtHead = node;
}
// Liet ke cac ban ghi DANG MUON cua 1 doc gia
inline std::vector<MuonTraNode*> list_dang_muon(DocGia& dg) {
    std::vector<MuonTraNode*> v;
    for (MuonTraNode* p = dg.mtHead; p != NULL; p = p->next) {
        if (p->trangThai == MT_DANG_MUON) {
            v.push_back(p);
        }
    }
    return v;
}
// =================== QUÁ HẠN ===================
inline bool doc_gia_co_qua_han_den_ngay(const DocGia& dg, const Date& today, int* outTreMax = NULL) {
    int treMax = 0;
    bool coQuaHan = false;
    for (MuonTraNode* p = dg.mtHead; p != NULL; p = p->next) {
        if (p->trangThai == MT_DANG_MUON) {
            int soNgay = diff_days(today, p->ngayMuon);
            int tre = std::max(0, soNgay - HAN_MUON_NGAY);
            if (tre > 0) {
                coQuaHan = true;
                if (tre > treMax) {
                    treMax = tre;
                }
            }
        }
    }
    if (outTreMax != NULL) {
        *outTreMax = treMax;
    }
    return coQuaHan;
}

// =================== MƯỢN / TRẢ – ===================
inline bool muon_sach(DocGia& dg,
    DauSach& ds,
    const Date& ngayMuon,
    std::string* outMaSachUsed = NULL,
    std::string* outError = NULL) {
    if (dg.trangThaiThe != 1) {
        if (outError != NULL) {
            *outError = "The doc gia dang bi khoa.";
        }
        return false;
    }
    if (dem_mt_dang_muon(dg) >= 3) {
        if (outError != NULL) {
            *outError = "Doc gia da muon toi da 3 cuon.";
        }
        return false;
    }
    int treMax = 0;
    if (doc_gia_co_qua_han_den_ngay(dg, ngayMuon, &treMax)) {
        if (outError != NULL) {
            *outError = "Doc gia dang co sach QUA HAN, khong duoc muon.";
        }
        return false;
    }
    DanhMucSachNode* banSao = dms_find_first_available(&ds);
    if (banSao == NULL) {
        if (outError != NULL) {
            *outError = "Khong con ban sao ranh de muon.";
        }
        return false;
    }
    if (!dms_mark_borrowed(banSao)) {
        if (outError != NULL) {
            *outError = "Khong the danh dau ban sao la DA MUON.";
        }
        return false;
    }
    them_phieu_muon_cho_doc_gia(dg, banSao->maSach, ngayMuon);
    ds.soLuotMuon += 1;
    if (outMaSachUsed != NULL) {
        *outMaSachUsed = banSao->maSach;
    }
    return true;
}
inline bool tra_sach(DocGia& dg,
    std::vector<DauSach*>& dsArr,
    MuonTraNode* target,
    const Date& ngayTra,
    int* outSoNgay = NULL,
    int* outTre = NULL,
    std::string* outError = NULL) {
    if (target == NULL) {
        if (outError != NULL) {
            *outError = "Khong tim thay phieu muon de tra.";
        }
        return false;
    }
    if (target->trangThai != MT_DANG_MUON) {
        if (outError != NULL) {
            *outError = "Phieu muon khong o trang thai DANG MUON.";
        }
        return false;
    }
    // Cap nhat ban ghi muon
    target->trangThai = MT_DA_TRA;
    target->ngayTra = ngayTra;
    // Danh dau ban sao da tra (neu tra cuu duoc)
    const std::string isbn = masach_to_isbn(target->maSach);
    DauSach* ds = tim_dau_sach_theo_isbn(dsArr, isbn);
    if (ds != NULL) {
        DanhMucSachNode* bs = dms_find_by_masach(ds, target->maSach);
        if (bs != NULL) {
            dms_mark_returned(bs);
        }
    }
    // Tinh so ngay muon & tre han
    const int soNgay = diff_days(ngayTra, target->ngayMuon);
    const int tre = std::max(0, soNgay - HAN_MUON_NGAY);
    if (outSoNgay != NULL) {
        *outSoNgay = soNgay;
    }
    if (outTre != NULL) {
        *outTre = tre;
    }
    (void)dg; 

    return true;
}

