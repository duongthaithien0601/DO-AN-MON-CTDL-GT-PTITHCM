#include <iostream>
#include <vector>
#include <string>

#include "cautruc.h"
#include "dsdms.h"
#include "dsdocgia.h"
#include "dsdausach.h"
#include "dsmuontra.h"
#include "luutru.h"
#include "menu_tui.h"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::vector<DauSach*> dsArr;
    DocGiaNode* root = NULL;

    if (!load_all_data(dsArr, root)) {
        std::cout << "Tai du lieu that bai. Chuong trinh se khoi dong voi CSDL rong.\n";
        dsArr.clear();
        root = NULL;
    }

    // Gọi đúng hàm trong UI (giữ nguyên menu_tui.h)
    menutui::menu_main_tui(dsArr, root);
    
    // Lưu & giải phóng
    if (save_all_data(dsArr, root)) {
        std::cout << "Da luu du lieu. Tam biet!\n";
    }
    else {
        std::cout << "Luu du lieu that bai!\n";
    }

    giai_phong_vector_dausach(dsArr);
    giai_phong_cay_doc_gia(root);
    return 0;
}
