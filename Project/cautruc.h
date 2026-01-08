#pragma warning(disable : 4996)
#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <sstream>

// ======================= CẤU TRÚC DỮ LIỆU CHÍNH =======================
#define MAX_DOC_GIA 20000
#define MAX_DAU_SACH 1000
enum { HAN_MUON_NGAY = 7 };
enum TrangThaiBanSao {
    BANSAO_CHO_MUON = 0,
    BANSAO_DA_MUON = 1
};
enum TrangThaiMuonTra {
    MT_DANG_MUON = 0,
    MT_DA_TRA = 1,
};
struct Date {
    int d;
    int m;
    int y;
};
//----------------- Danh mục sách -----------------
struct DanhMucSachNode {
    std::string maSach;        // "ISBN-<idx>"
    TrangThaiBanSao trangThai; // cho muon / da muon 
    std::string viTri;         // "Ke A - Hang 1"
    DanhMucSachNode* next;
    DanhMucSachNode() : trangThai(BANSAO_CHO_MUON), next(NULL) {
    }
};
//----------------- Đầu sách -----------------
struct DauSach {
    std::string ISBN;             // 9 chữ số
    std::string tenSach;
    std::string tacGia;
    std::string theLoai;
    int soTrang;
    int namXB;
    int soLuongBanSao;
    int soLuotMuon;
    DanhMucSachNode* dmsHead;
    DauSach()
        : soTrang(0), namXB(0), soLuongBanSao(0),
        soLuotMuon(0), dmsHead(NULL) {
    }
};
//----------------- Danh sách đầu sách  -----------------
struct DanhSachDauSach {
    DauSach* nodes[MAX_DAU_SACH];
    int n;
    DanhSachDauSach() {
        n = 0;
        for (int i = 0; i < MAX_DAU_SACH; i++) nodes[i] = NULL;
    }
};
//----------------- Mượn trả -----------------
struct MuonTraNode {
    std::string maSach;
    Date ngayMuon;
    Date ngayTra;
    TrangThaiMuonTra trangThai;
    MuonTraNode* next;
    MuonTraNode() : trangThai(MT_DANG_MUON), next(NULL) {
    }
};
//----------------- Độc giả -----------------
struct DocGia {
    int maThe;            // 6 chữ số
    std::string ho;
    std::string ten;
    std::string phai;     // "Nam"/"Nu"
    int trangThaiThe;     // 1: hoạt động, 0: khóa
    MuonTraNode* mtHead;
    DocGia() : maThe(0), phai("Nam"), trangThaiThe(1), mtHead(NULL) {
    }
};
//----------------- Cây nhị phân độc giả -----------------
struct DocGiaNode {
    DocGia info;
    DocGiaNode* left;
    DocGiaNode* right;
    explicit DocGiaNode(const DocGia& v) : info(v), left(NULL), right(NULL) {
    }
};
//----------------- Thống kê -----------------
struct TKQuaHanRow {
    int maThe = 0;
    std::string hoTen;
    std::string maSach;
    std::string isbn;
    std::string tenSach;
    Date ngayMuon{};
    int tre = 0; // so ngay tre (>0)
};

// ======================= HÀM TIỆN ÍCH =======================
// Cắt khoảng trắng đầu
inline std::string ltrim_copy(const std::string& s) {
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i])) != 0) {
        i++;
    }
    return s.substr(i);
}
// Cắt khoảng trắng đuôi
inline std::string rtrim_copy(const std::string& s) {
    if (s.empty()) return s;
    size_t i = s.size();
    while (i > 0 && std::isspace(static_cast<unsigned char>(s[i - 1])) != 0) {
        i--;
    }
    return s.substr(0, i);
}
// Cắt 2 đầu
inline std::string trim(const std::string& s) {
    return rtrim_copy(ltrim_copy(s));
}
// Chuyển chữ hoa
inline std::string to_upper_no_accents(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return std::toupper(c);
        });
    return s;
}
// Chuẩn hóa (Trim + Upper)
inline std::string chuan_hoa_str(const std::string& s) {
    return to_upper_no_accents(trim(s));
}
// ----------------- Ngày tháng -----------------
// Kiểm tra tính hợp lệ của ngày tháng
inline bool is_valid_date(const Date& a) {
    if (a.y <= 0 || a.m < 1 || a.m > 12 || a.d <= 0) return false;
    static const int daysInMonth[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (a.m == 2) {
        int maxFeb = 28;
        if ((a.y % 400 == 0) || (a.y % 4 == 0 && a.y % 100 != 0)) maxFeb = 29;
        return a.d <= maxFeb;
    }
    return a.d <= daysInMonth[a.m];
}
// Phân tích chuỗi thành ngày tháng định dạng dd/mm/yyyy
inline bool parse_date_ddmmyyyy(const std::string& s, Date& out) {
    int d, m, y;
    char c1, c2;
    if (std::sscanf(s.c_str(), "%d%c%d%c%d", &d, &c1, &m, &c2, &y) == 5) {
        if (c1 == '/' && c2 == '/') {
            out.d = d; out.m = m; out.y = y;
            return is_valid_date(out);
        }
    }
    return false;
}
// Phân tích chuỗi thành ngày tháng định dạng dd/mm/yyyy (trả về Date)
inline Date parse_date_ddmmyyyy(const std::string& s) {
    Date d{ 0,0,0 };
    (void)parse_date_ddmmyyyy(s, d);
    return d;
}
// Lấy ngày hiện tại
inline int date_to_serial_day(const Date& a) {
    int y = a.y;
    int m = a.m;
    int d = a.d;
    if (m < 3) { y -= 1; m += 12; }
    return 365 * y + y / 4 - y / 100 + y / 400 + (153 * m - 457) / 5 + d - 306;
}
// Tính số ngày giữa 2 ngày
inline int days_between(const Date& a, const Date& b) {
    if (!is_valid_date(a) || !is_valid_date(b)) return 0;
    return date_to_serial_day(b) - date_to_serial_day(a);
}
// Tính số ngày từ a đến b (b - a)
inline int diff_days(const Date& b, const Date& a) {
    return days_between(a, b);
}

// ----------------- Validate -----------------
// Kiểm tra tên (không chứa số và ký tự đặc biệt)
inline bool is_valid_name(const std::string& s) {
    for (unsigned char c : s) {
        if (std::isdigit(c)) return false;
        if (std::ispunct(c)) return false;
    }
    return true;
}
// Kiểm tra chuỗi toàn chữ số
inline bool is_all_digits(const std::string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}
// Kiểm tra chuỗi toàn chữ cái
inline bool is_all_alpha(const std::string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!std::isalpha(static_cast<unsigned char>(c))) return false;
    }
    return true;
}
// ----------------- Mã sách / ISBN -----------------
// Lấy ISBN từ mã sách
inline std::string masach_to_isbn(const std::string& maSach) {
    size_t pos = maSach.find('-');
    if (pos == std::string::npos) return maSach;
    return maSach.substr(0, pos);
}
// Tạo mã sách từ ISBN và chỉ số
inline std::string make_masach(const std::string& isbn, int idx) {
    std::ostringstream oss;
    oss << trim(isbn) << "-" << idx;
    return oss.str();
}