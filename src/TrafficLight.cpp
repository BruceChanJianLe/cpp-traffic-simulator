#include "TrafficLight.h"
#include <future>
#include <random>
#include <chrono>

// Implementation of MessageQueue class
template <class T>
T MessageQueue<T>::receive()
{
    // Create a lock and pass it to the condition variable
    std::unique_lock<std::mutex> lck(mutex_);
    cond_.wait(lck, [this](){return !queue_.empty();});

    // Obtain the latest element and remove it from the queue
    T msg = std::move(queue_.back());
    queue_.pop_back();
    return msg;
}

template <class T>
void MessageQueue<T>::send(T && msg)
{
    // Prevent data races
    std::lock_guard<std::mutex> guard(mutex_);

    // Move into queue and notify client
    queue_.push_back(std::move(msg));
    cond_.notify_one();
}

// Implementation of TrafficLight class
TrafficLight::TrafficLight()
{
    currentPhase_ = red;
    msgQueue_ = std::make_shared<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::cycleThroughPhases()
{
    // Create random engine to generate random number 4 and 6
    std::random_device rd;      // Random device will generate a random unsigned int
    std::mt19937 eng(rd());     // mt19937 is one of the random engine provided by C++, we seed it with the value from rd()
    std::uniform_int_distribution<> distr(4, 6);    // unifor_int_distribution make sure the value is between min and max

    // Display the id of the current thread
    std::unique_lock<std::mutex> lck(mutex_);
    std::cout << "TrafficLight #" << _id << "::CycleThroughPhases: thread id = " << std::this_thread::get_id() << std::endl;
    lck.unlock();

    // Initialize variables
    int cycleDuration = distr(eng);     // Duration of a cycle is randomly choosen between 4 to 6

    // Init stop watch
    auto lastUpdate = std::chrono::system_clock::now();
    while(true)
    {
        // Sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // Compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - lastUpdate).count();

        // Is is time to toogle our traffic light
        if(timeSinceLastUpdate >= cycleDuration)
        {
            // Toogle current phase of traffic light
            if(currentPhase_ == red)
                currentPhase_ = green;
            else
                currentPhase_ = red;
            
            auto msg = currentPhase_;
            auto isSent = std::async(std::launch::async, & MessageQueue<TrafficLightPhase>::send, msgQueue_, std::move(msg));
            isSent.wait();

            // Reset stop watch for next cycle
            lastUpdate = std::chrono::system_clock::now();

            // Randomly choos the cycle duration for the next cycle
            cycleDuration = distr(eng);
        }
    }
}

void TrafficLight::simulate()
{
    _threads.emplace_back(std::thread(& TrafficLight::cycleThroughPhases, this));
}

void TrafficLight::waitForGreen() const
{
    while(true)
    {
        // Sleep at every iteration to reduce CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        // Wait until the traffic light is green, received from message queue
        auto currentPhase =  msgQueue_->receive();
        if(currentPhase == green)
            return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase() const
{
    return currentPhase_;
}