//
// Created by Kevin Chou on 2022/1/13.
//
#include <iostream>
#include <list>
#include <random>
#include <thread>
#include "candy_lock.h"

using namespace std;

const int TRY_NUM = 100;
const int THREAD_NUM = 8;


int candy_number = 0;
CandyLock candy_lock{};


uniform_int_distribution<unsigned> u(0, 1);
default_random_engine e;


void read() {
    candy_lock.RLock();
    cout << "Thread " << this_thread::get_id() << " read candy number " << candy_number << endl;
    candy_lock.RUnlock();
}

void write() {
    candy_lock.Lock();
    cout << "Thread " << this_thread::get_id() << " increment candy number " << ++candy_number << endl;
    candy_lock.Unlock();
}

void readOrWrite() {
    for (int i = 0; i < TRY_NUM; i++) {
        bool read_turn = u(e);
        if (read_turn) {
            read();
        } else {
            write();
        }
    }
    cout << "Thread " << this_thread::get_id() << " exiting..." << endl;
}

int main() {
    e.seed(time(nullptr));
    thread ts[THREAD_NUM]{};
    for (auto &t : ts)
        t = thread(readOrWrite);
    for (auto &t : ts)
        t.join();
    cout << "Exits" << endl;
}