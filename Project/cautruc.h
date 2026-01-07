#pragma warning(disable : 4996)
#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <sstream>
#include <random>

// ======================= CẤU TRÚC DỮ LIỆU CHÍNH =======================
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
struct DanhMucSachNode {
    std::string maSach;        // "ISBN-<idx>"
    TrangThaiBanSao trangThai; // cho muon / da muon 
    std::string viTri;         // "Ke A - Hang 1"
    DanhMucSachNode* next;
    DanhMucSachNode() : trangThai(BANSAO_CHO_MUON), next(NULL) {
    }
};
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
struct MuonTraNode {
    std::string maSach;
    Date ngayMuon;
    Date ngayTra;
    TrangThaiMuonTra trangThai;
    MuonTraNode* next;
    MuonTraNode() : trangThai(MT_DANG_MUON), next(NULL) {
    }
};
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
struct DocGiaNode {
    DocGia info;
    DocGiaNode* left;
    DocGiaNode* right;
    explicit DocGiaNode(const DocGia& v) : info(v), left(NULL), right(NULL) {
    }
};
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
// ----------------- Chuỗi: trim  -----------------
// Cắt bỏ toàn bộ khoảng trắng ở đầu (bên trái)
inline std::string ltrim_copy(const std::string& s) {
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i])) != 0) {
        i++;
    }
    return s.substr(i);
}
// Cắt bỏ toàn bộ khoảng trắng ở cuối (bên phải) của chuỗi
inline std::string rtrim_copy(const std::string& s) {
    if (s.empty()) {
        return s;
    }
    size_t i = s.size();
    while (i > 0 && std::isspace(static_cast<unsigned char>(s[i - 1])) != 0) {
        i--;
    }
    return s.substr(0, i);
}
// Kết hợp cả hai hàm trên để cắt bỏ khoảng trắng ở cả đầu và cuối chuỗi.
inline std::string trim(const std::string& s) {
    return rtrim_copy(ltrim_copy(s));
}
// ----------------- Chuyển đỗi kí tự thành chuỗi in hoa  -----------------
// chuyển đổi các ký tự thường thành ký tự in hoa
inline char _to_upper_ascii(char ch) {
    if (ch >= 'a' && ch <= 'z') {
        return static_cast<char>(ch - 'a' + 'A');
    }
    return ch;
}
// duyệt qua toàn bộ chuỗi và chuyển tất cả các ký tự thành chữ in hoa
inline std::string to_upper_no_accents(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        out.push_back(_to_upper_ascii(s[i]));
    }
    return out;
}
inline std::string chuan_hoa_str(const std::string& s) {
    return to_upper_no_accents(trim(s));
}
// ----------------- Ngày tháng -----------------
// Kiểm tra tính hợp lệ của ngày tháng 
inline bool is_valid_date(const Date& a) {    
    if (a.y <= 0 || a.m < 1 || a.m > 12 || a.d <= 0) {
        return false;
    }    
    static const int daysInMonth[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };    
    if (a.m == 2) {
        int maxFeb = 28;        
        if ((a.y % 400 == 0) || (a.y % 4 == 0 && a.y % 100 != 0)) {
            maxFeb = 29;
        }
        return a.d <= maxFeb;
    }    
    return a.d <= daysInMonth[a.m];
}
// Phân tích chuỗi định dạng "DD/MM/YYYY" thành Date
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
//Phiên bản trả về Date trực tiếp
inline Date parse_date_ddmmyyyy(const std::string& s) {
    Date d{ 0,0,0 };
    (void)parse_date_ddmmyyyy(s, d);
    return d;
}
// Tính số ngày giữa hai ngày (b = ngày sau, a = ngày trước)
inline int date_to_serial_day(const Date& a) {
    int y = a.y;
    int m = a.m;
    int d = a.d;
    if (m < 3) {
        y -= 1;
        m += 12;
    }
    return 365 * y + y / 4 - y / 100 + y / 400 + (153 * m - 457) / 5 + d - 306;
}
// a -> b
inline int days_between(const Date& a, const Date& b) {
    if (!is_valid_date(a) || !is_valid_date(b)) {
        return 0;
    }
    return date_to_serial_day(b) - date_to_serial_day(a);
}
inline int diff_days(const Date& b, const Date& a) {
    return days_between(a, b);
}
// ----------------- Các tiện ích còn lại  -----------------
// Kiểm tra tên hợp lệ (không số, không ký tự đặc biệt)
inline bool is_valid_name(const std::string& s) {
    for (unsigned char c : s) {
        if (std::isdigit(c)) return false;
        if (std::ispunct(c)) return false;
    }
    return true;
}
// Kiểm tra chuỗi chỉ chứa số
inline bool is_all_digits(const std::string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}
// Kiểm tra chuỗi chỉ chứa chữ cái
inline bool is_all_alpha(const std::string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!std::isalpha(static_cast<unsigned char>(c))) return false;
    }
    return true;
}
// Tách ISBN từ "ISBN-idx"
inline std::string masach_to_isbn(const std::string& maSach) {
    size_t pos = maSach.find('-');
    if (pos == std::string::npos) {
        return maSach;
    }
    return maSach.substr(0, pos);
}
// Tạo mã bản sao "ISBN-idx"
inline std::string make_masach(const std::string& isbn, int idx) {
    std::ostringstream oss;
    oss << trim(isbn) << "-" << idx;
    return oss.str();
}
// Kiểm tra ISBN đã tồn tại trong danh sách đầu sách chưa
inline bool is_isbn_exists(const std::vector<DauSach*>& arr, const std::string& isbn);
// Hàm sinh ISBN ngẫu nhiên
inline std::string gen_isbn_unique(const std::vector<DauSach*>& dsArr) {
    static std::mt19937 rng(static_cast<unsigned int>(std::time(NULL)));
    static std::uniform_int_distribution<int> dist(100000000, 999999999);
    for (int i = 0; i < 5000; i++) {
        int x = dist(rng);
        std::string cand = std::to_string(x); 
        if (!is_isbn_exists(dsArr, cand)) {
            return cand;
        }
    }
    int maxVal = 100000000;
    for (const auto* ds : dsArr) {
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
