//
// Created by Kevin Chou on 2022/1/13.
//

#ifndef ORIENTATION_TASKS_WRITE_REQUEST_H
#define ORIENTATION_TASKS_WRITE_REQUEST_H

#endif //ORIENTATION_TASKS_WRITE_REQUEST_H


class WriteRequest {
    std::condition_variable &cv;
public:
    explicit WriteRequest(std::condition_variable &cv_) : cv(cv_) {};

    void WakeWriter();
};


void WriteRequest::WakeWriter() {
    cv.notify_one();
}