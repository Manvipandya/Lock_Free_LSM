// key -> T1
// hash_root(key) -> T2
// val -> T3
// root will be a list of (T1, T3)
// level_1 will be a vector of vector of (T1, T3)


#include <iostream>

class LinkedListNode {
    int val;
    LinkedListNode *next;
};

class LevelNode {
    vector<vector<int>> hashStore;
    public:
        void myInsert(int k, int slot) {
            // insert k in the correct slot
            // use chaining to resolve collisions, if any
        } 
        void myLookup(int k, int slot) {
            // look for k in the correct slot
            // if not present, look in the subsequent levels
        }
};

class Level {
    vector<LevelNode> arrays;
    Level *nextLevel;
    int maxSize;
    int currSize; // increment with each insert
    int myHash() { }
    public:
        Level(int n) {
            // based on myHash, find the number of arrays (n)
            // push n empty LevelNodes into arrays
        } 
}

class Root {
    LinkedListNode *head;
    int numNodes;
    int myHash(int v) { 
        // TO BE CHANGED
        return v; 
    }
    Level *nextLevel;
    int maxSize;            // sizeof(LinkedListNode) * numNodes should be less than maxSize
    public:
        Root(int ms) {
            head = nullptr;
            maxSize = ms;
            numNodes = 0;
            nextLevel = nullptr;
            // initialize more, if needed
        }
        void myInsert(LinkedListNode& newNode) {
            // insert it into the list pointed to by head
        }
        void myDelete(int k) {
            // call insert with a new node (k, TOMBSTONE)
        }
        int myLookup(int k) {
            // traverse the list pointed to by head
            // if not in the root, compute myHash(k) -> h
            // look into the appropriate bucket (obtained from nextLevelPtrs) depending on h
        }
        void myFlush() {

        }
};

class LSM {
    Root *treeRoot; // only point of entry into the data structure
    public:
        void myInsert() {

        }

};

void dump(Root *treeRoot) {
    // print the trees
}

int main() {
    return 0;
}
