#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <ctime>
#include <limits>
#include <cstdio>
#include <stdexcept>
#include <cstdlib>     
#include "tui.h"
#include "cautruc.h"
#include "dsdocgia.h"
#include "dsdausach.h"
#include "dsdms.h"
#include "dsmuontra.h"

namespace menutui {

    //==================== Khung menu ====================//
    inline void draw_menu_frame(const std::string& title, int itemsCount, int& outW, int& outH) {
        int h = 5 + itemsCount + 3; if (h < 12) h = 12;
        int w = 118;
        tui::clearScreen();
        tui::drawBox(2, 1, w, h, title);
        outW = w; outH = h;
    }
    inline void print_menu_item(int x, int y, const std::string& text, bool selected) {
        tui::gotoxy(x, y);
        if (selected) {            
            std::cout << "\x1b[7m" << text << "\x1b[0m";
            tui::setColor(tui::FG_HL);         
        }
        else {
            tui::setColor(tui::FG_HL);         
            std::cout << text;
        }
    }
    inline int menu_mui_ten(int x, int /*y*/, const std::string& title,
        const std::vector<std::string>& items) {
        const int MENU_Y = 6;
        int cur = 0;        
        int w = 0, h = 0;
        draw_menu_frame(title, (int)items.size(), w, h);
        int footerY = 1 + h - 2;        
        int pad = 0;
        for (auto& s : items) pad = std::max(pad, (int)s.size());
        
        auto paint_one = [&](int idx, bool selected) {
            tui::gotoxy(x, MENU_Y + idx);
            if (selected) {
                std::cout << "\x1b[7m" << items[idx];
                int sp = pad - (int)items[idx].size();
                if (sp > 0) std::cout << std::string(sp, ' ');
                std::cout << "\x1b[0m";        
                tui::setColor(tui::FG_HL);     
            }
            else {
                tui::setColor(tui::FG_HL);     
                std::cout << items[idx];
                int sp = pad - (int)items[idx].size();
                if (sp > 0) std::cout << std::string(sp, ' ');
            }
            };
        
        for (size_t i = 0; i < items.size(); ++i) paint_one((int)i, (int)i == cur);
        tui::print_footer_hints(4, footerY, "[Up/Down] Chon  -  [Enter] Xac nhan  -  [Esc] Quay lai");
                
        while (true) {
            auto e = tui::readKey();
            int prev = cur;
            if (e.key == tui::K_UP)      cur = (cur + (int)items.size() - 1) % (int)items.size();
            else if (e.key == tui::K_DOWN) cur = (cur + 1) % (int)items.size();
            else if (e.key == tui::K_ESC)   return (int)items.size() - 1;
            else if (e.key == tui::K_ENTER) return cur;
            if (cur != prev) {            
                paint_one(prev, false);
                paint_one(cur, true);
            }
        }
    }

