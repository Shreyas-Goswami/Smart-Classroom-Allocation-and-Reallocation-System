#include "interval_tree.h"
#include "mem_track.h"
#include <stddef.h>

static int key_less(int s1, int c1, int s2, int c2) {
    return s1 < s2 || (s1 == s2 && c1 < c2);
}

static void update_max(IntervalNode *n) {
    n->max_end = n->end;
    if (n->left  && n->left->max_end  > n->max_end) n->max_end = n->left->max_end;
    if (n->right && n->right->max_end > n->max_end) n->max_end = n->right->max_end;
}

IntervalNode *interval_insert(IntervalNode *root, int start, int end, int class_id) {
    if (!root) {
        IntervalNode *n = tracked_malloc(sizeof *n);
        n->start = start; n->end = end; n->class_id = class_id;
        n->max_end = end; n->left = n->right = NULL;
        return n;
    }
    if (key_less(start, class_id, root->start, root->class_id))
        root->left = interval_insert(root->left, start, end, class_id);
    else
        root->right = interval_insert(root->right, start, end, class_id);
    update_max(root);
    return root;
}

IntervalNode *interval_search_overlap(const IntervalNode *root, int start, int end) {
    while (root) {
        if (root->start < end && start < root->end) return (IntervalNode *)root;
        if (root->left && root->left->max_end > start) root = root->left;
        else root = root->right;
    }
    return NULL;
}

IntervalNode *interval_delete(IntervalNode *root, int start, int class_id, int *deleted) {
    if (!root) return NULL;

    if (start == root->start && class_id == root->class_id) {
        if (deleted) *deleted = 1;
        if (!root->left)  { IntervalNode *r = root->right; tracked_free(root); return r; }
        if (!root->right) { IntervalNode *l = root->left;  tracked_free(root); return l; }
        IntervalNode *succ = root->right;
        while (succ->left) succ = succ->left;
        root->start = succ->start; root->end = succ->end; root->class_id = succ->class_id;
        int dummy = 0;
        root->right = interval_delete(root->right, succ->start, succ->class_id, &dummy);
    } else if (key_less(start, class_id, root->start, root->class_id)) {
        root->left = interval_delete(root->left, start, class_id, deleted);
    } else {
        root->right = interval_delete(root->right, start, class_id, deleted);
    }
    update_max(root);
    return root;
}

void interval_free_tree(IntervalNode *root) {
    if (!root) return;
    interval_free_tree(root->left);
    interval_free_tree(root->right);
    tracked_free(root);
}