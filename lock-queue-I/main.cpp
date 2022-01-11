//
// Created by Kevin Chou on 2022/1/11.
//
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

//const int CANDY_NUM = 1e7;
const int CANDY_NUM = 1e3;
const int THREAD_NUM = 8;
const int ROUNDS = 20;
const int DONE = -1;

// invariant:
// either next_candy < CANDY_NUM && can_grab = true
// or next_candy == CANDY_NUM && can_grab = false
// or next_candy == DONE && can_grab = true (which means over)
int next_candy = CANDY_NUM;
bool can_grab = false;

std::mutex candy_mutex;
std::condition_variable cv_worker;
std::condition_variable cv_main;
// Distinguish cv_main from cv_worker to avoid unnecessary wakeup.
// Thus we can use cv_worker.notify_one() to wakeup exactly
// one worker after current worker just finished grabbing.
// Otherwise we have to use cv.notify_all() to wakeup both all
// workers and main every time.

void grab_candy() {
    while (true) {
        std::unique_lock <std::mutex> lk(candy_mutex);
        cv_worker.wait(lk, [] { return can_grab; });
        if (next_candy >= 0 && next_candy < CANDY_NUM) {
            std::cout << "Thread " << std::this_thread::get_id()
                      << " grabs candy " << next_candy << std::endl;
            next_candy++;
            lk.unlock();
            cv_worker.notify_one();    // release lock, notify one worker thread
        } else if (next_candy == CANDY_NUM) {
            can_grab = false;
            lk.unlock();
            cv_main.notify_one();   // notify main
        } else {
            std::cout << "All rounds finished, thread " << std::this_thread::get_id()
                      << " exiting..." << std::endl;
            cv_worker.notify_one();    // release lock, notify one worker thread
            return;
        }
    }
}

int main() {
    std::thread threads[THREAD_NUM]{};
    for (int i = 0; i < THREAD_NUM; i++) {
        threads[i] = std::thread(grab_candy);
    }
    for (int i = 0; i <= ROUNDS; i++) {
        std::unique_lock <std::mutex> lk(candy_mutex);
        cv_main.wait(lk, [] { return !can_grab; });
        std::cout << "Candies all grabbed, main thread wakes up." << std::endl;
        if (i == ROUNDS) {  // finished
            next_candy = DONE;
            can_grab = true;
            lk.unlock();
            cv_worker.notify_one();    // notify one worker thread
            std::cout << "All rounds finished, waiting for all threads exit..." << std::endl;
            break;
        }
        next_candy = 0;
        can_grab = true;
        std::cout << "Main thread starting a new round: " << i << std::endl;
        lk.unlock();
        cv_worker.notify_one();     // notify one worker thread
    }
    for (auto i = 0; i < THREAD_NUM; i++) {
        threads[i].join();
    }
    std::cout << "Main exits." << std::endl;
}

