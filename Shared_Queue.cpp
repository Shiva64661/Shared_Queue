#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

class SharedQueue {
private:
    std::queue<std::string> messages;
    std::mutex mutex;
    std::condition_variable cv;

public:
    // Add a string to the queue
    void enqueue(const std::string& message) {
        std::unique_lock<std::mutex> lock(mutex);
        messages.push(message);
        cv.notify_all();  // Notify waiting readers
    }

    // Get the next message from the queue
    std::string dequeue() {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this]() { return !messages.empty(); });
        std::string message = messages.front();
        messages.pop();
        return message;
    }
};

// Reader thread function
void readerThread(int id, SharedQueue& queue) {
    while (true) {
        std::string message = queue.dequeue();
        std::cout << "Reader " << id << " received message: " << message << std::endl;
    }
}

// Writer thread function
void writerThread(SharedQueue& queue) {
    int messageCount = 0;
    while (true) {
        std::string message = "Message " + std::to_string(messageCount++);
        queue.enqueue(message);
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Simulate writer producing messages
    }
}

int main() {
    SharedQueue queue;
    std::thread writer(writerThread, std::ref(queue));
    std::thread readers[5];

    // Create reader threads
    for (int i = 0; i < 5; ++i) {
        readers[i] = std::thread(readerThread, i + 1, std::ref(queue));
    }

    // Join reader threads
    for (int i = 0; i < 5; ++i) {
        readers[i].join();
    }

    writer.join();
    return 0;
}
