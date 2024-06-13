#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <limits>
#include <random>
#include <zmq.hpp>
#include <string>
#include <unordered_map>
#include <sstream>
#include <thread>
#include <memory>
#include <stdexcept>

double p_found = 1; // Probability of lookups for keys that will be found
double p_lookup = 0; // Probability of lookup

using namespace std;

const int TOMBSTONE = numeric_limits<int>::min(); // Define the tombstone value

class Level;

class LevelNode {
public:
    vector<pair<pair<int, pair<long long, int>>, int>> entries; // Contains {((key, (time, client_id)), value)}
    long long maxTimestamp = numeric_limits<long long>::min(); // Stores max timestamp
    int maxSize;
    LevelNode(int max_size) : maxSize(max_size) {}

    void insert(int key, long long time, int client_id, int value, Level* nextLevel);
    pair<pair<long long, int>, int> lookup(int key, pair<pair<long long, int>, int> result, long long lookupTimestamp);
};

class Level {
public:
    vector<LevelNode> nodes;
    Level* nextLevel;
    int nodeSize;
    long long maxTimestamp = numeric_limits<long long>::min(); // Max timestamp for the level

    Level(int count, int node_size) : nodeSize(node_size), nextLevel(nullptr) {
        nodes.reserve(count);
        for (int i = 0; i < count; ++i) {
            nodes.emplace_back(node_size);
        }
        cout << "Level created with " << count << " nodes, each with max size " << node_size << endl;
    }

    ~Level() {
        delete nextLevel;
    }

    void insert(int key, long long time, int client_id, int value) {
        int index = key % nodes.size();
        nodes[index].insert(key, time, client_id, value, this);
        maxTimestamp = max(maxTimestamp, nodes[index].maxTimestamp);
    }

    pair<pair<long long, int>, int> lookup(int key, pair<pair<long long, int>, int> result, long long lookupTimestamp) {
    if (maxTimestamp < result.first.first) return result;
    int index = key % nodes.size();
    pair<pair<long long, int>, int> result2 = nodes[index].lookup(key, result, lookupTimestamp);
    if (result2.first.first > result.first.first || 
        (result2.first.first == result.first.first && result2.first.second > result.first.second)) {
        result = result2;
    }

    if (nextLevel != nullptr) {
        return nextLevel->lookup(key, result, lookupTimestamp);
    }
    return result;
}


    void print(int levelIndex) {
        cout << "Level " << levelIndex << " maxTimestamp: " << maxTimestamp << endl;
        for (int i = 0; i < nodes.size(); ++i) {
            cout << "  Node " << i << ": ";
            for (auto& entry : nodes[i].entries) {
                cout << "(((" << entry.first.first << ", (" << entry.first.second.first << ", " << entry.first.second.second << ")), " << entry.second << ") ";
            }
            cout << endl;
        }
        if (nextLevel != nullptr) {
            nextLevel->print(levelIndex + 1);
        }
    }
};

void LevelNode::insert(int key, long long time, int client_id, int value, Level* currentLevel) {
    cout << "Inserting key: " << key << " at time: " << time << " with client_id: " << client_id << " and value: " << value << " into LevelNode\n";
    entries.emplace_back(make_pair(make_pair(key, make_pair(time, client_id)), value));
    maxTimestamp = max(maxTimestamp, time);

    if (entries.size() >= maxSize) {
        if (currentLevel->nextLevel == nullptr) {
            cout << "Creating next level from node size " << currentLevel->nodes.size() << " to " << currentLevel->nodes.size() * 2 << std::endl;
            currentLevel->nextLevel = new Level(currentLevel->nodes.size() * 2, maxSize);
        }
        for (auto& entry : entries) {
            int idx = entry.first.first % currentLevel->nextLevel->nodes.size();
            cout << "Moving entry with key " << entry.first.first << " to next level at index " << idx << std::endl;
            currentLevel->nextLevel->nodes[idx].insert(entry.first.first, entry.first.second.first, entry.first.second.second, entry.second, currentLevel->nextLevel);
            currentLevel->nextLevel->maxTimestamp = max(currentLevel->nextLevel->maxTimestamp, entry.first.second.first);
        }
        entries.clear();
        maxTimestamp = numeric_limits<long long>::min();
    }
}