    //==================== Input helpers ====================//
    inline void _flush_input_nonblock() {
        std::cin.clear();
        if (std::cin.rdbuf()->in_avail() > 0) std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    inline std::string _read_line_at(int x, int y, int maxlen = 64) {
        tui::gotoxy(x, y); tui::showCursor();
        std::string s; std::getline(std::cin, s); tui::hideCursor();
        if ((int)s.size() > maxlen) s.resize((size_t)maxlen);
        return trim(s);
    }
    inline int _read_int_opt_at(int x, int y, int minv, int maxv, int* outVal) {
        std::string s = _read_line_at(x, y, 16);
        if (s.empty()) return 0;
        try {
            int v = std::stoi(s);
            if (v<minv || v>maxv) throw std::out_of_range("range");
            if (outVal) *outVal = v;
            return 1;
        }
        catch (...) {
            tui::gotoxy(x, y); std::cout << std::string(24, ' ');
            tui::gotoxy(x, y); tui::setColor(tui::FG_ALERT); std::cout << "(so " << minv << "-" << maxv << ")"; tui::resetColor();
            return 0;
        }
    }
    inline int _radio_at(int x, int y, const std::vector<std::string>& opts, int sel0) {
        int sel = sel0;
        while (true) {
            tui::gotoxy(x, y);
            for (size_t i = 0; i < opts.size(); ++i) {
                bool on = ((int)i == sel);
                std::cout << (on ? "(*) " : "( ) ") << opts[i] << "   ";
            }
            auto ev = tui::readKey();
            if (ev.key == tui::K_LEFT) sel = (sel + (int)opts.size() - 1) % (int)opts.size();
            else if (ev.key == tui::K_RIGHT) sel = (sel + 1) % (int)opts.size();
            else if (ev.key == tui::K_ENTER) return sel;
            else if (ev.key == tui::K_ESC) return sel;
        }
    }

    //==================== Tiện ích ====================//
    inline std::string isbn_from_masach(const std::string& maSach) {
        size_t p = maSach.find('-'); return (p == std::string::npos) ? maSach : maSach.substr(0, p);
    }
    inline int count_borrowed_by_isbn(DocGiaNode* root, const std::string& isbn) {
        int cnt = 0; std::vector<DocGia*> v; duyet_LNR_luu_mang(root, v);
        for (auto* dg : v) for (MuonTraNode* p = dg->mtHead; p; p = p->next)
            if (p->trangThai == MT_DANG_MUON && isbn_from_masach(p->maSach) == isbn) cnt++;
        return cnt;
    }

    //================ Quản lý độc giả ================//
    //================= Thêm độc giả ==================//
    inline void form_them_doc_gia_tui(DocGiaNode*& root) {
        const int w = 118, h = 18, X0 = 4, Y0 = 3; const int footerY = 1 + h - 2;
        tui::clearScreen(); tui::drawBox(2, 1, w, h, "QUAN LY DOC GIA  >  THEM DOC GIA");
        int maThe = gen_ma_the_unique(root);
        int y = Y0 + 1;
        tui::gotoxy(X0, y); std::cout << "Ma the (tu sinh) : "; tui::gotoxy(X0 + 19, y); std::cout << maThe << " (read-only)"; y += 2;
        tui::gotoxy(X0, y); std::cout << "Ho va Ten Dem    : "; int hoX = X0 + 19, hoY = y; y += 2;
        tui::gotoxy(X0, y); std::cout << "Ten              : "; int tenX = X0 + 19, tenY = y; y += 2;
        tui::gotoxy(X0, y); std::cout << "Gioi tinh        : "; int gtX = X0 + 19, gtY = y; y += 2;
        tui::gotoxy(X0, y); std::cout << "Trang thai       : "; int ttX = X0 + 19, ttY = y; y += 2;
        tui::print_footer_hints(4, footerY, "[Enter] Chon/Luu  -  [Esc] Quay lai  -  (Left/Right) doi tuy chon");
        _flush_input_nonblock();
        std::string ho = _read_line_at(hoX, hoY, 60);
        std::string ten = _read_line_at(tenX, tenY, 60);
        int selGT = _radio_at(gtX, gtY, { "Nam","Nu" }, 0);
        std::string phai = (selGT == 1 ? "Nu" : "Nam");
        int selTT = _radio_at(ttX, ttY, { "Hoat dong","Khoa" }, 0);
        int trangThai = (selTT == 0 ? 1 : 0);
        if (trim(ho).empty() || trim(ten).empty()) {
            tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_ALERT); std::cout << "Loi: Ho/Ten khong duoc de trong."; tui::resetColor();
            tui::press_any_key_to_back(4, footerY - 1); return;
        }
        DocGia dg; dg.maThe = maThe; dg.ho = ho; dg.ten = ten; dg.phai = phai; dg.trangThaiThe = trangThai; dg.mtHead = NULL;
        insert_doc_gia(root, dg);
        tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_OK); std::cout << "Da them doc gia ma the " << maThe << "."; tui::resetColor();
        tui::press_any_key_to_back(4, footerY - 1);
    }

    //================== Xóa độc giả ==================//
    inline void form_xoa_doc_gia_tui(DocGiaNode*& root) {
        const int w = 118, h = 12, X0 = 4, Y0 = 3; const int footerY = 1 + h - 2;
        tui::clearScreen(); tui::drawBox(2, 1, w, h, "QUAN LY DOC GIA  >  XOA DOC GIA");
        int y = Y0 + 1;
        tui::gotoxy(X0, y); std::cout << "Nhap MA THE muon xoa : "; int maX = X0 + 24, maY = y;
        tui::print_footer_hints(4, footerY, "[Enter] Xac nhan  -  [Esc] Quay lai");
        _flush_input_nonblock();
        std::string sMa = _read_line_at(maX, maY, 12);
        int maThe = -1; try { maThe = std::stoi(sMa); }
        catch (...) {}
        if (maThe <= 0 || tim_node_doc_gia(root, maThe) == NULL) {
            tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_ALERT); std::cout << "Ma the khong hop le hoac khong ton tai."; tui::resetColor();
            tui::press_any_key_to_back(4, footerY - 1); return;
        }
        bool ok = xoa_doc_gia_if_no_borrowing(root, maThe);
        tui::gotoxy(X0, footerY - 2);
        if (ok) { tui::setColor(tui::FG_OK); std::cout << "Da xoa doc gia."; }
        else { tui::setColor(tui::FG_ALERT); std::cout << "Khong the xoa: doc gia dang muon sach."; }
        tui::resetColor();
        tui::press_any_key_to_back(4, footerY - 1);
    }

    //============== Sửa thông tin độc giả ===========//
    inline void form_sua_doc_gia_tui(DocGiaNode*& root) {
        const int w = 118, h = 22, X0 = 4, Y0 = 3; const int footerY = 1 + h - 2;
        tui::clearScreen(); tui::drawBox(2, 1, w, 10, "QUAN LY DOC GIA  >  CAP NHAT DOC GIA");
        int yAsk = Y0 + 1;
        tui::gotoxy(X0, yAsk); std::cout << "Nhap MA THE can cap nhat : "; int maX = X0 + 28, maY = yAsk;
        tui::print_footer_hints(4, 10 - 1, "[Enter] Xac nhan  -  [Esc] Quay lai");
        _flush_input_nonblock();
        std::string sMa = _read_line_at(maX, maY, 12);
        int maThe = -1; try { maThe = std::stoi(sMa); }
        catch (...) {}
        DocGiaNode* p = tim_node_doc_gia(root, maThe);
        if (!p) {
            const int footerHintY = 9;
            const int msgY = footerHintY - 1;
            const int errY = msgY - 1;
            tui::gotoxy(X0, errY);
            tui::setColor(tui::FG_ALERT);
            std::cout << "Khong tim thay.";
            tui::resetColor();
            tui::press_any_key_to_back(4, msgY);
            return;
        }
        tui::clearScreen(); tui::drawBox(2, 1, w, h, "QUAN LY DOC GIA  >  CAP NHAT DOC GIA");
        int y = Y0 + 1;
        tui::gotoxy(X0, y++); std::cout << "(Bo trong neu khong thay doi)";
        y++;
        tui::gotoxy(X0, y); std::cout << "Ho va Ten Dem    : "; int hoX = X0 + 19, hoY = y; y += 2;
        tui::gotoxy(X0, y); std::cout << "Ten              : "; int tenX = X0 + 19, tenY = y; y += 2;
        tui::gotoxy(X0, y); std::cout << "Gioi tinh        : "; int gtX = X0 + 19, gtY = y;  y += 2;
        tui::gotoxy(X0, y); std::cout << "Trang thai       : "; int ttX = X0 + 19, ttY = y;  y += 2;
        tui::print_footer_hints(4, footerY, "[Enter] Chon/Luu  -  [Esc] Quay lai  -  (Left/Right) doi");
        _flush_input_nonblock();
        // Đọc nhưng chưa áp dụng
        std::string inHo = _read_line_at(hoX, hoY, 60);
        std::string inTen = _read_line_at(tenX, tenY, 60);
        int selGT = (p->info.phai == "Nu") ? 1 : 0; selGT = _radio_at(gtX, gtY, { "Nam","Nu" }, selGT);
        std::string newPhai = (selGT == 1 ? "Nu" : "Nam");
        int selTT = (p->info.trangThaiThe == 1) ? 0 : 1; selTT = _radio_at(ttX, ttY, { "Hoat dong","Khoa" }, selTT);
        int newTrangThai = (selTT == 0 ? 1 : 0);
        // Xác nhận
        int confirmY = y;
        tui::gotoxy(X0, confirmY); std::cout << "Xac nhan cap nhat: ";
        int ok = _radio_at(X0 + 22, confirmY, { "Co","Khong" }, 0);
        if (ok != 0) { tui::gotoxy(X0, footerY - 2); std::cout << "Da huy cap nhat doc gia."; tui::press_any_key_to_back(4, footerY - 1); return; }
        // Áp dụng
        if (!inHo.empty())  p->info.ho = inHo;
        if (!inTen.empty()) p->info.ten = inTen;
        p->info.phai = newPhai;
        p->info.trangThaiThe = newTrangThai;
        // Thông báo kết quả
        tui::gotoxy(X0, footerY - 2); std::cout << "Da cap nhat doc gia.";
        tui::press_any_key_to_back(4, footerY - 1);
    }

    //============= Quản lý đầu sách ====================//
    //================ Thêm đầu sách ===================//
    inline void form_them_dau_sach_tui(std::vector<DauSach*>& dsArr) {
        const int w = 118, h = 20, X0 = 4, Y0 = 3; const int footerY = 1 + h - 2;
        tui::clearScreen(); tui::drawBox(2, 1, w, h, "QUAN LY DAU SACH  >  THEM DAU SACH");
        std::string isbn = gen_isbn_unique(dsArr);
        int y = Y0 + 1;
        tui::gotoxy(X0, y); std::cout << "ISBN (tu sinh): ";  tui::gotoxy(X0 + 16, y); std::cout << isbn << " (read-only)"; y += 2;
        tui::gotoxy(X0, y); std::cout << "Ten sach      : "; int tenX = X0 + 16, tenY = y; y += 2;
        tui::gotoxy(X0, y); std::cout << "So trang      : "; int stX = X0 + 16, stY = y; y += 2;
        tui::gotoxy(X0, y); std::cout << "Tac gia       : "; int tgX = X0 + 16, tgY = y; y += 2;
        tui::gotoxy(X0, y); std::cout << "Nam xuat ban  : "; int namX = X0 + 16, namY = y; y += 2;
        tui::gotoxy(X0, y); std::cout << "The loai      : "; int tlX = X0 + 16, tlY = y; y += 2;
        int X1 = X0 + 68; int y2 = Y0 + 1;
        tui::gotoxy(X1, y2); std::cout << "So luong ban sao : "; int slX = X1 + 19, slY = y2; y2 += 2;
        tui::gotoxy(X1, y2); std::cout << "Ke               : "; int keX = X1 + 19, keY = y2; y2 += 2;
        tui::gotoxy(X1, y2); std::cout << "Hang             : "; int hangX = X1 + 19, hangY = y2; y2 += 2;
        tui::print_footer_hints(4, footerY, "[Enter] Luu  -  [Esc] Quay lai");
        _flush_input_nonblock();
        std::string ten = _read_line_at(tenX, tenY, 60);
        int soTrang = 200; _read_int_opt_at(stX, stY, 1, 5000, &soTrang);
        std::string tacGia = _read_line_at(tgX, tgY, 60);
        std::time_t t = std::time(NULL); std::tm lt{};
#ifdef _WIN32
        localtime_s(&lt, &t);
#else
        std::tm* p = std::localtime(&t); if (p) lt = *p;
#endif
        int currentYear = lt.tm_year + 1900;
        int namXB = currentYear; _read_int_opt_at(namX, namY, 1900, currentYear, &namXB);
        std::string theLoai = _read_line_at(tlX, tlY, 48);
        int soLuong = 1; _read_int_opt_at(slX, slY, 1, 1000, &soLuong);
        std::string ke = _read_line_at(keX, keY, 16);
        std::string hang = _read_line_at(hangX, hangY, 16);
        if (ten.empty() || tacGia.empty() || theLoai.empty() || ke.empty() || hang.empty()) {
            tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_ALERT); std::cout << "Loi: Ten/Tac gia/The loai/Ke/Hang khong duoc de trong."; tui::resetColor();
            tui::press_any_key_to_back(4, footerY - 1); return;
        }
        DauSach* item = new DauSach();
        item->ISBN = isbn; item->tenSach = ten; item->soTrang = soTrang;
        item->tacGia = tacGia; item->namXB = namXB; item->theLoai = theLoai;
        item->dmsHead = NULL; item->soLuongBanSao = 0; item->soLuotMuon = 0;
        tao_ban_sao_tu_dong(item, soLuong, ke, hang);
        chen_dau_sach_sorted_by_ten(dsArr, item);
        tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_OK); std::cout << "Da them dau sach [" << isbn << "]."; tui::resetColor();
        tui::press_any_key_to_back(4, footerY - 1);
    }

    //================ Xóa đầu sách =================//
    inline void form_xoa_dau_sach_tui(std::vector<DauSach*>& dsArr, DocGiaNode* root) {
        const int w = 90, h = 16, X0 = 4, Y0 = 3; const int footerY = 1 + h - 2;
        tui::clearScreen(); tui::drawBox(2, 1, w, h, "QUAN LY DAU SACH  >  XOA DAU SACH");
        int y = Y0 + 1;
        tui::gotoxy(X0, y); std::cout << "Nhap ISBN: "; int isbnX = X0 + 12, isbnY = y; y += 1;
        tui::print_footer_hints(4, footerY, "[Enter] Xac nhan  -  [Esc] Quay lai");
        _flush_input_nonblock();
        std::string isbn = _read_line_at(isbnX, isbnY, 20);
        DauSach* ds = tim_dau_sach_theo_isbn(dsArr, isbn);
        if (!ds) { tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_ALERT); std::cout << "Khong tim thay ISBN."; tui::resetColor(); tui::press_any_key_to_back(4, footerY - 1); return; }
        int tong = dms_count_total(ds);
        int dangMuon = count_borrowed_by_isbn(root, isbn);
        tui::gotoxy(X0, y++); std::cout << "Tong so ban sao : " << tong;
        tui::gotoxy(X0, y++); std::cout << "Dang cho muon   : " << dangMuon;
        if (dangMuon > 0) {
            tui::gotoxy(X0, y + 1); tui::setColor(tui::FG_ALERT); std::cout << "Khong the xoa: co ban sao dang cho muon."; tui::resetColor();
            tui::press_any_key_to_back(4, footerY - 1); return;
        }
        tui::gotoxy(X0, y + 1); std::cout << "Xac nhan xoa dau sach: ";
        int sel = _radio_at(X0 + 26, y + 1, { "Xoa","Huy" }, 1);
        if (sel != 0) { tui::gotoxy(X0, y + 3); std::cout << "Da huy."; tui::press_any_key_to_back(4, footerY - 1); return; }
        dms_free_all(ds->dmsHead);
        for (size_t i = 0; i < dsArr.size(); ++i) if (dsArr[i] == ds) { delete dsArr[i]; dsArr.erase(dsArr.begin() + (long long)i); break; }
        tui::gotoxy(X0, y + 3); tui::setColor(tui::FG_OK); std::cout << "Da xoa dau sach."; tui::resetColor();
        tui::press_any_key_to_back(4, footerY - 1);
    }

    //========= Cập nhật thông tin đầu sách ==========//
    inline void form_cap_nhat_dau_sach_tui(std::vector<DauSach*>& dsArr) {
        const int w = 118, h = 26, X0 = 4, Y0 = 3;
        const int footerY = 1 + h - 2;
        tui::clearScreen();
        tui::drawBox(2, 1, w, h, "QUAN LY DAU SACH  >  CAP NHAT THONG TIN");
        int y = Y0 + 1;
        tui::gotoxy(X0, y); std::cout << "Nhap ISBN        : ";
        int isbnX = X0 + 18, isbnY = y;
        y += 2;
        tui::gotoxy(X0, y++); std::cout << "(Bo trong neu khong doi)";
        y++;
        tui::gotoxy(X0, y); std::cout << "TEN SACH         : "; int tenX = X0 + 18, tenY = y; y += 2;
        tui::gotoxy(X0, y); std::cout << "TAC GIA          : "; int tgX = X0 + 18, tgY = y; y += 2;
        tui::gotoxy(X0, y); std::cout << "NAM XUAT BAN     : "; int namX = X0 + 18, namY = y; y += 2;
        tui::gotoxy(X0, y); std::cout << "SO TRANG         : "; int stX = X0 + 18, stY = y; y += 2;
        tui::gotoxy(X0, y); std::cout << "SO LUONG BAN SAO : "; int slX = X0 + 18, slY = y; y += 2;
        tui::gotoxy(X0, y); std::cout << "KE               : "; int keX = X0 + 18, keY = y; y += 2;
        tui::gotoxy(X0, y); std::cout << "HANG             : "; int hangX = X0 + 18, hangY = y; y += 2;
        tui::print_footer_hints(4, footerY, "[Enter] Luu  -  [Esc] Quay lai");
        // ==== ĐỌC DỮ LIỆU ====
        _flush_input_nonblock();
        std::string isbn = _read_line_at(isbnX, isbnY, 20);
        DauSach* ds = tim_dau_sach_theo_isbn(dsArr, isbn);
        if (!ds) {
            tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_ALERT);
            std::cout << "Khong tim thay ISBN."; tui::resetColor();
            tui::press_any_key_to_back(4, footerY - 1);
            return;
        }
        std::string inTen = _read_line_at(tenX, tenY, 60);
        std::string inTG = _read_line_at(tgX, tgY, 60);
        int inNam = 0; _read_int_opt_at(namX, namY, 1900, 3000, &inNam);
        int inST = 0; _read_int_opt_at(stX, stY, 1, 5000, &inST);
        int slTarget = 0; int hasSL = _read_int_opt_at(slX, slY, 1, 10000, &slTarget);
        std::string inKe = _read_line_at(keX, keY, 16);
        std::string inHang = _read_line_at(hangX, hangY, 16);
        // ==== XÁC NHẬN ====
        int confirmY = hangY + 2;
        if (confirmY > footerY - 3) confirmY = footerY - 3;
        tui::gotoxy(X0, confirmY);
        std::cout << "Xac nhan cap nhat: ";
        int ok = _radio_at(X0 + 22, confirmY, { "Co", "Khong" }, 0);
        // ==== ÁP DỤNG THAY ĐỔI ====
        if (!inTen.empty()) ds->tenSach = inTen;
        if (!inTG.empty())  ds->tacGia = inTG;
        if (inNam > 0)      ds->namXB = inNam;
        if (inST > 0)      ds->soTrang = inST;
        // Đổi vị trí đồng loạt nếu nhập
        if (!inKe.empty() || !inHang.empty()) {
            std::string viTri = lay_vi_tri_chung(ds);
            std::string keCur = "A", hangCur = "1";
            size_t pKe = viTri.find("Ke "), pH = viTri.find(" - Hang ");
            if (pKe != std::string::npos && pH != std::string::npos) {
                keCur = trim(viTri.substr(pKe + 3, pH - (pKe + 3)));
                hangCur = trim(viTri.substr(pH + 8));
            }
            doi_vi_tri_tat_ca_ban_sao(ds, inKe.empty() ? keCur : inKe,
                inHang.empty() ? hangCur : inHang);
        }
        // Thay đổi số lượng 
        if (hasSL) {
            if (slTarget > ds->soLuongBanSao) {
                std::string viTri = lay_vi_tri_chung(ds);
                std::string keCur = "A", hangCur = "1";
                size_t pKe = viTri.find("Ke "), pH = viTri.find(" - Hang ");
                if (pKe != std::string::npos && pH != std::string::npos) {
                    keCur = trim(viTri.substr(pKe + 3, pH - (pKe + 3)));
                    hangCur = trim(viTri.substr(pH + 8));
                }
                tao_ban_sao_tu_dong(ds, slTarget - ds->soLuongBanSao,
                    inKe.empty() ? keCur : inKe, inHang.empty() ? hangCur : inHang);
            }
            else if (slTarget < ds->soLuongBanSao) {
                (void)giam_ban_sao_tu_cuoi(ds, ds->soLuongBanSao - slTarget);
            }
        }
        // Thông báo kết quả
        tui::gotoxy(X0, footerY - 2); std::cout << "Da cap nhat dau sach.";
        tui::press_any_key_to_back(4, footerY - 1);
    }

    //================ Danh sách theo thể loại =================//
    inline void ui_ds_in_theo_the_loai(std::vector<DauSach*>& dsArr) {
        const int w = 118, h = 30, X0 = 4;
        const int footerY = 1 + h - 2;
        const int yMax = 1 + h - 3;       
        std::map<std::string, std::vector<DauSach*>> groups;
        for (auto* ds : dsArr) {
            if (ds != NULL) {
                groups[ds->theLoai].push_back(ds);
            }
        }
        for (auto& kv : groups) {
            auto& v = kv.second;
            std::sort(v.begin(), v.end(), [](DauSach* a, DauSach* b) {
                if (a->tenSach != b->tenSach) { return a->tenSach < b->tenSach; }
                return a->ISBN < b->ISBN;
                });
        }        
        std::vector<std::string> linesOut;
        for (auto& kv : groups) {
            const std::string theLoai = kv.first;
            const auto& v = kv.second;
            linesOut.push_back("The loai: " + theLoai + " (So dau sach: " + std::to_string((int)v.size()) + ")");
            for (auto* ds : v) {
                std::string ten = ds->tenSach; if ((int)ten.size() > 45) { ten = ten.substr(0, 45); }
                std::string tg = ds->tacGia;  if ((int)tg.size() > 24) { tg = tg.substr(0, 24); }
                int soBan = dms_count_total(ds);
                std::string viTri = lay_vi_tri_chung(ds);

                std::string line =
                    "  - [" + ds->ISBN + "] " + ten +
                    " | " + tg +
                    " | " + std::to_string(ds->namXB) +
                    " | So ban sao: " + std::to_string(soBan) +
                    " | Vi tri: " + viTri;
                linesOut.push_back(line);
            }
            linesOut.push_back(std::string());
        }        
        if (linesOut.empty()) {
            tui::clearScreen();
            tui::drawBox(2, 1, w, h, "QUAN LY DAU SACH  >  DANH SACH THEO THE LOAI");
            tui::setColor(tui::FG_HL);
            tui::gotoxy(X0, 5); std::cout << "(Khong co dau sach.)";
            tui::print_footer_hints(4, footerY, "[Esc] Quay lai");
            while (true) { tui::KeyEvent e = tui::readKey(); if (e.key == tui::K_ESC) { return; } }
        }        
        const int startY = 5;
        const int CAP = yMax - startY + 1;
        const int total = (int)linesOut.size();
        const int pages = (total + CAP - 1) / CAP;
        int page = 0;        
        tui::clearScreen();
        tui::drawBox(2, 1, w, h, "QUAN LY DAU SACH  >  DANH SACH THEO THE LOAI");
        tui::print_footer_hints(4, footerY, "[Up/Down] Trang truoc/sau   -   [Esc] Quay lai");
        tui::setColor(tui::FG_HL);         
        auto paint_page_hint = [&](int p) {
            tui::gotoxy(4, 3);
            std::cout << "(Trang " << (p + 1) << "/" << std::max(1, pages) << ")"
                << std::string(20, ' ');
            };        
        auto paint_page = [&](int p) {
            paint_page_hint(p);
            tui::clear_rect(4, startY, w - 8, CAP); 
            int y = startY;
            int from = p * CAP;
            int to = std::min(total, from + CAP);
            for (int i = from; i < to; ++i) {
                tui::gotoxy(X0, y++);
                std::cout << linesOut[i];
            }
            };        
        paint_page(page);        
        while (true) {
            tui::KeyEvent ev = tui::readKey();
            if (ev.key == tui::K_ESC) { return; }
            if (ev.key == tui::K_UP) { if (page > 0) { --page; paint_page(page); } }
            if (ev.key == tui::K_DOWN) { if (page + 1 < pages) { ++page; paint_page(page); } }
        }
    }

    //================ Tim dau sach theo TEN =================//
    inline void ui_ds_tim_theo_ten(std::vector<DauSach*>& dsArr) {
        const int w = 118, h = 30, X0 = 4;
        const int footerY = 1 + h - 2;
        const int yMax = 1 + h - 3;        
        tui::clearScreen();
        tui::drawBox(2, 1, w, h, "QUAN LY DAU SACH  >  TIM THEO TEN");
        tui::setColor(tui::FG_HL);
        int y = 4;
        tui::gotoxy(X0, y); std::cout << "Nhap tu khoa ten sach: ";
        int qX = X0 + 24, qY = y; y += 2;
        tui::print_footer_hints(4, footerY, "[Enter] Tim  -  [Esc] Quay lai");
        _flush_input_nonblock();
        std::string q = _read_line_at(qX, qY, 64);
        if (q.empty()) {
            tui::gotoxy(X0, footerY - 2);
            tui::setColor(tui::FG_ALERT); std::cout << "Tu khoa khong duoc rong.";
            tui::resetColor();
            tui::press_any_key_to_back(4, footerY - 1);
            return;
        }       
        std::vector<DauSach*> found = tim_dau_sach_theo_ten(dsArr, q);
        std::vector<std::string> linesOut;
        auto fmt_line = [&](const std::string& s) -> std::string {
            if ((int)s.size() <= w - 8) return s;
            return s.substr(0, w - 8);
            };
        auto trang_thai_str = [&](int st) -> std::string {
            if (st == BANSAO_CHO_MUON) return "CHO MUON";
            if (st == BANSAO_DA_MUON) return "DA MUON";
            return "?";
        };

        if (found.empty()) {            
            tui::gotoxy(X0, 6); std::cout << "Khong tim thay dau sach phu hop.";
            tui::print_footer_hints(4, footerY, "[Esc] Quay lai");
            while (true) {
                tui::KeyEvent e = tui::readKey();
                if (e.key == tui::K_ESC) return;
            }
        }
        for (size_t i = 0; i < found.size(); ++i) {
            DauSach* ds = found[i];
            linesOut.push_back(fmt_line(std::to_string((int)i + 1) + ". ISBN: " + ds->ISBN + " | Ten: " + ds->tenSach));
            linesOut.push_back(fmt_line("    Tac gia: " + ds->tacGia + " | Nam: " + std::to_string(ds->namXB) + " | The loai: " + ds->theLoai));
            std::string cur = "    Ma ban sao: ";
            int countInLine = 0;
            for (DanhMucSachNode* p = ds->dmsHead; p != NULL; p = p->next) {
                std::string token = p->maSach + std::string(" (") + trang_thai_str(p->trangThai) + ")";
                if ((int)(cur.size() + token.size() + 2) > w - 8) {
                    linesOut.push_back(fmt_line(cur));
                    cur = "                  " + token;
                    countInLine = 1;
                }
                else {
                    if (countInLine > 0) cur += ", ";
                    cur += token;
                    countInLine++;
                }
            }
            linesOut.push_back(fmt_line(cur));
            linesOut.push_back("");
        }        
        const int startY = 5;
        const int CAP = yMax - startY + 1;
        const int total = (int)linesOut.size();
        const int pages = (total + CAP - 1) / CAP;
        int page = 0;        
        tui::clearScreen();
        tui::drawBox(2, 1, w, h, "QUAN LY DAU SACH  >  TIM THEO TEN");
        tui::print_footer_hints(4, footerY, "[Up/Down] Trang  -  [Esc] Quay lai");
        tui::setColor(tui::FG_HL);        
        auto paint_page_hint = [&](int p) {
            tui::gotoxy(4, 3);
            std::cout << "(Trang " << (p + 1) << "/" << std::max(1, pages) << ")"
                << std::string(20, ' ');
        };        
        auto paint_page = [&](int p) {
            paint_page_hint(p);
            tui::clear_rect(4, startY, w - 8, CAP); 
            int y2 = startY;
            int start = p * CAP, end = std::min(total, start + CAP);
            for (int i = start; i < end; ++i) {
                tui::gotoxy(X0, y2++);
                std::cout << linesOut[i];
            }
        };
        paint_page(page);
        while (true) {
            tui::KeyEvent ev = tui::readKey();
            if (ev.key == tui::K_ESC) return;
            if (ev.key == tui::K_UP) { if (page > 0) { --page; paint_page(page); } }
            if (ev.key == tui::K_DOWN) { if (page + 1 < pages) { ++page; paint_page(page); } }
        }
    }

    //================ Mượn/trả sách ==============//
    //================ Mượn sách =================//
    inline void form_muon_sach_tui(std::vector<DauSach*>& dsArr, DocGiaNode*& root) {
        const int w = 118, h = 18, X0 = 4, Y0 = 3;
        const int footerY = 1 + h - 2;
        tui::clearScreen();
        tui::drawBox(2, 1, w, h, "MUON / TRA  >  MUON SACH THEO ISBN");
        int y = Y0 + 1;
        tui::gotoxy(X0, y); std::cout << "Nhap MA THE                : "; int maX = X0 + 28, maY = y; y += 2;
        tui::gotoxy(X0, y); std::cout << "Nhap ISBN                  : "; int isbnX = X0 + 28, isbnY = y; y += 2;
        tui::gotoxy(X0, y); std::cout << "Nhap ngay muon (dd/mm/yyyy): "; int ngayX = X0 + 28, ngayY = y; y += 2;
        tui::print_footer_hints(4, footerY, "[Enter] Xac nhan  -  [Esc] Quay lai");
        _flush_input_nonblock();
        int maThe = -1;
        while (true) {
            std::string s = _read_line_at(maX, maY, 12);
            if (!s.empty()) { try { maThe = std::stoi(s); } catch (...) { maThe = -1; } }
            if (maThe > 0) break;
            tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_ALERT); std::cout << "Ma the khong hop le. Moi nhap lai."; tui::resetColor();
            tui::gotoxy(maX, maY); std::cout << std::string(20, ' ');
        }
        DocGiaNode* pNode = tim_node_doc_gia(root, maThe);
        if (pNode == NULL) {
            tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_ALERT); std::cout << "Khong tim thay doc gia."; tui::resetColor();
            tui::press_any_key_to_back(4, footerY - 1); return;
        }
        if (pNode->info.trangThaiThe != 1) {
            tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_ALERT); std::cout << "The dang bi khoa/khong hoat dong."; tui::resetColor();
            tui::press_any_key_to_back(4, footerY - 1); return;
        }
        if (dem_mt_dang_muon(pNode->info) >= 3) {
            tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_ALERT); std::cout << "Doc gia da muon toi da 3 cuon."; tui::resetColor();
            tui::press_any_key_to_back(4, footerY - 1); return;
        }
        std::string isbn = _read_line_at(isbnX, isbnY, 20);
        DauSach* ds = tim_dau_sach_theo_isbn(dsArr, isbn);
        if (ds == NULL) {
            tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_ALERT); std::cout << "Khong tim thay dau sach voi ISBN da nhap."; tui::resetColor();
            tui::press_any_key_to_back(4, footerY - 1); return;
        }
        DanhMucSachNode* banSao = dms_find_first_available(ds);
        if (banSao == NULL) {
            tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_ALERT); std::cout << "Tat ca ban sao cua sach nay dang khong kha dung."; tui::resetColor();
            tui::press_any_key_to_back(4, footerY - 1); return;
        }
        Date ngayMuon;
        while (true) {
            std::string s = _read_line_at(ngayX, ngayY, 16);
            ngayMuon = parse_date_ddmmyyyy(s);
            if (is_valid_date(ngayMuon)) break;
            tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_ALERT); std::cout << "Ngay khong hop le. Moi nhap lai!"; tui::resetColor();
            tui::gotoxy(ngayX, ngayY); std::cout << std::string(20, ' ');
        }
        {
            int treMax = 0;
            if (doc_gia_co_qua_han_den_ngay(pNode->info, ngayMuon, &treMax)) {
                tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_ALERT);
                std::cout << "Doc gia dang co sach QUA HAN, khong duoc muon (Tre toi da: " << treMax << " ngay).";
                tui::resetColor();
                tui::press_any_key_to_back(4, footerY - 1); return;
            }
        }
        const int confirmY = footerY - 3;
        tui::gotoxy(X0, confirmY); std::cout << "Xac nhan muon sach: ";
        int ok = _radio_at(X0 + 22, confirmY, { "Co", "Khong" }, 0);
        if (ok != 0) {
            tui::gotoxy(X0, footerY - 2); std::cout << "Da huy muon sach.";
            tui::press_any_key_to_back(4, footerY - 1); return;
        }
        if (!dms_mark_borrowed(banSao)) {
            tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_ALERT); std::cout << "Khong the danh dau ban sao la DA MUON."; tui::resetColor();
            tui::press_any_key_to_back(4, footerY - 1); return;
        }
        them_phieu_muon_cho_doc_gia(pNode->info, banSao->maSach, ngayMuon);
        ds->soLuotMuon += 1;
        tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_OK);
        std::cout << "Da MUON thanh cong: " << ds->ISBN << " | " << ds->tenSach << " | MaSach: " << banSao->maSach;
        tui::resetColor();
        tui::press_any_key_to_back(4, footerY - 1);
    }

    //================ Trả sách =================//   
    inline void form_tra_sach_tui(std::vector<DauSach*>& dsArr, DocGiaNode*& root) {
        const int w = 118, h = 24, X0 = 4, Y0 = 3;
        const int footerY = 1 + h - 2;
        const int yMax = 1 + h - 3;
        const int CW_STT = 4, CW_MS = 13, CW_ISBN = 11, CW_TEN = 42, CW_NGAY = 10;
        auto PAD = [](const std::string& s, int w) -> std::string {
            if ((int)s.size() >= w) return s.substr(0, w);
            return s + std::string(w - (int)s.size(), ' ');
            };
        auto DASH = [](int n) -> std::string { return std::string(n, '-'); };
        auto fmt_date = [](const Date& d) -> std::string {
            char buf[16]; std::snprintf(buf, sizeof(buf), "%02d/%02d/%04d", d.d, d.m, d.y); return std::string(buf);
            };
        tui::clearScreen();
        tui::drawBox(2, 1, w, h, "MUON / TRA  >  TRA SACH");
        int y = Y0 + 1;
        tui::gotoxy(X0, y); std::cout << "Nhap MA THE: ";
        int maX = X0 + 14, maY = y;
        tui::print_footer_hints(4, footerY, "[Enter] Xac nhan  -  [Esc] Quay lai");
        _flush_input_nonblock();
        std::string sMa = _read_line_at(maX, maY, 12);
        int maThe = -1; try { maThe = std::stoi(sMa); }
        catch (...) { maThe = -1; }
        DocGiaNode* pNode = tim_node_doc_gia(root, maThe);
        if (maThe <= 0 || pNode == NULL) {
            tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_ALERT); std::cout << "Ma the khong hop le hoac khong ton tai."; tui::resetColor();
            tui::press_any_key_to_back(4, footerY - 1); return;
        }
        std::vector<MuonTraNode*> rows;
        for (MuonTraNode* p = pNode->info.mtHead; p; p = p->next)
            if (p->trangThai == MT_DANG_MUON) rows.push_back(p);
        y += 2;
        tui::gotoxy(X0, y++); std::cout << "Danh sach DANG MUON:";
        if (rows.empty()) {
            tui::gotoxy(X0, y++); std::cout << "(Doc gia khong co sach dang muon.)";
            tui::press_any_key_to_back(4, footerY - 1); return;
        }
        std::string header = PAD("STT", CW_STT) + " | "
            + PAD("MaSach", CW_MS) + " | "
            + PAD("ISBN", CW_ISBN) + " | "
            + PAD("Ten sach", CW_TEN) + " | "
            + PAD("Ngay muon", CW_NGAY);
        std::string sep = DASH(CW_STT) + "-+-" + DASH(CW_MS) + "-+-" + DASH(CW_ISBN) + "-+-" + DASH(CW_TEN) + "-+-" + DASH(CW_NGAY);
        tui::gotoxy(X0, y++); std::cout << header;
        tui::gotoxy(X0, y++); std::cout << sep;
        for (size_t i = 0; i < rows.size(); ++i) {
            MuonTraNode* mt = rows[i];
            std::string isbn = isbn_from_masach(mt->maSach);
            DauSach* ds = tim_dau_sach_theo_isbn(dsArr, isbn);
            std::string ten = ds ? ds->tenSach : "";
            if ((int)ten.size() > CW_TEN) ten = ten.substr(0, CW_TEN);
            std::string line = PAD(std::to_string((int)i + 1), CW_STT) + " | "
                + PAD(mt->maSach, CW_MS) + " | "
                + PAD(isbn, CW_ISBN) + " | "
                + PAD(ten, CW_TEN) + " | "
                + PAD(fmt_date(mt->ngayMuon), CW_NGAY);
            tui::gotoxy(X0, y++); std::cout << line;
            if (y > yMax) {
                tui::press_any_key_to_back(4, footerY - 1);
                tui::clearScreen(); tui::drawBox(2, 1, w, h, "MUON / TRA  >  TRA SACH (tiep)");
                y = 5;
                tui::gotoxy(X0, y++); std::cout << header;
                tui::gotoxy(X0, y++); std::cout << sep;
            }
        }
        int promptY = std::min(footerY - 6, y + 1);
        tui::gotoxy(X0, promptY); std::cout << "Nhap STT de tra (hoac bo trong de nhap ma sach de tra): ";
        int chooseX = X0 + 55, chooseY = promptY;
        _flush_input_nonblock();
        std::string sChon = _read_line_at(chooseX, chooseY, 8);
        MuonTraNode* target = NULL;
        if (!sChon.empty()) {
            int idx = -1; try { idx = std::stoi(sChon); }
            catch (...) { idx = -1; }
            if (1 <= idx && idx <= (int)rows.size()) target = rows[(size_t)idx - 1];
        }
        if (target == NULL) {
            tui::gotoxy(X0, chooseY + 1); std::cout << "Nhap MaSach can tra: ";
            int msX = X0 + 23, msY = chooseY + 1;
            std::string ms = _read_line_at(msX, msY, 20);
            for (auto* p : rows) if (p->maSach == ms) { target = p; break; }
        }
        if (target == NULL) {
            tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_ALERT); std::cout << "Lua chon khong hop le."; tui::resetColor();
            tui::press_any_key_to_back(4, footerY - 1); return;
        }
        Date ngayTra{};
        while (true) {
            tui::gotoxy(X0, chooseY + 3); std::cout << "Nhap ngay tra (dd/mm/yyyy): ";
            int ntX = X0 + 30, ntY = chooseY + 3;
            std::string s = _read_line_at(ntX, ntY, 16);
            ngayTra = parse_date_ddmmyyyy(s);
            if (is_valid_date(ngayTra)) break;
            tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_ALERT); std::cout << "Ngay khong hop le. Moi nhap lai!"; tui::resetColor();
            tui::gotoxy(ntX, ntY); std::cout << std::string(20, ' ');
        }
        target->trangThai = MT_DA_TRA;
        target->ngayTra = ngayTra;
        std::string isbn2 = isbn_from_masach(target->maSach);
        if (DauSach* ds2 = tim_dau_sach_theo_isbn(dsArr, isbn2)) {
            if (DanhMucSachNode* bs = dms_find_by_masach(ds2, target->maSach)) dms_mark_returned(bs);
        }
        int soNgay = diff_days(ngayTra, target->ngayMuon);
        int tre = soNgay - HAN_MUON_NGAY;
        tui::gotoxy(X0, footerY - 2);
        std::cout << "Da TRA sach: " << target->maSach
            << " | So ngay muon: " << soNgay
            << (tre > 0 ? (" | Tre han: " + std::to_string(tre) + " ngay") : "") << ".";
        tui::press_any_key_to_back(4, footerY - 1);
    }

    //========= Danh sách sách đang mược của độc giả ===========//
    inline void form_in_dang_muon_doc_gia_tui(std::vector<DauSach*>& dsArr, DocGiaNode* root) {
        const int w = 118, h = 24, X0 = 4, Y0 = 3;
        const int footerY = 1 + h - 2, yMax = 1 + h - 3;
        const int CW_STT = 4;
        const int CW_MS = 13;
        const int CW_ISBN = 11;
        const int CW_TEN = 42;
        const int CW_NGAY = 10;
        const int CW_SONG = 7;
        auto PAD = [](const std::string& s, int w)->std::string {
            if ((int)s.size() >= w) return s.substr(0, w);
            return s + std::string(w - (int)s.size(), ' ');
            };
        auto DASH = [](int n)->std::string { return std::string(n, '-'); };
        auto fmt_date = [](const Date& d)->std::string {
            char buf[16]; std::snprintf(buf, sizeof(buf), "%02d/%02d/%04d", d.d, d.m, d.y);
            return std::string(buf);
            };
        auto find_ds = [&](const std::string& isbn)->const DauSach* {
            for (auto* p : dsArr) if (p && p->ISBN == isbn) return p; return nullptr;
            };
        auto isbn_from_ms = [](const std::string& ms)->std::string {
            size_t pos = ms.find('-'); return (pos == std::string::npos) ? ms : ms.substr(0, pos);
            };
        tui::clearScreen();
        tui::drawBox(2, 1, w, h, "MUON / TRA  >  DANH SACH DANG MUON CUA DOC GIA");
        int y = Y0 + 1;
        tui::gotoxy(X0, y); std::cout << "Nhap MA THE: ";
        int theX = X0 + 14, theY = y; y += 2;
        tui::gotoxy(X0, y); std::cout << "Nhap ngay hien tai (dd/mm/yyyy): ";
        int dateX = X0 + 34, dateY = y; y += 2;
        tui::print_footer_hints(4, footerY, "[Enter] Xac nhan  -  [Esc] Quay lai");
        _flush_input_nonblock();
        std::string sThe = _read_line_at(theX, theY, 12);
        int maThe = -1; try { maThe = std::stoi(sThe); }
        catch (...) { maThe = -1; }
        DocGiaNode* pNode = tim_node_doc_gia(root, maThe);
        if (maThe <= 0 || pNode == nullptr) {
            tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_ALERT);
            std::cout << "Ma the khong hop le hoac khong ton tai."; tui::resetColor();
            tui::press_any_key_to_back(4, footerY - 1); return;
        }
        Date today{};
        while (true) {
            std::string s = _read_line_at(dateX, dateY, 16);
            today = parse_date_ddmmyyyy(s);
            if (is_valid_date(today)) break;
            tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_ALERT);
            std::cout << "Ngay khong hop le. Moi nhap lai!"; tui::resetColor();
            tui::gotoxy(dateX, dateY); std::cout << std::string(20, ' ');
        }
        struct Row { std::string maSach, isbn, ten; Date ngayMuon; int soNgay; };
        std::vector<Row> rows;
        for (MuonTraNode* p = pNode->info.mtHead; p; p = p->next) {
            if (p->trangThai == MT_DANG_MUON) {
                Row r;
                r.maSach = p->maSach;
                r.isbn = isbn_from_ms(p->maSach);
                if (const DauSach* ds = find_ds(r.isbn)) r.ten = ds->tenSach; else r.ten = "";
                r.ngayMuon = p->ngayMuon;
                r.soNgay = diff_days(today, r.ngayMuon);
                rows.push_back(r);
            }
        }
        tui::gotoxy(X0, y++); {
            std::string name = pNode->info.ho + std::string(" ") + pNode->info.ten;
            std::cout << "Doc gia: " << pNode->info.maThe << " | " << name;
        }
        y++;
        if (rows.empty()) {
            tui::gotoxy(X0, y++); std::cout << "(Khong co sach dang muon.)";
            tui::press_any_key_to_back(4, footerY - 1); return;
        }
        std::string header = PAD("STT", CW_STT) + " | "
            + PAD("MaSach", CW_MS) + " | "
            + PAD("ISBN", CW_ISBN) + " | "
            + PAD("Ten sach", CW_TEN) + " | "
            + PAD("Ngay muon", CW_NGAY) + " | "
            + PAD("So ngay", CW_SONG);
        std::string sep = DASH(CW_STT) + "-+-" + DASH(CW_MS) + "-+-" + DASH(CW_ISBN)
            + "-+-" + DASH(CW_TEN) + "-+-" + DASH(CW_NGAY) + "-+-" + DASH(CW_SONG);
        tui::gotoxy(X0, y++); std::cout << header;
        tui::gotoxy(X0, y++); std::cout << sep;
        for (size_t i = 0; i < rows.size(); ++i) {
            Row& r = rows[i];
            std::string tenCut = (int)r.ten.size() > CW_TEN ? r.ten.substr(0, CW_TEN) : r.ten;
            std::string line = PAD(std::to_string((int)i + 1), CW_STT) + " | "
                + PAD(r.maSach, CW_MS) + " | "
                + PAD(r.isbn, CW_ISBN) + " | "
                + PAD(tenCut, CW_TEN) + " | "
                + PAD(fmt_date(r.ngayMuon), CW_NGAY) + " | "
                + PAD(std::to_string(r.soNgay), CW_SONG);
            tui::gotoxy(X0, y++); std::cout << line;
            if (y > yMax) {
                tui::press_any_key_to_back(4, footerY - 1);
                tui::clearScreen(); tui::drawBox(2, 1, w, h, "MUON / TRA  >  DANH SACH DANG MUON CUA DOC GIA (tiep)");
                y = 5;
                tui::gotoxy(X0, y++); std::cout << header;
                tui::gotoxy(X0, y++); std::cout << sep;
            }
        }
        tui::press_any_key_to_back(4, footerY - 1);
    }

    //==================== Bảng in Độc giả ====================//
    static const int CW_MATHE = 9, CW_HODEM = 32, CW_TEN = 12, CW_GIOITINH = 9, CW_TRANGTHAI = 11;
    inline std::string _pad(const std::string& s, int w) {
        std::string t = s; if ((int)t.size() > w) t = t.substr(0, w);
        return t + std::string(w - (int)t.size(), ' ');
    }
    inline int _draw_docgia_table_header(const std::string& /*title*/, int /*w*/ = 118, int /*h*/ = 24) {
        int y = 5;
        tui::setColor(tui::FG_HL); 
        tui::gotoxy(4, y++); std::cout
            << _pad("Ma the", CW_MATHE) << " | "
            << _pad("Ho va Ten Dem", CW_HODEM) << " | "
            << _pad("Ten", CW_TEN) << " | "
            << _pad("Gioi tinh", CW_GIOITINH) << " | "
            << _pad("Trang thai", CW_TRANGTHAI);
        auto dash = [](int n) {return std::string(n, '-'); };
        tui::gotoxy(4, y++); std::cout
            << dash(CW_MATHE) << "-+-" << dash(CW_HODEM) << "-+-"
            << dash(CW_TEN) << "-+-" << dash(CW_GIOITINH) << "-+-"
            << dash(CW_TRANGTHAI);
        return y; 
    }
    inline void _ui_docgia_print_table(const std::vector<DocGia*>& rows, const std::string& title) {
        const int w = 118, h = 24;
        const int footerY = 1 + h - 2;
        const size_t PAGE = 15;
        const size_t total = rows.size();
        const size_t pages = (total == 0 ? 1 : (total + PAGE - 1) / PAGE);
        size_t page = 0;        
        tui::clearScreen();
        tui::drawBox(2, 1, w, h, title);
        tui::print_footer_hints(4, footerY, "[Up/Down] Trang truoc/sau   -   [Esc] Quay lai");
        tui::setColor(tui::FG_HL);         
        const int dataY = _draw_docgia_table_header(title, w, h);
        const int dataH = footerY - dataY - 1; 
        const int dataW = w - 8;          
        auto paint_page_hint = [&](size_t cur) {
            tui::gotoxy(4, 3);
            std::cout << "(Trang " << (int)(cur + 1) << "/" << (int)pages << ")"
                << std::string(20, ' ');
        };        
        auto paint_page = [&](size_t cur) {
            paint_page_hint(cur);
            tui::clear_rect(4, dataY, dataW, dataH);
            size_t startIdx = cur * PAGE;
            size_t endIdx = std::min(total, startIdx + PAGE);
            int y = dataY;
            for (size_t i = startIdx; i < endIdx; ++i) {
                const DocGia* dg = rows[i];
                tui::gotoxy(4, y++); std::cout
                    << _pad(std::to_string(dg->maThe), CW_MATHE) << " | "
                    << _pad(dg->ho, CW_HODEM) << " | "
                    << _pad(dg->ten, CW_TEN) << " | "
                    << _pad(dg->phai, CW_GIOITINH) << " | "
                    << _pad((dg->trangThaiThe == 1 ? "Hoat dong" : "Khoa"), CW_TRANGTHAI);
            }
        };        
        paint_page(page);              
        while (true) {
            tui::KeyEvent ev = tui::readKey();
            if (ev.key == tui::K_ESC) { return; }
            if (ev.key == tui::K_UP) { if (page > 0) { --page; paint_page(page); } }
            if (ev.key == tui::K_DOWN) { if (page + 1 < pages) { ++page; paint_page(page); } }
        }
    }
    inline void ui_dg_in_theo_ten_ho(DocGiaNode* root) {
        std::vector<DocGia*> v; duyet_LNR_luu_mang(root, v);
        std::sort(v.begin(), v.end(), [](const DocGia* a, const DocGia* b) {
            if (a->ten != b->ten) return a->ten < b->ten;
            if (a->ho != b->ho) return a->ho < b->ho;
            return a->maThe < b->maThe;
            });
        _ui_docgia_print_table(v, "QUAN LY DOC GIA  >  IN DANH SACH  (sap theo Ten + Ho)");
    }
    inline void ui_dg_in_theo_ma_the(DocGiaNode* root) {
        std::vector<DocGia*> v; duyet_LNR_luu_mang(root, v);
        std::sort(v.begin(), v.end(), [](const DocGia* a, const DocGia* b) { return a->maThe < b->maThe; });
        _ui_docgia_print_table(v, "QUAN LY DOC GIA  >  IN DANH SACH  (sap theo Ma the)");
    }

    //==================== Thống kê ====================//
    inline Date today_date() {
        std::time_t t = std::time(NULL); std::tm lt{};
#ifdef _WIN32
        localtime_s(&lt, &t);
#else
        std::tm* p = std::localtime(&t); if (p) lt = *p;
#endif
        Date d; d.d = lt.tm_mday; d.m = lt.tm_mon + 1; d.y = lt.tm_year + 1900; return d;
    }
    struct OverdueRow { int maThe = 0; std::string hoTen, maSach, ISBN, tenSach; Date ngayMuon{}; int days_over = 0; };
    inline const DauSach* _find_ds_by_isbn_const(const std::vector<DauSach*>& a, const std::string& isbn) {
        for (auto* ds : a) if (ds && ds->ISBN == isbn) return ds; return NULL;
    }

    //================ Top 10 sach mượn nhiều nhất =================//
    inline void tk_top10_so_luot_muon(std::vector<DauSach*>& dsArr) {
        const int w = 118, h = 24;
        const int footerY = 1 + h - 2;
        const int CW_STT = 4;
        const int CW_ISBN = 12;
        const int CW_TEN = 47;
        const int CW_CNT = 14;
        auto PAD = [](const std::string& s, int W) -> std::string {
            if ((int)s.size() >= W) return s.substr(0, W);
            return s + std::string(W - (int)s.size(), ' ');
            };
        auto LPAD = [](const std::string& s, int W) -> std::string {
            if ((int)s.size() >= W) return s.substr(0, W);
            return std::string(W - (int)s.size(), ' ') + s;
            };
        auto DASH = [](int n) -> std::string { return std::string(n, '-'); };
        std::vector<const DauSach*> a;
        for (auto* ds : dsArr) {
            if (ds != nullptr && ds->soLuotMuon > 0) {
                a.push_back(ds);
            }
        }
        std::sort(a.begin(), a.end(), [](const DauSach* L, const DauSach* R) {
            if (L->soLuotMuon != R->soLuotMuon) { return L->soLuotMuon > R->soLuotMuon; }
            if (L->tenSach != R->tenSach) { return L->tenSach < R->tenSach; }
            return L->ISBN < R->ISBN;
            });
        tui::clearScreen();
        tui::drawBox(2, 1, w, h, "Trang chu > Thong ke > Top 10 sach muon nhieu nhat");
        int y = 5;
        tui::gotoxy(4, y++); std::cout << "Top 10 theo so luot muon (giam dan):";
        tui::gotoxy(4, y++);
        std::cout << PAD("STT", CW_STT) << " | "
            << PAD("ISBN", CW_ISBN) << " | "
            << PAD("Ten sach", CW_TEN) << " | "
            << PAD("So luot muon", CW_CNT);
        tui::gotoxy(4, y++);
        std::cout << DASH(CW_STT) << "-+-" << DASH(CW_ISBN) << "-+-"
            << DASH(CW_TEN) << "-+-" << DASH(CW_CNT);
        if (a.empty()) {
            tui::gotoxy(4, y++); std::cout << "(Chua co sach nao duoc muon.)";
            tui::press_any_key_to_back(4, footerY - 1);
            return;
        }
        int out = 0;
        for (const DauSach* ds : a) {
            if (out >= 10) { break; }
            std::string tenCut = ds->tenSach;
            if ((int)tenCut.size() > CW_TEN) { tenCut = tenCut.substr(0, CW_TEN); }
            tui::gotoxy(4, y++);
            std::cout << PAD(std::to_string(out + 1) + ".", CW_STT) << " | "
                << PAD(ds->ISBN, CW_ISBN) << " | "
                << PAD(tenCut, CW_TEN) << " | "
                << LPAD(std::to_string(ds->soLuotMuon), CW_CNT);
            if (y >= footerY - 1) { break; }
            ++out;
        }
        tui::press_any_key_to_back(4, footerY - 1);
    }

    //================ Danh sách mượn sách qua hạn =================//
    inline void tk_ds_qua_han(std::vector<DauSach*>& dsArr, DocGiaNode* root) {
        const int w = 118, h = 24, X0 = 4, Y0 = 3;
        const int footerY = 1 + h - 2, yMax = 1 + h - 3;
        const int CW_STT = 4;
        const int CW_MATHE = 9;
        const int CW_MS = 13;
        const int CW_TEN = 47;
        const int CW_NGAY = 10;
        const int CW_TRE = 5;
        auto PAD = [](const std::string& s, int W)->std::string { return (int)s.size() >= W ? s.substr(0, W) : s + std::string(W - (int)s.size(), ' '); };
        auto LPAD = [](const std::string& s, int W)->std::string { return (int)s.size() >= W ? s.substr(0, W) : std::string(W - (int)s.size(), ' ') + s; };
        auto DASH = [](int n)->std::string { return std::string(n, '-'); };
        auto fmt_date = [](const Date& d)->std::string { char b[16]; std::snprintf(b, sizeof(b), "%02d/%02d/%04d", d.d, d.m, d.y); return std::string(b); };
        tui::clearScreen();
        tui::drawBox(2, 1, w, h, "Trang chu > Thong ke > Danh sach QUA HAN (giam dan so ngay tre)");
        int y = Y0 + 1;
        tui::gotoxy(X0, y); std::cout << "Nhap ngay hien tai (dd/mm/yyyy): ";
        int dateX = X0 + 34, dateY = y; y += 2;
        tui::print_footer_hints(4, footerY, "[Enter] Xac nhan  -  [Esc] Quay lai");
        _flush_input_nonblock();
        Date today{};
        while (true) {
            std::string s = _read_line_at(dateX, dateY, 16);
            today = parse_date_ddmmyyyy(s);
            if (is_valid_date(today)) break;
            tui::gotoxy(X0, footerY - 2); tui::setColor(tui::FG_ALERT); std::cout << "Ngay khong hop le. Moi nhap lai!"; tui::resetColor();
            tui::gotoxy(dateX, dateY); std::cout << std::string(20, ' ');
        }
        struct Row { int maThe; std::string maSach, tenSach; Date ngayMuon; int tre; };
        std::vector<DocGia*> dsDG; duyet_LNR_luu_mang(root, dsDG);
        std::vector<Row> rows;
        for (auto* dg : dsDG) {
            for (MuonTraNode* p = dg->mtHead; p; p = p->next) {
                if (p->trangThai != MT_DANG_MUON) continue;
                int soNgay = diff_days(today, p->ngayMuon);
                int tre = soNgay - HAN_MUON_NGAY;
                if (tre <= 0) continue;
                std::string isbn = isbn_from_masach(p->maSach);
                const DauSach* ds = _find_ds_by_isbn_const(dsArr, isbn);
                Row r; r.maThe = dg->maThe; r.maSach = p->maSach; r.tenSach = ds ? ds->tenSach : "";
                r.ngayMuon = p->ngayMuon; r.tre = tre;
                rows.push_back(r);
            }
        }
        std::sort(rows.begin(), rows.end(), [](const Row& a, const Row& b) {
            if (a.tre != b.tre) return a.tre > b.tre;
            if (a.maThe != b.maThe) return a.maThe < b.maThe;
            return a.maSach < b.maSach;
            });
        tui::gotoxy(X0, y++);
        std::cout << PAD("STT", CW_STT) << " | "
            << PAD("MaThe", CW_MATHE) << " | "
            << PAD("MaSach", CW_MS) << " | "
            << PAD("Ten sach", CW_TEN) << " | "
            << PAD("Ngay muon", CW_NGAY) << " | "
            << PAD("Tre", CW_TRE);
        tui::gotoxy(X0, y++);
        std::cout << DASH(CW_STT) << "-+-" << DASH(CW_MATHE) << "-+-" << DASH(CW_MS)
            << "-+-" << DASH(CW_TEN) << "-+-" << DASH(CW_NGAY) << "-+-" << DASH(CW_TRE);
        if (rows.empty()) {
            tui::gotoxy(X0, y++); std::cout << "(Khong co phieu qua han.)";
            tui::press_any_key_to_back(4, footerY - 1); return;
        }
        for (size_t i = 0; i < rows.size(); ++i) {
            const Row& r = rows[i];
            std::string tenCut = r.tenSach.size() > (size_t)CW_TEN ? r.tenSach.substr(0, CW_TEN) : r.tenSach;
            tui::gotoxy(X0, y++);
            std::cout << PAD(std::to_string((int)i + 1), CW_STT) << " | "
                << PAD(std::to_string(r.maThe), CW_MATHE) << " | "
                << PAD(r.maSach, CW_MS) << " | "
                << PAD(tenCut, CW_TEN) << " | "
                << PAD(fmt_date(r.ngayMuon), CW_NGAY) << " | "
                << LPAD(std::to_string(r.tre), CW_TRE);
            if (y > yMax) {
                tui::press_any_key_to_back(4, footerY - 1);
                tui::clearScreen();
                tui::drawBox(2, 1, w, h, "Trang chu > Thong ke > Danh sach QUA HAN (tiep)");
                y = 5;
                tui::gotoxy(X0, y++);
                std::cout << PAD("STT", CW_STT) << " | "
                    << PAD("MaThe", CW_MATHE) << " | "
                    << PAD("MaSach", CW_MS) << " | "
                    << PAD("Ten sach", CW_TEN) << " | "
                    << PAD("Ngay muon", CW_NGAY) << " | "
                    << PAD("Tre", CW_TRE);
                tui::gotoxy(X0, y++);
                std::cout << DASH(CW_STT) << "-+-" << DASH(CW_MATHE) << "-+-" << DASH(CW_MS)
                    << "-+-" << DASH(CW_TEN) << "-+-" << DASH(CW_NGAY) << "-+-" << DASH(CW_TRE);
            }
        }
        tui::press_any_key_to_back(4, footerY - 1);
    }

    //==================== Menus ====================//
    inline void submenu_doc_gia(std::vector<DauSach*>& dsArr, DocGiaNode*& root) {
        (void)dsArr; const std::string title = "QUAN LY DOC GIA";
        while (true) {
            int ch = menu_mui_ten(8, 0, title, {
                "1. Them doc gia (ma the tu sinh)",
                "2. Xoa doc gia theo ma the",
                "3. Cap nhat doc gia theo ma the",
                "4. In danh sach doc gia theo Ten + Ho",
                "5. In danh sach doc gia theo Ma the",
                "0. Quay lai"
                });
            if (ch < 0) return; if (ch == 5) return;
            switch (ch) {
            case 0: form_them_doc_gia_tui(root); break;
            case 1: form_xoa_doc_gia_tui(root);  break;
            case 2: form_sua_doc_gia_tui(root);  break;
            case 3: ui_dg_in_theo_ten_ho(root);  break;
            case 4: ui_dg_in_theo_ma_the(root);  break;
            default: break;
            }
        }
    }
    inline void submenu_dau_sach(std::vector<DauSach*>& dsArr, DocGiaNode*& root) {
        const std::string title = "QUAN LY DAU SACH";
        while (true) {
            int ch = menu_mui_ten(8, 0, title, {
                "1. Them dau sach",
                "2. Xoa dau sach",
                "3. Cap nhat thong tin dau sach",
                "4. In danh sach theo the loai (kem ISBN)",
                "5. Tim dau sach theo ten",
                "0. Quay lai"
                });
            if (ch < 0) return; if (ch == 5) return;
            switch (ch) {
            case 0: form_them_dau_sach_tui(dsArr);      break;
            case 1: form_xoa_dau_sach_tui(dsArr, root); break;
            case 2: form_cap_nhat_dau_sach_tui(dsArr);  break;
            case 3: ui_ds_in_theo_the_loai(dsArr);      break;
            case 4: ui_ds_tim_theo_ten(dsArr);          break;
            default: break;
            }
        }
    }
    inline void submenu_muon_tra(std::vector<DauSach*>& dsArr, DocGiaNode*& root) {
        const std::string title = "MUON / TRA";
        while (true) {
            int ch = menu_mui_ten(8, 0, title, {
                "1. Lap phieu muon (toi da 3 cuon / doc gia)",
                "2. Tra sach",
                "3. In danh sach DANG MUON cua doc gia",
                "0. Quay lai"
                });
            if (ch < 0) return; if (ch == 3) return;
            switch (ch) {
            case 0: form_muon_sach_tui(dsArr, root); break;
            case 1: form_tra_sach_tui(dsArr, root); break;
            case 2: form_in_dang_muon_doc_gia_tui(dsArr, root); break;
            default: break;
            }
        }
    }
    inline void submenu_thong_ke(std::vector<DauSach*>& dsArr, DocGiaNode*& root) {
        const std::string title = "THONG KE";
        while (true) {
            int ch = menu_mui_ten(8, 0, title, {
                "1. Top 10 sach MUON nhieu nhat",
                "2. Danh sach QUA HAN (giam dan theo ngay tre)",
                "0. Quay lai"
                });
            if (ch < 0) return; if (ch == 2) return;
            switch (ch) {
            case 0: tk_top10_so_luot_muon(dsArr); break;
            case 1: tk_ds_qua_han(dsArr, root);    break;
            default: break;
            }
        }
    }
    inline void menu_main_tui(std::vector<DauSach*>& dsArr, DocGiaNode*& root) {
        const std::string title = "QUAN LY THU VIEN - MENU CHINH";
        while (true) {
            int ch = menu_mui_ten(8, 0, title, {
                "1. Quan ly doc gia",
                "2. Quan ly dau sach",
                "3. Muon / Tra",
                "4. Thong ke",
                "0. Thoat"
                });
            if (ch < 0) return; if (ch == 4) return;
            switch (ch) {
            case 0: submenu_doc_gia(dsArr, root); break;
            case 1: submenu_dau_sach(dsArr, root); break;
            case 2: submenu_muon_tra(dsArr, root); break;
            case 3: submenu_thong_ke(dsArr, root); break;
            default: break;
            }
        }
    }
}
