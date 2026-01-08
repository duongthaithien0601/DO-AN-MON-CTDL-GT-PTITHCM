#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdio>
#include "cautruc.h"
#include "dsdms.h"
#include "dsdausach.h"
#include "dsdocgia.h"
#ifdef _WIN32
#include <direct.h>  
#else
#include <sys/stat.h> 
#include <sys/types.h>
#endif

// ========================= ĐƯỜNG DẪN TỆP DỮ LIỆU =========================
inline std::string path_data_dir() { return std::string("data"); }
inline std::string path_file_dausach() { return path_data_dir() + "/dausach.txt"; }
inline std::string path_file_dms() { return path_data_dir() + "/dms.txt"; }
inline std::string path_file_docgia() { return path_data_dir() + "/docgia.txt"; }
inline std::string path_file_muontra() { return path_data_dir() + "/muontra.txt"; }
inline bool ensure_data_dir() {
#ifdef _WIN32
    int r = _mkdir(path_data_dir().c_str()); (void)r;
#else
    mkdir(path_data_dir().c_str(), 0755);
#endif
    return true;
}

//========================= TIỆN ÍCH XỬ LÝ CHUỖI =========================
// Tách chuỗi theo ký tự '|'
inline std::vector<std::string> split_bar(const std::string& line) {
    std::vector<std::string> out; std::string cur;
    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        if (c == '|') { out.push_back(cur); cur.clear(); }
        else { cur.push_back(c); }
    }
    out.push_back(cur);
    return out;
}
// Nối chuỗi với ký tự '|'
inline std::string join_bar(const std::vector<std::string>& v) {
    std::ostringstream oss;
    for (size_t i = 0; i < v.size(); i++) {
        if (i > 0) { oss << '|'; }
        oss << v[i];
    }
    return oss.str();
}
template <typename T>
inline std::string to_str(T v) { std::ostringstream oss; oss << v; return oss.str(); }

// ========================= TIỆN ÍCH XỬ LÝ NGÀY THÁNG =========================
// Kiểm tra tính hợp lệ của ngày tháng
inline std::string date_to_string(const Date& d) {
    if (!is_valid_date(d)) { return std::string("0/0/0"); }
    std::ostringstream oss;
    if (d.d < 10) { oss << '0'; } oss << d.d << '/';
    if (d.m < 10) { oss << '0'; } oss << d.m << '/';
    oss << d.y; return oss.str();
}
// Phân tích chuỗi thành ngày tháng
inline bool parse_date_token(const std::string& s, Date& out) {
    if (s.empty()) { out = Date{ 0,0,0 }; return true; }
    int a = 0, b = 0, c = 0; char sep1 = 0, sep2 = 0;
    std::istringstream iss(s);
    if ((iss >> a >> sep1 >> b >> sep2 >> c)) {
        if (sep1 == '/' && sep2 == '/') { out.d = a; out.m = b; out.y = c; return is_valid_date(out); }
        if (sep1 == '-' && sep2 == '-') { out.y = a; out.m = b; out.d = c; return is_valid_date(out); }
    }
    if (s.size() == 8) {
        out.d = std::atoi(s.substr(0, 2).c_str());
        out.m = std::atoi(s.substr(2, 2).c_str());
        out.y = std::atoi(s.substr(4, 4).c_str());
        return is_valid_date(out);
    }
    return false;
}

