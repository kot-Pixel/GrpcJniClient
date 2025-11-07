//
// Created by Tom on 2025/11/6.
//

#include <string>
#include <mutex>
#include "concurrentqueue.h"

#ifndef GRPCJNICLIENT_CARPLAYVEDIOFRAME_H
#define GRPCJNICLIENT_CARPLAYVEDIOFRAME_H


struct CarplayScreenFrame {
    std::vector<uint8_t> data;
    int width = 0;
    int height = 0;
};

class CarplayScreenFrameQueue {
public:
    void push(std::shared_ptr<CarplayScreenFrame> frame) {
        queue_.enqueue(std::move(frame));
    }

    std::shared_ptr<CarplayScreenFrame> pop() {
        std::shared_ptr<CarplayScreenFrame> frame;
        if (queue_.try_dequeue(frame)) {
            return frame;
        }
        return nullptr;
    }

private:
    moodycamel::ConcurrentQueue<std::shared_ptr<CarplayScreenFrame>> queue_;
};

#endif //GRPCJNICLIENT_CARPLAYVEDIOFRAME_H
