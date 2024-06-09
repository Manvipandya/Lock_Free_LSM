#include <iostream>
#include <vector>
using namespace std;

 // Forward declaration of the Level class
class Level;

class LevelNode {
public:
    vector<pair<int, int>> entries; 
    int maxSize;

    LevelNode(int max_size) : maxSize(max_size) {}

    void insert(int key, int value, Level* nextLevel);
};

class Level {
public:
    vector<LevelNode> nodes; 
    Level* nextLevel; 
    int nodeSize; 

    Level(int count, int node_size) : nodeSize(node_size), nextLevel(nullptr) {
        nodes.resize(count, LevelNode(node_size));
    }

    void insert(int key, int value) {
        int index = key % nodes.size(); // Simple hash function based on modulus
        if (nextLevel == nullptr) {
            nextLevel = new Level(nodes.size() * 2, nodeSize); // (Need to change) Double the number of nodes in the next level
        }
        nodes[index].insert(key, value, nextLevel);
    }

    void print(int levelIndex = 0) {
        cout << "Level " << levelIndex << ":" << endl;
        for (int i = 0; i < nodes.size(); ++i) {
            cout << "  Node " << i << ": ";
            for (auto& entry : nodes[i].entries) {
                cout << "(" << entry.first << ", " << entry.second << ") ";
            }
            cout << endl;
        }
        if (nextLevel != nullptr) {
            nextLevel->print(levelIndex + 1); // Recursively print the next level
        }
    }
};

// Define the insert function after Level has been declared (to remove circular dependancy)
void LevelNode::insert(int key, int value, Level* nextLevel) {
    entries.push_back(make_pair(key, value));
    if (entries.size() >= maxSize && nextLevel != nullptr) {
        if (nextLevel->nodes.empty()) {
            nextLevel->nodes.resize(nextLevel->nodes.size(), LevelNode(maxSize));
        }
        int index = key % nextLevel->nodes.size();
        for (auto& entry : entries) {
            nextLevel->nodes[index].insert(entry.first, entry.second, nextLevel->nextLevel);
        }
        entries.clear(); // Clear entries after flushing
        if (nextLevel->nextLevel == nullptr && !nextLevel->nodes[index].entries.empty()) {
            nextLevel->nextLevel = new Level(nextLevel->nodes.size() * 2, maxSize);
        }
    }
}

class LinkedListNode {
public:
    int key;
    int value;
    LinkedListNode *next;
    LinkedListNode(int k, int v) : key(k), value(v), next(nullptr) {}
};

class Root {
    LinkedListNode *head;
    int numNodes;
    int maxSize;
    Level *nextLevel;

public:
    Root(int ms) : head(nullptr), numNodes(0), maxSize(ms), nextLevel(nullptr) {}

    void myInsert(int key, int value) {
        LinkedListNode *newNode = new LinkedListNode(key, value);
        newNode->next = head;
        head = newNode;
        numNodes++;

        if (numNodes > maxSize) {
            myFlush();
        }
    }

    void myFlush() {
        if (nextLevel == nullptr) {
            nextLevel = new Level(maxSize*2, maxSize); 
        }

        LinkedListNode *current = head;
        while (current != nullptr) {
            nextLevel->insert(current->key, current->value);
            LinkedListNode *temp = current;
            current = current->next;
            delete temp;
        }
        head = nullptr;
        numNodes = 0;
    }

    LinkedListNode* getHead() {
        return head;
    }

    Level* getNextLevel() {
        return nextLevel;
    }

    void print() {
        cout << "Root Level (LinkedList):" << endl;
        LinkedListNode *current = head;
        while (current != nullptr) {
            cout << "(" << current->key << ", " << current->value << ") -> ";
            current = current->next;
        }
        cout << "nullptr" << endl;
        if (nextLevel != nullptr) {
            nextLevel->print(1);  // Start from level 1 as the next level
        }
    }
};

class LSM {
    Root *treeRoot;

public:
    LSM(int maxSize) {
        treeRoot = new Root(maxSize);
    }

    ~LSM() {
        delete treeRoot;
    }

    void myInsert(int key, int value) {
        treeRoot->myInsert(key, value);
    }

    Root* getRoot() {
        return treeRoot;
    }
};

int main() {
    LSM lsm(3); // Initialize LSM with a root max size
    lsm.myInsert(1, 100);
    lsm.myInsert(2, 200);
    lsm.myInsert(3, 300);
    lsm.myInsert(4, 400); 
    lsm.myInsert(5, 500);
    lsm.myInsert(6, 600);
    lsm.myInsert(7, 700); 
    lsm.myInsert(1, 101);
    lsm.myInsert(1, 102);
    lsm.myInsert(7, 100);
    lsm.myInsert(7, 100);
    lsm.myInsert(7, 101);

    lsm.getRoot()->print();

    return 0;
}