//========================= GIẢI PHÓNG BỘ NHỚ =========================
// ----------------- Danh mục sách -----------------
inline void giai_phong_dms(DanhMucSachNode*& head) { dms_free_all(head); }
// ----------------- Đầu sách -----------------
inline void giai_phong_vector_dausach(std::vector<DauSach*>& dsArr) {
    for (size_t i = 0; i < dsArr.size(); i++) {
        DauSach* ds = dsArr[i];
        if (ds != NULL) {
            giai_phong_dms(ds->dmsHead);
            delete ds;
        }
    }
    dsArr.clear();
}
// ----------------- Độc giả (BST) -----------------
inline void giai_phong_ds_muontra(MuonTraNode*& head) {
    MuonTraNode* p = head;
    while (p != NULL) { MuonTraNode* del = p; p = p->next; delete del; }
    head = NULL;
}
// Giải phóng toàn bộ cây BST độc giả
inline void giai_phong_cay_doc_gia(DocGiaNode*& root) {
    if (root == NULL) { return; }
    giai_phong_cay_doc_gia(root->left);
    giai_phong_cay_doc_gia(root->right);
    giai_phong_ds_muontra(root->info.mtHead);
    delete root; root = NULL;
}

// ========================= LƯU / ĐỌC: ĐẦU SÁCH =========================
// Lưu danh sách đầu sách
inline bool save_dau_sach(const DanhSachDauSach& dsArr) {
    ensure_data_dir();
    std::ofstream fo(path_file_dausach().c_str());
    if (!fo) { return false; }
    // Duyệt mảng tĩnh
    for (int i = 0; i < dsArr.n; i++) {
        const DauSach* ds = dsArr.nodes[i];
        if (ds == NULL) { continue; }
        // Ghi trực tiếp không dùng vector nối chuỗi
        fo << ds->ISBN << "|"
            << ds->tenSach << "|"
            << ds->tacGia << "|"
            << ds->theLoai << "|"
            << ds->soTrang << "|"
            << ds->namXB << "|"
            << ds->soLuongBanSao << "|"
            << ds->soLuotMuon << "\n";
    }
    return true;
}
// Đọc danh sách đầu sách
inline bool load_dau_sach(DanhSachDauSach& dsArr) {
    dsArr.n = 0; 
    std::ifstream fi(path_file_dausach().c_str());
    if (!fi) { return true; }
    std::string line;
    while (std::getline(fi, line)) {
        if (line.empty()) continue;
        std::string cols[8];
        int colIdx = 0;
        std::string cur = "";
        for (char c : line) {
            if (c == '|') {
                if (colIdx < 8) cols[colIdx++] = cur;
                cur = "";
            }
            else {
                cur += c;
            }
        }
        if (colIdx < 8) cols[colIdx++] = cur; 

        if (colIdx < 8) continue; 
        DauSach* ds = new DauSach();
        ds->ISBN = cols[0];
        ds->tenSach = cols[1];
        ds->tacGia = cols[2];
        ds->theLoai = cols[3];
        ds->soTrang = std::atoi(cols[4].c_str());
        ds->namXB = std::atoi(cols[5].c_str());
        ds->soLuongBanSao = std::atoi(cols[6].c_str());
        ds->soLuotMuon = std::atoi(cols[7].c_str());
        ds->dmsHead = NULL;
        // Thêm vào mảng tĩnh
        if (!is_full(dsArr)) {
            dsArr.nodes[dsArr.n++] = ds;
        }
    }
    return true;
}

