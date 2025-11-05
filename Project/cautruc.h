#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <sstream>
#include <random>

// ======================= HẰNG SỐ DÙNG CHUNG =======================
// Thời hạn mượn tối đa (ngày) mà TUI đang dùng.
enum { HAN_MUON_NGAY = 7 };

// ======================= CHUỖI: TRIM & CHUẨN HÓA =======================

inline std::string ltrim_copy(const std::string& s) {
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i])) != 0) {
        i++;
    }
    return s.substr(i);
}

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

inline std::string trim(const std::string& s) {
    return rtrim_copy(ltrim_copy(s));
}

inline char _to_upper_ascii(char ch) {
    if (ch >= 'a' && ch <= 'z') {
        return static_cast<char>(ch - 'a' + 'A');
    }
    return ch;
}

inline std::string to_upper_no_accents(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        out.push_back(_to_upper_ascii(s[i]));
    }
    return out;
}

// ======================= NGÀY THÁNG =======================

struct Date {
    int d; // 1..31 hoặc 0 nếu rỗng
    int m; // 1..12 hoặc 0 nếu rỗng
    int y; // >=1  hoặc 0 nếu rỗng
};

inline bool is_valid_date(const Date& a) {
    if (a.d <= 0 || a.m <= 0 || a.y <= 0) {
        return false;
    }
    if (a.m < 1 || a.m > 12) {
        return false;
    }
    static const int mdays[] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };
    int maxd = mdays[a.m];
    bool leap = ((a.y % 400 == 0) || ((a.y % 4 == 0) && (a.y % 100 != 0)));
    if (a.m == 2 && leap) {
        maxd = 29;
    }
    return (a.d >= 1 && a.d <= maxd);
}

// dd/mm/yyyy → Date (trả về qua out). True nếu hợp lệ.
inline bool parse_date_ddmmyyyy(const std::string& s, Date& out) {
    std::string t = trim(s);
    int d = 0, m = 0, y = 0;
    char c1 = 0, c2 = 0;
    std::istringstream iss(t);
    if (!(iss >> d)) { return false; }
    if (!(iss >> c1) || c1 != '/') { return false; }
    if (!(iss >> m)) { return false; }
    if (!(iss >> c2) || c2 != '/') { return false; }
    if (!(iss >> y)) { return false; }
    out.d = d; out.m = m; out.y = y;
    return is_valid_date(out);
}

// OVERLOAD khớp với TUI: dd/mm/yyyy -> Date (invalid => {0,0,0})
inline Date parse_date_ddmmyyyy(const std::string& s) {
    Date d{ 0,0,0 };
    (void)parse_date_ddmmyyyy(s, d);
    return d;
}

// Khoảng ngày: b - a (âm nếu b < a)
inline std::tm _to_tm(const Date& a) {
    std::tm tmv;
    tmv.tm_sec = 0; tmv.tm_min = 0; tmv.tm_hour = 0;
    tmv.tm_mday = (a.d <= 0 ? 1 : a.d);
    tmv.tm_mon = (a.m <= 0 ? 0 : (a.m - 1));
    tmv.tm_year = (a.y <= 0 ? 70 : (a.y - 1900)); // fallback 1970
    tmv.tm_isdst = -1;
    return tmv;
}

inline int days_between(const Date& a, const Date& b) {
    std::tm ta = _to_tm(a);
    std::tm tb = _to_tm(b);
    std::time_t ea = std::mktime(&ta);
    std::time_t eb = std::mktime(&tb);
    long diff = static_cast<long>(std::difftime(eb, ea) / (24L * 3600L));
    return static_cast<int>(diff);
}

// Tên hàm đúng như TUI đang gọi
inline int diff_days(const Date& b, const Date& a) {
    return days_between(a, b);
}

// ======================= CẤU TRÚC DỮ LIỆU CHÍNH =======================

enum TrangThaiBanSao {
    BANSAO_CHO_MUON = 0,
    BANSAO_DA_MUON = 1
};

enum TrangThaiMuonTra {
    MT_DANG_MUON = 0,
    MT_DA_TRA = 1,
    MT_MAT_SACH = 2
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

// Theo TUI: phai = "Nam"/"Nu"; trangThaiThe = 1/0
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

// ======================= TIỆN ÍCH KHÁC =======================

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

// gen_isbn_unique cần is_isbn_exists(...) (được định nghĩa ở dsdausach.h)
inline bool is_isbn_exists(const std::vector<DauSach*>& arr, const std::string& isbn);

inline std::string gen_isbn_unique(const std::vector<DauSach*>& dsArr) {
    std::mt19937 rng(static_cast<unsigned int>(std::time(NULL)));
    std::uniform_int_distribution<int> dist(100000000, 999999999);

    for (int i = 0; i < 4096; i++) {
        int x = dist(rng);
        std::ostringstream oss; oss << x;
        std::string cand = oss.str();
        if (!is_isbn_exists(dsArr, cand)) { return cand; }
    }
    for (int x = 100000000; x <= 999999999; x++) {
        std::ostringstream oss; oss << x;
        std::string cand = oss.str();
        if (!is_isbn_exists(dsArr, cand)) { return cand; }
    }
    return "999999999";
}


