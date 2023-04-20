//===----------------------------------------------------------------------===//
//
//                         Rutgers CS539 - Database System
//                         ***DO NO SHARE PUBLICLY***
//
// Identification:   include/b_plus_tree.h
//
// Copyright (c) 2022, Rutgers University
//
//===----------------------------------------------------------------------===//
#pragma once

#include <queue>
#include <string>
#include <vector>
#include "para.h"
#include <iostream>

using namespace std;
using std::cout;

// Value structure we insert into BPlusTree
struct RecordPointer
{
    int page_id;
    int record_id;
    RecordPointer() : page_id(0), record_id(0) {};
    RecordPointer(int page, int record) : page_id(page), record_id(record) {};
};

// BPlusTree Node
class Node
{
public:
    Node(bool leaf) : key_num(0), is_leaf(leaf) {};
    bool is_leaf;
    int key_num;
    KeyType keys[MAX_FANOUT - 1];
    bool hasparent= false;
    Node* parent=nullptr;
    //Node* children[MAX_FANOUT];
};

// internal b+ tree node
class InternalNode : public Node
{
public:
    InternalNode() : Node(false) {};
    Node* children[MAX_FANOUT];
};

class LeafNode : public Node
{
public:
    LeafNode() : Node(true) {};
    RecordPointer pointers[MAX_FANOUT - 1];
    // pointer to the next/prev leaf node
    LeafNode* next_leaf = NULL;
    LeafNode* prev_leaf = NULL;
};

/**
 * Main class providing the API for the Interactive B+ Tree.
 *
 * Implementation of simple b+ tree data structure where internal pages direct
 * the search and leaf pages contain record pointers
 * (1) We only support (and test) UNIQUE key
 * (2) Support insert & remove
 * (3) Support range scan, return multiple values.
 * (4) The structure should shrink and grow dynamically
 */

class BPlusTree
{
public:
    BPlusTree() {};

    // Returns true if this B+ tree has no keys and values
    bool IsEmpty() const;

    // Insert a key-value pair into this B+ tree.
    bool Insert(const KeyType& key, const RecordPointer& value);
    
    //to fetch the index of where a key gets inserted/deleted from
    int searchIndex(const KeyType& key, KeyType(&keys)[MAX_FANOUT - 1], int n);

    //helper function, for splitting leaf node condition.
    LeafNode* splitLeaf(LeafNode* curNode);

    //helpder function, to recursively add in internal node of the tree, if any node is inserted in leaf node.
    void insertInternal(InternalNode* curNode, Node* childNode, int keyValue);

    //helper function, to just insert in leafnode
    void insert_in_leaf(LeafNode* curNode, const KeyType& key, const RecordPointer& value, int index);

    // Remove a key and its value from this B+ tree.
    void Remove(const KeyType& key);

    // return the value associated with a given key
    bool GetValue(const KeyType& key, RecordPointer& result);

    // return the values within a key range [key_start, key_end) not included key_end
    void RangeScan(const KeyType& key_start, const KeyType& key_end,
        std::vector<RecordPointer>& result);
    
    //method overloaded func, to return if a value exists in the tree or not.
    int searchIndex(const KeyType& key, KeyType(&keys)[MAX_FANOUT - 1], int n, bool& contains);
    
    // re-evaluating parent keys, once changes have been made to children nodes.
    void reevaluateParent(InternalNode* node, const int key,int val);

    //merging/shrinking the tree; helper for remove()
    void reorganizeNodes(InternalNode* node);

    //visualizing tree, for debugging only.
    void print();

private:
    // pointer to the root node.
    Node* root = nullptr;

    void printTree(Node* ptr);
};
