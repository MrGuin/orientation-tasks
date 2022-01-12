//
// Created by Kevin Chou on 2022/1/12.
//
#include <iostream>
#include <mutex>
#include <vector>
#include <fstream>
#include <thread>
#include <map>
#include <filesystem>

using namespace std;

const int THREAD_NUM = 3;
const string DIR_PATH = "/Users/kevin/Documents/work/orientation-tasks/word-count/articles/";

vector<string> articles;
vector<string>::iterator ite;
mutex m_articles;   // to protect articles and ite
map<string, int> word_count;
mutex m_count;      // to protect word_count

void print(map<string, int> &mp) {
    for (const auto &p : mp) {
        cout << p.first << " : " << p.second << " ";
    }
    cout << endl;
}

void calculate() {
    while (true) {
        string filepath;
        // get a file
        {
            lock_guard<mutex> guard(m_articles);
            if (ite == articles.end()) {
                return;
            }
            filepath = *ite;
            ite++;
        }
        // open file
        ifstream in(filepath);
        if (!in.is_open()) {
            cout << "Thread " << this_thread::get_id() << " could not open file: " << filepath << endl;
            return;
        }
        // count words in file
        map<string, int> cnt{};
        string word;
        while (in) {
            in >> word;
            // filter non-alphabet-characters
            word.erase(remove_if(word.begin(), word.end(), [](char x) { return !isalpha(x); }), word.end());
            transform(word.begin(), word.end(), word.begin(), ::tolower);
            cnt[word]++;
        }
        // merge into global result
        lock_guard<mutex> guard(m_count);
        for (auto &p : cnt) {
            word_count[p.first] += p.second;
        }
        cout << "Thread " << this_thread::get_id() << " finished calculating file " << filepath << endl;
    }
}

int main() {
    // prepare articles
    for (const auto &entry : filesystem::directory_iterator(DIR_PATH)) {
        articles.push_back(entry.path());
    }
    ite = articles.begin();
    // generating threads
    thread threads[THREAD_NUM]{};
    for (auto &t : threads) {
        t = thread(calculate);
    }
    // waiting for results
    for (auto &thread : threads) {
        thread.join();
    }
    cout << "All finished" << endl;
    cout << "Result: " << endl;
    print(word_count);
    cout << "Exit" << endl;
}