// ========================= LƯU / ĐỌC: DANH MỤC SÁCH =========================
// 1. Lưu danh mục sách 
inline bool save_dms(const DanhSachDauSach& dsArr) {
    ensure_data_dir();
    std::ofstream fo(path_file_dms().c_str());
    if (!fo) { return false; }
    // Duyệt mảng tĩnh nodes từ 0 đến n
    for (int i = 0; i < dsArr.n; i++) {
        const DauSach* ds = dsArr.nodes[i];
        if (ds == NULL) { continue; }
        DanhMucSachNode* p = ds->dmsHead;
        while (p != NULL) {            
            fo << ds->ISBN << "|"
                << p->maSach << "|"
                << (int)p->trangThai << "|"
                << p->viTri << "\n";

            p = p->next;
        }
    }
    return true;
}
// 2. Đọc danh mục sách 
inline bool load_dms(DanhSachDauSach& dsArr) {
    std::ifstream fi(path_file_dms().c_str());
    if (!fi) { return true; } // File chưa tồn tại thì coi như rỗng, không lỗi
    std::string line;
    while (std::getline(fi, line)) {
        if (line.empty()) continue;
        size_t p1 = line.find('|');
        if (p1 == std::string::npos) continue;
        size_t p2 = line.find('|', p1 + 1);
        if (p2 == std::string::npos) continue;
        size_t p3 = line.find('|', p2 + 1);
        if (p3 == std::string::npos) continue;
        std::string isbn = line.substr(0, p1);
        std::string maSach = line.substr(p1 + 1, p2 - p1 - 1);
        std::string sTrangThai = line.substr(p2 + 1, p3 - p2 - 1);
        std::string viTri = line.substr(p3 + 1);
        // Tìm đầu sách trong danh sách đã load
        DauSach* ds = tim_dau_sach_theo_isbn(dsArr, isbn);
        if (ds == NULL) { continue; } // Nếu không thấy ISBN tương ứng thì bỏ qua
        // Tạo node bản sao
        int tt = std::atoi(sTrangThai.c_str());
        TrangThaiBanSao t = (tt == BANSAO_DA_MUON ? BANSAO_DA_MUON : BANSAO_CHO_MUON);
        DanhMucSachNode* node = new DanhMucSachNode();
        node->maSach = maSach;
        node->trangThai = t;
        node->viTri = viTri;
        // Thêm vào DSLK đơn
        dms_append_tail(ds, node);
    }
    return true;
}

// ========================= LƯU / ĐỌC: ĐỘC GIẢ (BST) =========================
// Lưu cây BST độc giả 
inline void _save_doc_gia_inorder(std::ofstream& fo, DocGiaNode* root) {
    if (root == NULL) { return; }
    _save_doc_gia_inorder(fo, root->left);
    {
        const DocGia& dg = root->info;
        std::vector<std::string> cols;
        cols.push_back(to_str(dg.maThe));
        cols.push_back(dg.ho);
        cols.push_back(dg.ten);
        cols.push_back(dg.phai);
        cols.push_back(to_str(dg.trangThaiThe));
        fo << join_bar(cols) << "\n";
    }
    _save_doc_gia_inorder(fo, root->right);
}
// Đọc cây BST độc giả
inline bool save_doc_gia(DocGiaNode* root) {
    ensure_data_dir();
    std::ofstream fo(path_file_docgia().c_str());
    if (!fo) { return false; }
    _save_doc_gia_inorder(fo, root);
    return true;
}
// Đọc cây BST độc giả
inline bool load_doc_gia(DocGiaNode*& root) {
    root = NULL;
    std::ifstream fi(path_file_docgia().c_str());
    if (!fi) { return true; }
    std::string line;
    while (std::getline(fi, line)) {
        std::vector<std::string> cols = split_bar(line);
        if (cols.size() < 5) { continue; }
        DocGia dg;
        dg.maThe = std::atoi(cols[0].c_str());
        dg.ho = cols[1];
        dg.ten = cols[2];
        std::string phaiTok = trim(cols[3]);
        if (phaiTok == "0") { dg.phai = "Nam"; }
        else if (phaiTok == "1") { dg.phai = "Nu"; }
        else { dg.phai = phaiTok; }
        int raw = std::atoi(cols[4].c_str());
        dg.trangThaiThe = (raw != 0 ? 1 : 0);
        dg.mtHead = NULL;
        insert_doc_gia(root, dg);
    }
    return true;
}

