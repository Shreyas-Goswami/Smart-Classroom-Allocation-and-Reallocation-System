#ifndef INTERVAL_TREE_H
#define INTERVAL_TREE_H

typedef struct IntervalNode {
    int start;
    int end;
    int class_id;
    int max_end;
    struct IntervalNode *left, *right;
} IntervalNode;

IntervalNode *interval_insert(IntervalNode *root, int start, int end, int class_id);
IntervalNode *interval_search_overlap(const IntervalNode *root, int start, int end);
IntervalNode *interval_delete(IntervalNode *root, int start, int class_id, int *deleted);
void interval_free_tree(IntervalNode *root);

#endif