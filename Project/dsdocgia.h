#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>
#include "cautruc.h"

// ========================= KHAI BAO CAY DOC GIA (BST) =========================

struct DocGiaNode {
    DocGia info;
    DocGiaNode* left;
    DocGiaNode* right;

    explicit DocGiaNode(const DocGia& v) : info(v), left(NULL), right(NULL) {
    }
};

// ========================= TIEN ICH NOI BO (BST) =========================

inline DocGiaNode* _find_min(DocGiaNode* root) {
    if (root == NULL) {
        return NULL;
    }
    while (root->left != NULL) {
        root = root->left;
    }
    return root;
}

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

// ========================= HAM LÕI PUBLIC =========================
// Tra ve con tro node co maThe; NULL neu khong co
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

// Chen DocGia theo khoa maThe (neu da ton tai -> khong chen de tranh mat du lieu)
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
        else {
            // Trung maThe -> bo qua (khong cap nhat de tranh mat lich su mtHead)
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

// Dem so ban ghi muon co trang thai DANG MUON cua 1 doc gia
inline int dem_mt_dang_muon(const DocGia& dg) {
    int cnt = 0;
    for (MuonTraNode* p = dg.mtHead; p != NULL; p = p->next) {
        if (p->trangThai == MT_DANG_MUON) {
            cnt++;
        }
    }
    return cnt;
}

// Xoa doc gia chi khi khong con muon sach (MT_DANG_MUON)
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

// Duyet LNR va luu con tro DocGia* vao vector (de TUI sap xep/in bang)
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

// Sinh maThe 6 chu so khong trung (uu tien random, fallback quet tuan tu)
inline int gen_ma_the_unique(DocGiaNode* root) {
    // Pham vi: 100000..999999 (6 chu so)
    static std::mt19937 rng(static_cast<unsigned>(std::time(NULL)));
    std::uniform_int_distribution<int> dist(100000, 999999);
    // Thu ngau nhien mot so lan
    for (int i = 0; i < 2048; i++) {
        int cand = dist(rng);
        if (!_exists_ma_the(root, cand)) {
            return cand;
        }
    }
    // Fallback: quet tuan tu
    for (int cand = 100000; cand <= 999999; cand++) {
        if (!_exists_ma_the(root, cand)) {
            return cand;
        }
    }
    // Khong xay ra trong thuc te
    return 999999;
}

//Dem tong so node tren cay — chua duoc TUI goi, de phong sau nay can
inline int dem_tong_doc_gia(DocGiaNode* root) {
    if (root == NULL) {
        return 0;
    }
    return 1 + dem_tong_doc_gia(root->left) + dem_tong_doc_gia(root->right);
}



