#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>
#include "cautruc.h"

// ========================= TIỆN ÍCH =========================
// Tìm node có giá trị nhỏ nhất trong cây con
inline DocGiaNode* _find_min(DocGiaNode* root) {
    if (root == NULL) {
        return NULL;
    }
    while (root->left != NULL) {
        root = root->left;
    }
    return root;
}
// Xóa node có maThe khỏi cây
inline void _delete_node(DocGiaNode*& root, int maThe) {
    if (root == NULL) {
        return;
    }
    if (maThe < root->info.maThe) {
        _delete_node(root->left, maThe);
    }
    else if (maThe > root->info.maThe) {
        _delete_node(root->right, maThe);
    }
    else {        
        DocGiaNode* del = root;
        if (root->left == NULL) {
            root = root->right;
            delete del;
        }
        else if (root->right == NULL) {
            root = root->left;
            delete del;
        }
        else {            
            DocGiaNode* succ = _find_min(root->right);
            root->info = succ->info; 
            _delete_node(root->right, succ->info.maThe);
        }
    }
}

// Tìm node doc gia theo maThe
inline DocGiaNode* tim_node_doc_gia(DocGiaNode* root, int maThe) {
    DocGiaNode* p = root;
    while (p != NULL) {
        if (maThe < p->info.maThe) {
            p = p->left;
        }
        else if (maThe > p->info.maThe) {
            p = p->right;
        }
        else {
            return p;
        }
    }
    return NULL;
}
// Chèn một doc gia mới vào cây (theo maThe)
inline void insert_doc_gia(DocGiaNode*& root, const DocGia& v) {
    if (root == NULL) {
        root = new DocGiaNode(v);
        return;
    }
    DocGiaNode* p = root;
    DocGiaNode* parent = NULL;
    while (p != NULL) {
        parent = p;
        if (v.maThe < p->info.maThe) {
            p = p->left;
        }
        else if (v.maThe > p->info.maThe) {
            p = p->right;
        }
        else { // Trung maThe -> bo qua (khong cap nhat de tranh mat du lieu)           
            return;
        }
    }
    if (v.maThe < parent->info.maThe) {
        parent->left = new DocGiaNode(v);
    }
    else {
        parent->right = new DocGiaNode(v);
    }
}
// Dem so luong sach dang muon cua doc gia
inline int dem_mt_dang_muon(const DocGia& dg) {
    int cnt = 0;
    for (MuonTraNode* p = dg.mtHead; p != NULL; p = p->next) {
        if (p->trangThai == MT_DANG_MUON) {
            cnt++;
        }
    }
    return cnt;
}
// Xoa doc gia chi khi khong con muon sach 
inline bool xoa_doc_gia_if_no_borrowing(DocGiaNode*& root, int maThe) {
    DocGiaNode* p = tim_node_doc_gia(root, maThe);
    if (p == NULL) {
        return false;
    }
    if (dem_mt_dang_muon(p->info) > 0) {
        return false;
    }
    _delete_node(root, maThe);
    return true;
}
// Duyet cay LNR va luu cac con tro DocGia vao mang
inline void duyet_LNR_luu_mang(DocGiaNode* root, std::vector<DocGia*>& out) {
    if (root == NULL) {
        return;
    }
    duyet_LNR_luu_mang(root->left, out);
    out.push_back(&root->info);
    duyet_LNR_luu_mang(root->right, out);
}
// Kiem tra maThe da ton tai tren cay
inline bool _exists_ma_the(DocGiaNode* root, int maThe) {
    return tim_node_doc_gia(root, maThe) != NULL;
}
// Sinh maThe ngau nhien va khong trung tren cay
inline int gen_ma_the_unique(DocGiaNode* root) {    
    static std::mt19937 rng(static_cast<unsigned>(std::time(NULL)));
    std::uniform_int_distribution<int> dist(100000, 999999);   
    for (int i = 0; i < 2048; i++) {
        int cand = dist(rng);
        if (!_exists_ma_the(root, cand)) {
            return cand;
        }
    }   
    for (int cand = 100000; cand <= 999999; cand++) {
        if (!_exists_ma_the(root, cand)) {
            return cand;
        }
    }    
    return 999999;
}
//Dem tong so node tren cay 
inline int dem_tong_doc_gia(DocGiaNode* root) {
    if (root == NULL) {
        return 0;
    }
    return 1 + dem_tong_doc_gia(root->left) + dem_tong_doc_gia(root->right);
}