pair<pair<long long, int>, int> LevelNode::lookup(int key, pair<pair<long long, int>, int> result, long long lookupTimestamp) {
    if (maxTimestamp < result.first.first) return result;
    for (auto& entry : entries) {
        if (entry.first.first == key && entry.first.second.first < lookupTimestamp) {
            cout << "LevelNode: Found key " << key << " with timestamp " << entry.first.second.first << " before lookup timestamp " << lookupTimestamp << endl;
            if (entry.first.second.first > result.first.first || 
                (entry.first.second.first == result.first.first && entry.first.second.second > result.first.second)) {
                result = {entry.first.second, entry.second};
            }
        }
    }
    if (result.second == TOMBSTONE) {
        return {result.first, -1};
    }
    return result;
}


class LinkedListNode {
public:
    int key;
    long long time;
    int client_id;
    int value;
    LinkedListNode* next;
    LinkedListNode(int k, long long t, int c, int v) : key(k), time(t), client_id(c), value(v), next(nullptr) {}
};

class Root {
public:
    LinkedListNode* head;
    int numNodes;
    int maxSize;
    Level* nextLevel;

    Root(int ms) : head(nullptr), numNodes(0), maxSize(ms), nextLevel(nullptr) {}

    ~Root() {
        while (head != nullptr) {
            LinkedListNode* temp = head;
            head = head->next;
            delete temp;
        }
        delete nextLevel;
    }

    void myInsert(int key, long long time, int client_id, int value) {
        LinkedListNode* newNode = new LinkedListNode(key, time, client_id, value);
        newNode->next = head;
        head = newNode;
        numNodes++;
        if (numNodes >= maxSize) {
            myFlush();
        }
    }

    void myFlush() {
        cout << "Flushing Root" << endl;
        if (nextLevel == nullptr) {
            cout << "Creating next level with initial size" << endl;
            nextLevel = new Level(2, maxSize);
        }
        LinkedListNode* current = head;
        while (current != nullptr) {
            nextLevel->insert(current->key, current->time, current->client_id, current->value);
            LinkedListNode* temp = current;
            current = current->next;
            delete temp;
        }
        head = nullptr;
        numNodes = 0;
    }

    pair<pair<long long, int>, int> lookup(int key, long long lookupTimestamp) {
    pair<pair<long long, int>, int> result = {{0, 0}, -1};
    cout << "Root: Starting lookup for key " << key << " with lookup timestamp " << lookupTimestamp << endl;
    LinkedListNode* current = head;
    while (current != nullptr) {
        if (current->key == key && current->time < lookupTimestamp) {
            cout << "Root: Found key " << key << " with timestamp " << current->time << endl;
            if (current->time > result.first.first || 
                (current->time == result.first.first && current->client_id > result.first.second)) {
                result = {{current->time, current->client_id}, current->value};
            }
        }
        current = current->next;
    }
    if (nextLevel != nullptr) {
        result = nextLevel->lookup(key, result, lookupTimestamp);
    }
    if (result.second == TOMBSTONE) {
        return {result.first, -1};
    }
    return result;
}

    void print() {
        cout << "Root Level (LinkedList):" << endl;
        LinkedListNode* current = head;
        while (current != nullptr) {
            cout << "(((" << current->key << ", (" << current->time << ", " << current->client_id << ")), " << current->value << ") -> ";
            current = current->next;
        }
        cout << "nullptr" << endl;
        if (nextLevel != nullptr) {
            nextLevel->print(1);
        }
    }
};

class LSM {
    Root* treeRoot;

public:
    LSM(int maxSize) {
        treeRoot = new Root(maxSize);
        cout << "LSM tree created with max size " << maxSize << endl;
    }

    ~LSM() {
        cout << "Destroying LSM tree" << endl;
        delete treeRoot;
    }

    void myInsert(int key, long long time, int client_id, int value) {
        treeRoot->myInsert(key, time, client_id, value);
    }

    void myDelete(int key, long long time, int client_id) {
        myInsert(key, time, client_id, TOMBSTONE); 
    }

    pair<pair<long long, int>, int> lookup(int key, long long lookupTimestamp) {
        return treeRoot->lookup(key, lookupTimestamp);
    }

