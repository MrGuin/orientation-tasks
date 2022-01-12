//
// Created by Kevin Chou on 2022/1/12.
//
#include <iostream>
#include <deque>
#include <thread>

using namespace std;


template<typename T>
class BoundedBuffer {
public:
    BoundedBuffer(size_t capacity) : capacity_(capacity) {};

    void Enqueue(const T &item);

    void Enqueue(T &&item);

    T Dequeue();

private:
    std::deque<T> queue_;
    size_t capacity_;
    std::mutex mux_;
    std::condition_variable cv_for_deque;
    std::condition_variable cv_for_enqueue;
};

template<typename T>
void BoundedBuffer<T>::Enqueue(const T &item) {
    unique_lock<mutex> lk(mux_);
    cv_for_enqueue.wait(lk, [this] { return queue_.size() < capacity_; });
    queue_.push_back(item);
    lk.unlock();
    cv_for_deque.notify_one();
}

template<typename T>
void BoundedBuffer<T>::Enqueue(T &&item) {
    unique_lock<mutex> lk(mux_);
    cv_for_enqueue.wait(lk, [this] { return queue_.size() < capacity_; });
    queue_.push_back(item);
    lk.unlock();
    cv_for_deque.notify_one();
}

template<typename T>
T BoundedBuffer<T>::Dequeue() {
    unique_lock<mutex> lk(mux_);
    cv_for_deque.wait(lk, [this] { return queue_.size() > 0; });
    T t = queue_.front();
    queue_.pop_front();
    lk.unlock();
    cv_for_enqueue.notify_one();
    return t;
}

int main() {
    BoundedBuffer<int> bb(1);
    thread t1([&bb] {
        for (int i = 0; i < 200; i++)
            bb.Enqueue(i);
    });
    thread t2([&bb] {
        for (int i = 0; i < 200; i++)
            cout << bb.Dequeue() << " ";
    });
    t1.join();
    t2.join();
}