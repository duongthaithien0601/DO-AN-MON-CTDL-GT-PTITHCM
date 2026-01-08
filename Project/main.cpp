#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include "cautruc.h"
#include "dsdms.h"
#include "dsdocgia.h"
#include "dsdausach.h"
#include "dsmuontra.h"
#include "luutru.h"
#include "menu_tui.h"

int main() {
    // 1. Tối ưu hóa tốc độ nhập/xuất (I/O)    
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    // 2. Khởi tạo cấu trúc dữ liệu chính
    DanhSachDauSach dsArr; 
    DocGiaNode* root = NULL; 
    // 3. Nạp dữ liệu từ File    
    if (!load_all_data(dsArr, root)) {
        std::cout << "Tai du lieu that bai. Chuong trinh se khoi dong voi CSDL rong.\n";
        dsArr.n = 0;
        root = NULL;
    }
    // 4. Chạy giao diện Menu chính 
    menutui::menu_main_tui(dsArr, root);
    // 5. Lưu dữ liệu trước khi thoát    
    if (save_all_data(dsArr, root)) {
        std::cout << "Da luu du lieu. Tam biet!\n";
    }
    else {
        std::cout << "Luu du lieu that bai!\n";
    }
    // 6. Dọn dẹp bộ nhớ (Clean up)    
    giai_phong_danh_sach_dausach(dsArr);
    giai_phong_cay_doc_gia(root);
    return 0;
}