// ========================= LƯU / ĐỌC: MƯỢN TRẢ SÁCH =========================
// Lưu danh sách mượn trả sách
inline void _duyet_save_muon_tra(std::ofstream& fo, DocGiaNode* root) {
    if (root == NULL) { return; }
    _duyet_save_muon_tra(fo, root->left);
    {
        const DocGia& dg = root->info;
        MuonTraNode* p = dg.mtHead;
        while (p != NULL) {
            std::vector<std::string> cols;
            cols.push_back(to_str(dg.maThe));
            cols.push_back(p->maSach);
            cols.push_back(date_to_string(p->ngayMuon));
            cols.push_back(date_to_string(p->ngayTra));
            cols.push_back(to_str(static_cast<int>(p->trangThai)));
            fo << join_bar(cols) << "\n";
            p = p->next;
        }
    }
    _duyet_save_muon_tra(fo, root->right);
}
// Đọc danh sách mượn trả sách
inline bool save_muon_tra(DocGiaNode* root) {
    ensure_data_dir();
    std::ofstream fo(path_file_muontra().c_str());
    if (!fo) { return false; }
    _duyet_save_muon_tra(fo, root);
    return true;
}
// Đọc danh sách mượn trả sách
inline bool load_muon_tra(const DanhSachDauSach& dsArr, DocGiaNode*& root) {
    std::ifstream fi(path_file_muontra().c_str());
    if (!fi) { return true; }
    std::string line;
    while (std::getline(fi, line)) {
        if (line.empty()) continue;
        // Tách chuỗi thủ công
        std::string cols[5];
        int colIdx = 0;
        std::string cur = "";
        for (char c : line) {
            if (c == '|') {
                if (colIdx < 5) cols[colIdx++] = cur;
                cur = "";
            }
            else {
                cur += c;
            }
        }
        if (colIdx < 5) cols[colIdx++] = cur;
        if (colIdx < 5) continue;
        int maThe = std::atoi(cols[0].c_str());
        std::string maSach = cols[1];
        Date ngayMuon{ 0,0,0 }, ngayTra{ 0,0,0 };
        parse_date_token(cols[2], ngayMuon);
        parse_date_token(cols[3], ngayTra);
        int tt = std::atoi(cols[4].c_str());
        DocGiaNode* dgNode = tim_node_doc_gia(root, maThe);
        if (dgNode == NULL) { continue; }
        MuonTraNode* node = new MuonTraNode();
        node->maSach = maSach;
        node->ngayMuon = ngayMuon;
        node->ngayTra = ngayTra;
        node->trangThai = static_cast<TrangThaiMuonTra>(tt);
        node->next = dgNode->info.mtHead;
        dgNode->info.mtHead = node;       
        DauSach* ds = tim_dau_sach_theo_isbn(dsArr, masach_to_isbn(maSach));
        if (ds != NULL) {
            DanhMucSachNode* copy = dms_find_by_masach(ds, maSach);
            if (copy != NULL) {
                if (node->trangThai == MT_DANG_MUON) {
                    copy->trangThai = BANSAO_DA_MUON;
                }
                else {
                    copy->trangThai = BANSAO_CHO_MUON;
                }
            }
        }
    }
    return true;
}

// ========================= LƯU / ĐỌC: TẤT CẢ DỮ LIỆU =========================
// Lưu tất cả dữ liệu
inline bool save_all_data(const DanhSachDauSach& dsArr, DocGiaNode* root) {
    bool ok1 = save_dau_sach(dsArr);
    bool ok2 = save_dms(dsArr);
    bool ok3 = save_doc_gia(root);
    bool ok4 = save_muon_tra(root);
    return ok1 && ok2 && ok3 && ok4;
}
inline bool load_all_data(DanhSachDauSach& dsArr, DocGiaNode*& root) {   
    giai_phong_danh_sach_dausach(dsArr);
    giai_phong_cay_doc_gia(root);
    bool ok1 = load_dau_sach(dsArr);
    bool ok2 = load_dms(dsArr);
    for (int i = 0; i < dsArr.n; ++i) {
        dms_recount_update(dsArr.nodes[i]);
    }
    bool ok3 = load_doc_gia(root);
    bool ok4 = load_muon_tra(dsArr, root);
    return ok1 && ok2 && ok3 && ok4;
}