#include <zmq.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <random>
#include <thread>
#include <chrono>
using namespace std;

void client_operation(int id, const std::string& operation, int key, int value = 0) {
    zmq::context_t context(1);
    zmq::socket_t socket(context, zmq::socket_type::req);
    socket.connect("tcp://localhost:5555");

    long long timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    std::ostringstream oss;
    if (operation == "INSERT") {
        oss << "INSERT " << key << " " << timestamp << " " << id << " " << value;
    } else if (operation == "LOOKUP") {
        oss << "LOOKUP " << key<< " " << timestamp;
    } else if (operation == "DELETE") {
        oss << "DELETE " << key << " " << timestamp << " " << id;
    }

    std::string message = oss.str();
    zmq::message_t request(message.c_str(), message.size());
    //std::cout << "Client [" << id << "] sending request: " << message << std::endl;
    socket.send(request, zmq::send_flags::none);

    zmq::message_t reply;
    socket.recv(reply, zmq::recv_flags::none);
    std::string reply_str(static_cast<char*>(reply.data()), reply.size());
    if (operation == "LOOKUP")
        std::cout << "Client [" << id << "] received reply: " << reply_str << std::endl;
}

int main() {
    std::vector<std::thread> clients;
    //start
    auto start = std::chrono::high_resolution_clock::now();

    // Test Case 1: Basic Insert and Lookup
    clients.emplace_back([]() { client_operation(1, "INSERT", 1, 100); });
    clients.emplace_back([]() { client_operation(2, "INSERT", 2, 200); });
    
    clients.emplace_back([]() { client_operation(3, "LOOKUP", 1); });
    clients.emplace_back([]() { client_operation(4, "LOOKUP", 2); });

    // Test Case 2: Handling Updates
    clients.emplace_back([]() { client_operation(5, "INSERT", 1, 150); });
    clients.emplace_back([]() { client_operation(6, "LOOKUP", 1); });

    // Test Case 3: Deletion Handling
    clients.emplace_back([]() { client_operation(7, "DELETE", 1); });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    clients.emplace_back([]() { client_operation(8, "LOOKUP", 1); });

    // Test Case 4: Multi-Level Insert and Lookup
    for (int i = 9; i < 15; ++i) {
        clients.emplace_back([i]() { client_operation(i, "INSERT", i - 8, (i - 8) * 100); });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    clients.emplace_back([]() { client_operation(15, "LOOKUP", 1); });
    clients.emplace_back([]() { client_operation(16, "LOOKUP", 5); });

    // // Test Case 5: Concurrency Test
    // for (int i = 17; i < 22; ++i) {
    //     clients.emplace_back([i]() { client_operation(i, "INSERT", i, i * 10); });
    // }
    // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // for (int i = 22; i < 27; ++i) {
    //     clients.emplace_back([i]() { client_operation(i, "LOOKUP", i - 17); });
    // }

    // // Test Case 6: Insert and Delete Same Key Concurrently
    // clients.emplace_back([]() { client_operation(27, "INSERT", 1, 1000); });
    // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // clients.emplace_back([]() { client_operation(28, "DELETE", 1); });
    // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // clients.emplace_back([]() { client_operation(29, "LOOKUP", 1); });

    // // Test Case 7: Lookup After Flush
    // for (int i = 30; i < 40; ++i) {
    //     clients.emplace_back([i]() { client_operation(i, "INSERT", i, i * 10); });
    // }
    // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // clients.emplace_back([]() { client_operation(40, "LOOKUP", 1); });
    // clients.emplace_back([]() { client_operation(41, "LOOKUP", 10); });
    // clients.emplace_back([]() { client_operation(42, "LOOKUP", 39); });

    for (auto& thread : clients) {
        thread.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    //end
    std::chrono::duration<double> duration = end - start;
    cout << "Duration = " << duration.count() << " seconds" << endl;

    return 0;
}
