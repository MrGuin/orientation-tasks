//
// Created by Kevin Chou on 2022/1/13.
//

#ifndef ORIENTATION_TASKS_CANDY_LOCK_H
#define ORIENTATION_TASKS_CANDY_LOCK_H

#endif //ORIENTATION_TASKS_CANDY_LOCK_H

#include <iostream>
#include "write_request.h"


// A fair read write lock implementation with writers FIFO guarantee.
// Writers won't block forever when readers keep coming.
// After a writer releases the lock, all waiting readers and the
// next waiting writer will be woken up to compete for the lock.
class CandyLock {
    // invariants:
    // 1. can_read and can_write are false after one successful Lock call
    //    until the corresponding Unlock call returns;
    // 2. can_read is false after a new writer entering writer_queue
    //    until one writer get lock and unlock setting can_read = true;
    // 3. can_write is false after one successful RLock call
    //    until reader_count decrease to 0;
    // 4. writer_waiting = false if and only if writer_queue is empty
    int reader_count;
    bool can_read;
    bool can_write;
    bool writer_waiting;
    std::list<WriteRequest> writer_queue{};
    std::mutex m{};
    std::condition_variable cv_readers{};
public:
    CandyLock() : reader_count(0), writer_waiting(false), can_read(true), can_write(true) {};

    void RLock();

    void RUnlock();

    void Lock();

    void Unlock();

};

void CandyLock::RLock() {
    std::unique_lock<std::mutex> lk(m);
    // if !can_read, wait until one waiting writer get the lock and set can_read
    cv_readers.wait(lk, [this] { return can_read; });
    reader_count++;
    can_write = false;
}

void CandyLock::RUnlock() {
    std::unique_lock<std::mutex> lk(m);
    reader_count--;
    if (reader_count == 0) {
        can_write = true;
        if (writer_waiting) {   // wake up one waiting writer
            WriteRequest req = writer_queue.front();
            req.WakeWriter();
        }
    }
}

void CandyLock::Lock() {
    std::unique_lock<std::mutex> lk(m);
    if (!can_write) {   // cannot write, entering writer queue
        writer_waiting = true;
        can_read = false;
        std::condition_variable cv;
        writer_queue.emplace_back(cv);
        cv.wait(lk, [this] { return can_write; });
        // after waiting, current writer is guaranteed to acquire the lock
        // remove the write request
        writer_queue.pop_front();
    }
    can_read = false;
    can_write = false;
}

void CandyLock::Unlock() {
    std::unique_lock<std::mutex> lk(m);
    can_read = true;
    can_write = true;
    // wake up all readers (if any) and one writer (if any)
    cv_readers.notify_all();
    if (!writer_queue.empty()) {
        WriteRequest req = writer_queue.front();
        req.WakeWriter();
    } else {
        writer_waiting = false;
    }
}