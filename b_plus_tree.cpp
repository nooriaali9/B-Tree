
#include "include/b_plus_tree.h"
using namespace std;
/*
 * Helper function to decide whether current b+tree is empty
 */
bool BPlusTree::IsEmpty() const 
{ 
    return root == nullptr;
}

/*****************************************************************************
 * SEARCH
 *****************************************************************************/
/*
 * Return the only value that associated with input key
 * This method is used for point query
 * @return : true means key exists
 */
bool BPlusTree::GetValue(const KeyType &key, RecordPointer &result)
{
    if (IsEmpty()) { 
        return false;
    }
    Node *cursor = root;
    while (!cursor->is_leaf)
    {
        int index = searchIndex(key, cursor->keys, cursor->key_num); //to find which child to go into
        cursor = ((InternalNode *)cursor)->children[index];
    }

    LeafNode *current_leaf_node = static_cast<LeafNode *>(cursor);

    for (int i = 0; i < current_leaf_node->key_num; i++) // iterating in the leaf node, val could be present & storing said value, in result.
    {
        if (current_leaf_node->keys[i] == key)
        {
            result = current_leaf_node->pointers[i];
            return true;
        }
    }
    return false;
}

int BPlusTree::searchIndex(const KeyType& key, KeyType(&keys)[MAX_FANOUT - 1], int n)
{
    int low = 0, high = n - 1;
    while (low <= high)
    {
        int mid = (low + high) / 2;
        if (keys[mid] <= key)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return low;
}

int BPlusTree::searchIndex(const KeyType& key, KeyType(&keys)[MAX_FANOUT - 1], int n, bool& contains) // method overload, to check if a value exists in a tree or not.
{
    int low = 0, high = n;
    while (low < high)
    {
        int mid = (low + high) / 2;
        if (keys[mid] == key)
        {
            contains = true;
            return mid;
        }
        else if (keys[mid] > key)
            high = mid;
        else
            low = mid + 1;
    }
    return low;
}

//splitting node, when leaf node is full.
LeafNode *BPlusTree::splitLeaf(LeafNode *curNode) 

{
    LeafNode *newNode = new LeafNode(); //new node to store remaining values

    int splitPoint = ceil((double)(MAX_FANOUT - 1) / 2); 
    newNode->is_leaf = true;
    newNode->next_leaf = curNode->next_leaf;
    
    if (newNode->next_leaf != nullptr) {
        newNode->next_leaf->prev_leaf = newNode;
    }

    curNode->next_leaf = newNode;
    newNode->prev_leaf = curNode;
    
    int writeHead = 0;
    for (int i = splitPoint; i < curNode->key_num; i++)
    {
        newNode->keys[writeHead] = curNode->keys[i];
        newNode->pointers[writeHead] = curNode->pointers[i];
        newNode->key_num += 1;
        writeHead++;
    }

    curNode->key_num = splitPoint;
    return newNode;
}

//helper function to insert node inside a non-leaf node
void BPlusTree::insertInternal(InternalNode* curNode, Node* childNode, int keyValue)
{
    if (curNode->key_num == MAX_FANOUT - 1) // if key_num reaches max threshold, split.
    {
        int insertionPoint = 0;
        while (insertionPoint < MAX_FANOUT - 1 && keyValue > curNode->keys[insertionPoint])
        {
            insertionPoint++;
        }

        KeyType arbitaryKeys[MAX_FANOUT];
        Node* arbitaryChildren[MAX_FANOUT + 1];

        for (int i = 0; i < MAX_FANOUT; i++) //storing values in arrays, to further sort while splitting
        {
            if (i < MAX_FANOUT - 1)
            {
                arbitaryKeys[i] = curNode->keys[i];
            }
            arbitaryChildren[i] = curNode->children[i];
        }

        for (int k = MAX_FANOUT - 1; k > insertionPoint; k--)
        {
            arbitaryKeys[k] = arbitaryKeys[k - 1];
        }

        for (int k = MAX_FANOUT; k > insertionPoint + 1; k--)
        {
            arbitaryChildren[k] = arbitaryChildren[k - 1];
        }

        arbitaryKeys[insertionPoint] = keyValue;
        arbitaryChildren[insertionPoint + 1] = childNode;

        InternalNode* newNode = new InternalNode();
        newNode->is_leaf = false;

        int splitPoint = ceil((double)(MAX_FANOUT - 1) / 2);
        int writeHead = 0;
        for (int i = splitPoint + 1; i < MAX_FANOUT + 1; i++)
        {
            arbitaryChildren[i]->parent = newNode;  //updating to newnode
            newNode->children[writeHead++] = arbitaryChildren[i]; 
        }
        childNode->hasparent = true;
        if (insertionPoint + 1 < splitPoint + 1) 
        {
            childNode->parent = curNode;
        }
        else
        {
            childNode->parent = newNode;
        }

        writeHead = 0;
        for (int i = splitPoint + 1; i < MAX_FANOUT; i++)
        {
            newNode->keys[writeHead++] = arbitaryKeys[i];
            newNode->key_num += 1;
        }

        curNode->key_num = splitPoint;

        if (curNode == root)
        {
            // Split current root node which is leaf into two leaf and set new root (Internal Node)
            InternalNode* inode = new InternalNode();
            root = inode;
            inode->hasparent = false;
            inode->is_leaf = false;

            inode->key_num += 1;
            // Set left, right pointer and a single key (New Internal Node)
            inode->children[0] = curNode;
            inode->keys[0] = curNode->keys[splitPoint];
            inode->children[1] = newNode;

            curNode->parent = inode;
            curNode->hasparent = true;

            newNode->parent = inode;
            newNode->hasparent = true;
        }
        else
        {
            insertInternal((InternalNode*)curNode->parent, newNode, curNode->keys[splitPoint]); //inserting in internal node, recursively until all parent nodes are updated.
        }
    }
    else
    {
        // There is space in Internal Node to accomodate new node
        int index = 0;
        while (childNode->keys[0] > curNode->keys[index] && index < curNode->key_num)
        {
            index++;
        }

        for (int i = curNode->key_num + 1; i > index; i--) //shifting keys & children; to accomodate the new key & child ptr.
        {
            if (i <= curNode->key_num)
            {
                curNode->keys[i] = curNode->keys[i - 1];
            }
            curNode->children[i] = curNode->children[i - 1];
        }

        curNode->keys[index] = keyValue;
        curNode->key_num += 1;
        curNode->children[index + 1] = childNode;

        childNode->hasparent = true;
        childNode->parent = curNode;
    }
}

void BPlusTree::insert_in_leaf(LeafNode *curNode, const KeyType &key, const RecordPointer &value, int index)
{
    if (curNode->key_num == MAX_FANOUT - 1)
    {

        // Leaf Node has reached capacity split the leaf Node
        LeafNode *newNode = splitLeaf(curNode);
        int splitPoint = ceil((double)(MAX_FANOUT - 1) / 2);
        if (index < splitPoint)
        {
            // Insertion should happen in the old leaf node
            insert_in_leaf(curNode, key, value, index);
        }
        else
        {
            // Insertion should happen to the new leaf node
            insert_in_leaf(newNode, key, value, index - (splitPoint));
        }

        if (curNode->hasparent)
        {
            // Write Push up logic to insert in internal Node
            insertInternal((InternalNode *)curNode->parent, (Node *)newNode, newNode->keys[0]);
        }
        else
        {
            // Split current root node which is leaf into two leaf and set new root (Internal Node)
            InternalNode *inode = new InternalNode();
            root = inode;
            inode->hasparent = false;
            inode->is_leaf = false;

            inode->key_num += 1;
            // Set left, right pointer and a single key (New Internal Node)
            inode->children[0] = curNode;

            inode->keys[0] = newNode->keys[0];

            inode->children[1] = newNode;

            curNode->parent = inode;
            curNode->hasparent = true;

            newNode->parent = inode;
            newNode->hasparent = true;
        }
    }
    else
    {
        // We have space in Leaf node to accomadate new insertion
        for (int i = curNode->key_num; i > index; i--) //shifting keys & pointers to insert new value
        {
            curNode->keys[i] = curNode->keys[i - 1];
            curNode->pointers[i] = curNode->pointers[i - 1];
        }
        curNode->keys[index] = key;
        curNode->pointers[index] = value;
        curNode->key_num += 1;
    }
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Insert constant key & value pair into b+ tree
 * If current tree is empty, start new tree, otherwise insert into leaf Node.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
bool BPlusTree::Insert(const KeyType &key, const RecordPointer &value)
{
  
    if (root == nullptr) // creating an empty leaf node, which is the root.
    {
        // added new
        LeafNode *newroot = new LeafNode();
        insert_in_leaf(newroot, key, value, 0);
        newroot->hasparent = false;
        newroot->is_leaf = true;
        root = newroot;

        return true;
    }
    else
    {
        Node *cursor = root;
        bool contains  = false;
        int index = -1;
        while (!cursor->is_leaf)
        {

            index = searchIndex(key, cursor->keys, cursor->key_num); //getting index of child
            cursor = ((InternalNode *)cursor)->children[index];
        }

        index = searchIndex(key, cursor->keys, cursor->key_num,contains); //getting index of key in leaf node
        
        if (contains) return false;

        LeafNode *current_leaf_node = static_cast<LeafNode *>(cursor);
        insert_in_leaf(current_leaf_node, key, value, index); 

        return true;
    }

}

void BPlusTree::print()
{
    printTree(root);
}

void BPlusTree::printTree(Node *tree)
{
    cout << endl;
    int i;

    if (tree != nullptr)
    {
        
        for (int i = 0; i < tree->key_num; i++)
        {
            cout << tree->keys[i] << " ";
        }
        cout << endl;

        if (!tree->is_leaf)
        {
            InternalNode* inode = static_cast<InternalNode*>(tree);
            for (i = 0; i <= inode->key_num; i++)
            {
                printTree(inode->children[i]);
            }
        }
        else
        {
            LeafNode* leafNode = static_cast<LeafNode*>(tree);
            cout << "RPs = ";
            for (int i = 0; i < tree->key_num; i++)
            {
                cout << leafNode->pointers[i].page_id << " " << leafNode->pointers[i].record_id << " | ";
            }
            cout << endl;
        }
    }
    else
    {
        cout << "tree is empty" << endl;
    }

    cout << endl;
}

//helper, to rearrage keys in parent node; once changes have been made to children node.
void BPlusTree::reevaluateParent(InternalNode* cursor, const int key,int val) {

    if (cursor == nullptr)
    { return; }

    bool contains = false;
    int index = searchIndex(key, cursor->keys, cursor->key_num, contains); 
    
    if (contains) { return; }

    if (index != 0) {
        cursor->keys[index - 1] = key;
    }
    else if (cursor->parent != nullptr) {
        reevaluateParent(static_cast<InternalNode*>(cursor->parent), key,val);
    }
}

//helper, to reorganize nodes; in a tree once node has been deleted from a leaf node.
void BPlusTree::reorganizeNodes(InternalNode* curr_node) {
    
    if (curr_node == root) {
        
        if (curr_node->key_num == 0) {  // if curr_node has no elements; set root to empty.
            root = curr_node->children[0];
            curr_node->children[0]->parent = nullptr;
        }
        return;
    }

    int node_size = (MAX_FANOUT - 1) / 2;

    if (curr_node->key_num >= node_size) { return; }

    InternalNode* parent = static_cast<InternalNode*>(curr_node->parent);
    
    bool contains = false;
    int index = searchIndex(curr_node->keys[0],parent->keys, parent->key_num, contains); //index of the child, where the key would exist
    
    if (contains) { index++; }

    if (index - 1 >= 0 && parent->children[index - 1]->key_num > node_size) { //checking if left node has enough values to merge.

        InternalNode* left_child = static_cast<InternalNode*>(parent->children[index - 1]); //taking left child

        int last_key = left_child->keys[left_child->key_num - 1]; //storing last key from left child
        Node* last_child_ptr = left_child->children[left_child->key_num]; //storing last ptr from left child    
        left_child->key_num--;

        for (int i = curr_node->key_num - 1; i >= 0; i--) { //moving keys & child pointers.
            curr_node->keys[i + 1] = curr_node->keys[i];
            curr_node->children[i + 2] = curr_node->children[i + 1]; 
        }
       
        curr_node->children[1] = curr_node->children[0];
        curr_node->keys[0] = parent->keys[index - 1];
        curr_node->children[0] = last_child_ptr;
        last_child_ptr->parent = curr_node;
        curr_node->key_num++;

        parent->keys[index - 1] = last_key;
    }
    else if (index + 1 <= parent->key_num && parent->children[index + 1]->key_num > node_size) { //checking if right node has enough keys to merge.
        
        InternalNode* right_child = static_cast<InternalNode*>(parent->children[index + 1]); //taking right child
        int first_key = right_child->keys[0];

        //moving first val from right node, to end of curr_node
        curr_node->keys[curr_node->key_num] = parent->keys[index];
        curr_node->children[curr_node->key_num + 1] = right_child->children[0];
        curr_node->children[curr_node->key_num + 1]->parent = curr_node;
        curr_node->key_num++;


        for (int i = 1; i < right_child->key_num; i++) { //updating right_node, after moving its first value.
            right_child->keys[i - 1] = right_child->keys[i];
            right_child->children[i - 1] = right_child->children[i];
        }
        right_child->children[right_child->key_num - 1] = right_child->children[right_child->key_num];
        right_child->key_num--;

        parent->keys[index] = first_key;
    }
    else { //if not enough keys are present in navigate through the parent
        if (index - 1 >= 0) {
            InternalNode* left_child = static_cast<InternalNode*>(parent->children[index - 1]);

            left_child->keys[left_child->key_num] = parent->keys[index - 1];
            left_child->key_num++;

            for (int i = 0; i < curr_node->key_num; i++) {
                left_child->keys[left_child->key_num] = curr_node->keys[i];
                left_child->children[left_child->key_num] = curr_node->children[i];
                left_child->children[left_child->key_num]->parent = left_child;
                left_child->key_num++;
            }

            left_child->children[left_child->key_num] = curr_node->children[curr_node->key_num];
            left_child->children[left_child->key_num]->parent = left_child;

            for (int i = index - 1; i < parent->key_num - 1; i++) {
                parent->keys[i] = parent->keys[i + 1];
                parent->children[i + 1] = parent->children[i + 2]; 
            }
            parent->children[parent->key_num - 1] = parent->children[parent->key_num]; 
            parent->key_num--; 

            //recursively calling this function until root node is reached.
            reorganizeNodes(parent);
        }
        else if (index + 1 <= parent->key_num) {
            InternalNode* right_child = static_cast<InternalNode*>(parent->children[index + 1]);

            int buffer = curr_node->key_num + 1;
            right_child->children[right_child->key_num + buffer] = right_child->children[right_child->key_num];

            for (int i = right_child->key_num-1; i >= 0; i--) { 
                right_child->keys[i + buffer] = right_child->keys[i];
                right_child->children[i + buffer] = right_child->children[i];
            }

            for (int i = 0; i < curr_node->key_num; i++) {
                right_child->keys[i] = curr_node->keys[i];
                right_child->children[i] = curr_node->children[i];
                right_child->children[i]->parent = right_child;
            }

            right_child->keys[curr_node->key_num] = parent->keys[index];
            right_child->children[curr_node->key_num] = curr_node->children[curr_node->key_num];
            right_child->children[curr_node->key_num]->parent = right_child;
            right_child->key_num += buffer;

            for (int i = index; i < parent->key_num - 1; i++) {
                parent->keys[index] = parent->keys[index + 1];
                parent->children[index] = parent->children[index + 1];
            }
            parent->children[parent->key_num - 1] = parent->children[parent->key_num];
            parent->key_num--;
            
            //recursively calling this function until root node is reached.
            reorganizeNodes(parent);
        }        
    }
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * Delete key & value pair associated with input key
 * If current tree is empty, return immdiately.
 * If not, User needs to first find the right leaf node as deletion target, then
 * delete entry from leaf node. Remember to deal with redistribute or merge if
 * necessary.
 */

void BPlusTree::Remove(const KeyType &key) {
    if (IsEmpty())
        return;
    Node* cursor = root;
    bool contains = false;
    int index = 0;
    int val = 0;

    while (!cursor->is_leaf)
    {
        index = searchIndex(key, cursor->keys, cursor->key_num); //to find which child to go into
        cursor = ((InternalNode*)cursor)->children[index];
    }
    index = searchIndex(key, cursor->keys, cursor->key_num,contains); // to find where to delete from in current leafNode
   
    if (!contains) return; // key not in leaf node, no delete needs to be performed

    LeafNode* curr_leaf = static_cast<LeafNode*>(cursor);

    //deleting node, from curr leaf node.
    while (index + 1 < curr_leaf->key_num) {
        curr_leaf->keys[index] = curr_leaf->keys[index + 1];
        curr_leaf->pointers[index] = curr_leaf->pointers[index + 1];
        index++;
    }
    curr_leaf->key_num--;

    if (curr_leaf == root)
    {
        if (curr_leaf->key_num == 0) // if nothing in root; make root NULL
        {
            root = nullptr;
        }
        return;
    }

    int node_size = (MAX_FANOUT - 1) / 2;
    InternalNode* parent = static_cast<InternalNode*>(curr_leaf->parent);
    contains = false;

    index = searchIndex(key, parent->keys, parent->key_num, contains); // to find where to delete from in current leafNode

    if (contains){index++;}

    if (curr_leaf->key_num < node_size)
    {

         if (index - 1 >= 0 && curr_leaf->prev_leaf->key_num > node_size) { // checking if left sibling has enough elements.
                                                                    //copying last element from left most node and add it to start of current leaf node          
            int i = curr_leaf->key_num - 1;
            while (i >= 0)
            {
                curr_leaf->keys[i + 1] = curr_leaf->keys[i];
                curr_leaf->pointers[i + 1] = curr_leaf->pointers[i];
                i--;
            }
            curr_leaf->keys[0] = curr_leaf->prev_leaf->keys[curr_leaf->prev_leaf->key_num - 1];
            curr_leaf->pointers[0] = curr_leaf->prev_leaf->pointers[curr_leaf->prev_leaf->key_num - 1];
            curr_leaf->key_num++;
            curr_leaf->prev_leaf->key_num--;

            parent->keys[index - 1] = curr_leaf->keys[0];
            }
        else if (index + 1 <= parent->key_num && curr_leaf->next_leaf->key_num > node_size) // checking if right sibling has enough keys, adding the smallest
        {                                                                               // val to last of curr_node
            curr_leaf->keys[curr_leaf->key_num] = curr_leaf->next_leaf->keys[0];
            curr_leaf->pointers[curr_leaf->key_num] = curr_leaf->next_leaf->pointers[0];
            curr_leaf->key_num++;

            int i = 1;
            while (i < curr_leaf->next_leaf->key_num) {
                curr_leaf->next_leaf->keys[i - 1] = curr_leaf->next_leaf->keys[i];
                curr_leaf->next_leaf->pointers[i - 1] = curr_leaf->next_leaf->pointers[i];
                i++;
            }
            curr_leaf->next_leaf->key_num--;

            //add the newly modified right nodes, val to its parent & keep going till root to update it if needed.
            reevaluateParent(parent, curr_leaf->next_leaf->keys[0],val);
        }
        else
        {
            if (index - 1 >= 0) {
                LeafNode* prev_node = curr_leaf->prev_leaf;

                int i = 0;
                while (i < curr_leaf->key_num)
                {
                    prev_node->keys[prev_node->key_num] = curr_leaf->keys[i];
                    prev_node->pointers[prev_node->key_num] = curr_leaf->pointers[i];
                    prev_node->key_num++;
                    i++;
                }

                for (int i = index; i <= parent->key_num - 1; i++) {   //removing pointer from parent
                    parent->children[i] = parent->children[i + 1];
                }

                for (int i = index - 1; i < parent->key_num - 1; i++) {     //removing key from parent
                    parent->keys[i] = parent->keys[i + 1];
                }
                parent->key_num--;

                
                prev_node->next_leaf = curr_leaf->next_leaf;
                if (curr_leaf->next_leaf != nullptr) {
                    curr_leaf->next_leaf->prev_leaf = prev_node;
                }

                //re organizing nodes, in the tree after modification of left node.
                reorganizeNodes(parent);
                curr_leaf = prev_node;
            }
            else if (index + 1 <= parent->key_num) {
                LeafNode* next_node = curr_leaf->next_leaf;
                int i = 0;
                while( i < next_node->key_num) {        //moving all the keys to left sibling
                    curr_leaf->keys[curr_leaf->key_num] = next_node->keys[i];
                    curr_leaf->pointers[curr_leaf->key_num] = next_node->pointers[i];
                    curr_leaf->key_num++;
                    i++;
                }

                for (int i = index + 1; i <= parent->key_num - 1; i++) {           //removing pointer from parent
                    parent->children[i] = parent->children[i + 1];
                }

                for (int i = index; i < parent->key_num - 1; i++) {           //removing key from parent
                    parent->keys[i] = parent->keys[i + 1];
                }
                parent->key_num--;

                curr_leaf->next_leaf = next_node->next_leaf;
                if (next_node->next_leaf != nullptr) {
                    next_node->next_leaf->prev_leaf = curr_leaf;
                }
                
                //re organizing nodes, in the tree after modification of left node.
                reorganizeNodes(parent);
            }          
        }
    }

   reevaluateParent(parent, curr_leaf->keys[0],val);

}

/*****************************************************************************
 * RANGE_SCAN
 *****************************************************************************/
/*
 * Return the values that within the given key range
 * First find the node large or equal to the key_start, then traverse the leaf
 * nodes until meet the key_end position, fetch all the records.
 */
void BPlusTree::RangeScan(const KeyType &key_start, const KeyType &key_end,
                          std::vector<RecordPointer> &result)
{

    Node *cursor = root;

    while (!cursor->is_leaf)
    {
        int index = searchIndex(key_start, cursor->keys, cursor->key_num); //to find which child to go into
        cursor = ((InternalNode *)cursor)->children[index];
    }

    LeafNode *current_leaf_node = static_cast<LeafNode *>(cursor);
    int i;
    for (i = 0; i < current_leaf_node->key_num; i++)
    {
        if (current_leaf_node->keys[i] == key_start)
        {
            break;
        }
    }

    while (current_leaf_node->keys[i] <= key_end && i < current_leaf_node->key_num)
    {
        if (current_leaf_node->keys[i] == key_end)
        {
            break;
        }
        result.push_back(current_leaf_node->pointers[i]);

        i++;
        if (i == current_leaf_node->key_num && current_leaf_node->next_leaf != NULL)
        {
            current_leaf_node = current_leaf_node->next_leaf;
            i = 0;
        }
    }
}