    Root* getRoot() {
        return treeRoot;
    }
};

void worker_thread(zmq::context_t& context, LSM& lsm, int thread_id) {
    zmq::socket_t socket(context, zmq::socket_type::rep);
    socket.connect("inproc://workers");
    
    while (true) {
        zmq::message_t request;

        // Wait for the next request from a client
        socket.recv(request, zmq::recv_flags::none);
        std::string recv_message(static_cast<char*>(request.data()), request.size());
        std::cout << "Thread " << thread_id << " received: " << recv_message << std::endl;

        std::istringstream iss(recv_message);
        std::string command;
        int key, value, client_id;
        long long timestamp;

        iss >> command;
        std::string reply;
        if (command == "INSERT") {
            iss >> key >> timestamp >> client_id >> value;
            std::cout << "Thread " << thread_id << " processing INSERT for key: " << key << std::endl;
            lsm.myInsert(key, timestamp, client_id, value);
            reply = "Insert Successful";
        } else if (command == "LOOKUP") {
            iss >> key >> timestamp;
            std::cout << "Thread " << thread_id << " processing LOOKUP for key: " << key << std::endl;
            auto result = lsm.lookup(key, timestamp);
            lsm.getRoot()->print();
            std::ostringstream oss;
            oss << "Lookup Result: " << key << ", (" << result.first.first << ", " << result.first.second << "), " << result.second;
            reply = oss.str();
        } else if (command == "DELETE") {
            iss >> key >> timestamp >> client_id;
            std::cout << "Thread " << thread_id << " processing DELETE for key: " << key << std::endl;
            lsm.myDelete(key, timestamp, client_id);
            reply = "Delete Successful";
        }
        zmq::message_t reply_msg(reply.c_str(), reply.size());
        socket.send(reply_msg, zmq::send_flags::none);
        lsm.getRoot()->print();
    }
}


int main() {
    zmq::context_t context(1);
    zmq::socket_t frontend(context, zmq::socket_type::router);
    zmq::socket_t backend(context, zmq::socket_type::dealer);

    frontend.bind("tcp://*:5555");
    backend.bind("inproc://workers");

    std::cout << "Server is ready at tcp://localhost:5555" << std::endl;

    std::vector<std::unique_ptr<LSM>> lsms;
    for (int i = 0; i < 10; ++i) {
        lsms.emplace_back(std::unique_ptr<LSM>(new LSM(10)));
    }

    const int num_workers = 10;
    std::vector<std::thread> workers;
    for (int i = 0; i < num_workers; ++i) {
        workers.emplace_back(worker_thread, std::ref(context), std::ref(*lsms[i]), i);
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    int num_insertions = pow(2, 10);
    for (int i = 0; i < num_insertions; ++i) {
        int key = i;
        long long timestamp = chrono::system_clock::now().time_since_epoch().count();
        int client_id = 1;
        int value = rand() % 1000;
        lsms[0]->myInsert(key, timestamp, client_id, value);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;

    std::cout << "Time taken for " << num_insertions << " insertions: " << duration.count() << " seconds" << std::endl;

    // Performance testing for lookups
    int num_lookups = pow(2, 10);
    int found_count = 0;
    int not_found_count = 0;

    start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_lookups; ++i) {
        int key;
        if (static_cast<double>(rand()) / RAND_MAX < p_found) {
            key = rand() % num_insertions; // Key that is likely to be found
            found_count++;
        } else {
            key = num_insertions + rand() % num_insertions; // Key that is likely not to be found
            not_found_count++;
        }
        long long timestamp = chrono::system_clock::now().time_since_epoch().count();
        auto result = lsms[0]->lookup(key, timestamp);
    }

    end_time = std::chrono::high_resolution_clock::now();
    duration = end_time - start_time;

    std::cout << "Time taken for " << num_lookups << " lookups: " << duration.count() << " seconds" << std::endl;
    std::cout << "Number of found lookups: " << found_count << std::endl;
    std::cout << "Number of not found lookups: " << not_found_count << std::endl;

    zmq::proxy(frontend, backend);

    for (auto& worker : workers) {
        worker.join();
    }
    return 0;
}

