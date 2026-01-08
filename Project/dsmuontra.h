#pragma once
#include <string>
#include <algorithm>
#include "cautruc.h"
#include "dsdms.h"
#include "dsdocgia.h"
#include "dsdausach.h"

// ===================== TIỆN ÍCH CƠ BẢN & HỖ TRỢ =========================
// Lấy tên sách dựa vào ISBN (Dùng để hiển thị thông báo nhanh)
inline std::string ten_sach_theo_isbn(const DanhSachDauSach& dsArr, const std::string& isbn) {
    const DauSach* p = tim_dau_sach_theo_isbn(dsArr, isbn);
    if (p == NULL) return "";
    return p->tenSach;
}
// Tạo node mượn trả mới và gắn vào đầu danh sách liên kết của độc giả (Lưu ý: Hành động này chưa kiểm tra điều kiện mượn, chỉ đơn thuần là thêm dữ liệu)
inline void them_phieu_muon_cho_doc_gia(DocGia& dg, const std::string& maSach, const Date& ngayMuon) {
    MuonTraNode* node = new MuonTraNode();
    node->maSach = maSach;
    node->trangThai = MT_DANG_MUON;
    node->ngayMuon = ngayMuon;
    // Thêm vào đầu danh sách (LIFO)
    node->next = dg.mtHead;
    dg.mtHead = node;
}
// Trích xuất danh sách các sách ĐANG MƯỢN ra mảng (để hiển thị lên bảng)
inline void list_dang_muon(DocGia& dg, MuonTraNode* outArr[], int& outN) {
    outN = 0;
    for (MuonTraNode* p = dg.mtHead; p != NULL; p = p->next) {
        if (p->trangThai == MT_DANG_MUON) {
            outArr[outN++] = p;
        }
    }
}

// ========== KIỂM TRA TÌNH TRẠNG SÁCH TRÊN TOÀN HỆ THỐNG ====================
// Kiểm tra xem một đầu sách (ISBN) có đang được độc giả nào mượn không
// Mục đích: Chặn xóa đầu sách nếu đang có người mượn
inline int count_borrowed_by_isbn(DocGiaNode* root, const std::string& isbn) {
    int cnt = 0;
    // 1. Chuyển cây độc giả sang mảng tuyến tính để dễ duyệt
    static DocGia* arr[MAX_DOC_GIA]; 
    int n = 0;
    duyet_LNR_to_array(root, arr, n);
    // 2. Duyệt qua từng độc giả
    for (int i = 0; i < n; i++) {
        DocGia* dg = arr[i];
        // Duyệt qua danh sách mượn trả của từng người
        for (MuonTraNode* p = dg->mtHead; p; p = p->next) {
            // Nếu đang mượn VÀ đúng ISBN cần tìm
            if (p->trangThai == MT_DANG_MUON && masach_to_isbn(p->maSach) == isbn) {
                cnt++;
            }
        }
    }
    return cnt;
}

// ===================== KIỂM TRA QUÁ HẠN =======================
// Kiểm tra độc giả có giữ sách quá hạn không
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

// =================== MƯỢN / TRẢ ==================
// Xử lý MƯỢN SÁCH
// Trả về true nếu mượn thành công, false nếu vi phạm quy định
inline bool muon_sach(DocGia& dg,
    DauSach& ds,
    const Date& ngayMuon,
    std::string* outMaSachUsed = NULL,
    std::string* outError = NULL) {
    // 1. Kiểm tra trạng thái thẻ
    if (dg.trangThaiThe != 1) {
        if (outError != NULL) *outError = "The doc gia dang bi khoa.";
        return false;
    }
    // 2. Kiểm tra số lượng sách đang mượn (Tối đa 3 cuốn)
    if (dem_mt_dang_muon(dg) >= 3) {
        if (outError != NULL) *outError = "Doc gia da muon toi da 3 cuon.";
        return false;
    }
    // 3. Kiểm tra sách quá hạn
    int treMax = 0;
    if (doc_gia_co_qua_han_den_ngay(dg, ngayMuon, &treMax)) {
        if (outError != NULL) *outError = "Doc gia dang co sach QUA HAN (" + std::to_string(treMax) + " ngay), khong duoc muon.";
        return false;
    }
    // 4. Tìm bản sao còn trong kho
    DanhMucSachNode* banSao = dms_find_first_available(&ds);
    if (banSao == NULL) {
        if (outError != NULL) *outError = "Khong con ban sao ranh de muon.";
        return false;
    }
    // 5. Cập nhật trạng thái bản sao -> ĐÃ MƯỢN
    if (!dms_mark_borrowed(banSao)) {
        if (outError != NULL) *outError = "Loi he thong: Khong the danh dau ban sao.";
        return false;
    }
    // 6. Ghi nhận phiếu mượn vào hồ sơ độc giả
    them_phieu_muon_cho_doc_gia(dg, banSao->maSach, ngayMuon);
    ds.soLuotMuon += 1; // Tăng thống kê lượt mượn
    // Trả về mã sách vừa mượn
    if (outMaSachUsed != NULL) {
        *outMaSachUsed = banSao->maSach;
    }
    return true;
}
// Xử lý TRẢ SÁCH
inline bool tra_sach(DocGia& dg,
    DanhSachDauSach& dsArr,
    MuonTraNode* target,
    const Date& ngayTra,
    int* outSoNgay = NULL,
    int* outTre = NULL,
    std::string* outError = NULL) {
    // 1. Kiểm tra tính hợp lệ của phiếu mượn
    if (target == NULL) {
        if (outError) *outError = "Khong tim thay phieu muon.";
        return false;
    }
    if (target->trangThai != MT_DANG_MUON) {
        if (outError) *outError = "Phieu khong o trang thai DANG MUON.";
        return false;
    }
    // 2. Cập nhật trạng thái phiếu -> ĐÃ TRẢ
    target->trangThai = MT_DA_TRA;
    target->ngayTra = ngayTra;
    // 3. Cập nhật trạng thái bản sao trong kho -> CHO MƯỢN
    const std::string isbn = masach_to_isbn(target->maSach);
    DauSach* ds = tim_dau_sach_theo_isbn(dsArr, isbn);
    if (ds != NULL) {
        DanhMucSachNode* bs = dms_find_by_masach(ds, target->maSach);
        if (bs != NULL) {
            dms_mark_returned(bs);
        }
    }
    // 4. Tính toán số liệu (Số ngày mượn, Số ngày trễ)
    const int soNgay = diff_days(ngayTra, target->ngayMuon);
    const int tre = std::max(0, soNgay - HAN_MUON_NGAY);
    if (outSoNgay) *outSoNgay = soNgay;
    if (outTre) *outTre = tre;
    (void)dg; 
    return true;
}

