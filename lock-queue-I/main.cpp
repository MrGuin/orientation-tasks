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
// either next_candy < CANDY_NUM && can_grab == true
// or next_candy == CANDY_NUM && can_grab == false
// or next_candy == DONE && can_grab == true (which means over)
int next_candy = CANDY_NUM;
bool can_grab = false;

std::mutex candy_mutex;
std::condition_variable cv;

void grab_candy() {
    while (true) {
        std::unique_lock <std::mutex> lk(candy_mutex);  // workers blocked here trying to grab the lock
        cv.wait(lk, [] { return can_grab; });    // workers blocked here waiting for a new round
        if (next_candy >= 0 && next_candy < CANDY_NUM) {
            std::cout << "Thread " << std::this_thread::get_id()
                      << " grabs candy " << next_candy << std::endl;
            next_candy++;
        } else if (next_candy == CANDY_NUM) {
            can_grab = false;
            cv.notify_all();   // notify main
        } else {
            std::cout << "All rounds finished, thread " << std::this_thread::get_id()
                      << " exiting..." << std::endl;
            break;
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
        cv.wait(lk, [] { return !can_grab; });
        std::cout << "Candies all grabbed, main thread wakes up." << std::endl;
        if (i == ROUNDS) {  // finished
            next_candy = DONE;
            can_grab = true;
            cv.notify_all();    // notify all workers
            std::cout << "All rounds finished, waiting for all threads exit..." << std::endl;
            break;
        }
        next_candy = 0;
        can_grab = true;
        std::cout << "Main thread starting a new round: " << i << std::endl;
        cv.notify_all();     // notify all workers
    }
    for (auto i = 0; i < THREAD_NUM; i++) {
        threads[i].join();
    }
    std::cout << "Main exits." << std::endl;
}

