#ifndef __TRAFFIC_LIGHT_H_
#define __TRAFFIC_LIGHT_H_

#include <iostream>
#include <thread>
#include <mutex>
#include <deque>
#include <memory>
#include <condition_variable>

#include "TrafficObject.h"

class Vehicle;

enum TrafficLightPhase
{
    red,
    green
};

template <class T>
class MessageQueue
{
    private:
        std::mutex mutex_;
        std::condition_variable cond_;
        std::deque<T> queue_;
    public:
        T receive();
        void send(T && msg);
};

class TrafficLight : public TrafficObject
{
    private:
        void cycleThroughPhases();

        std::shared_ptr<MessageQueue<TrafficLightPhase>> msgQueue_;
        TrafficLightPhase currentPhase_;
        std::condition_variable cond_;
        std::mutex mutex_;

    public:
        TrafficLight();
        void waitForGreen() const;
        void simulate();
        TrafficLightPhase getCurrentPhase() const;
};

#endif