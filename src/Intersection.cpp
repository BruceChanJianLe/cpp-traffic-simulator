#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <random>

#include "Street.h"
#include "Intersection.h"
#include "Vehicle.h"


// Implementation of class WaitingVehicles
int WaitingVehicles::getSize()
{
    std::unique_lock<std::mutex> lck(_mutex);
    return _vehicle.size();
}

void WaitingVehicles::pushBack(const std::shared_ptr<Vehicle> & vehicle, std::promise<void> && promise)
{
    std::unique_lock<std::mutex> lck(_mutex);
    _vehicle.push_back(vehicle);
    _promises.push_back(std::move(promise));
}

void WaitingVehicles::permitEntryToFirstInQueue()
{
    std::unique_lock<std::mutex> lck(_mutex);
    // Obtain entries from the front of the both queues
    auto first_promise = _promises.begin();
    auto first_vehicle = _vehicle.begin();

    // Returning the promise for addVehicleToQueue()
    first_promise->set_value();

    _promises.erase(first_promise);
    _vehicle.erase(first_vehicle);
}

// Implementation of class Intersection
Intersection::Intersection()
{
    _type = ObjectType::objectIntersection;
    _isBlocked = false;
}

void Intersection::addStreet(const std::shared_ptr<Street> & street)
{
    _streets.push_back(street);
}

std::vector<std::shared_ptr<Street>> Intersection::queryStreets(const std::shared_ptr<Street> & incoming)
{
    // store all outgoing streets in a vector ...
    std::vector<std::shared_ptr<Street>> outgoings;
    for (const auto & it : _streets)
    {
        if (incoming->getID() != it->getID()) // ... except the street making the inquiry
        {
            outgoings.push_back(it);
        }
    }

    return outgoings;
}

void Intersection::addVehicleToQueue(const std::shared_ptr<Vehicle> & vehicle)
{
    std::unique_lock<std::mutex> lck(_mtx);
    std::cout << "Intersection #" << _id << "::addVehicleToQueue: thread id = " << std::this_thread::get_id() << std::endl;
    lck.unlock();
    // Add new vehicle to waiting line
    std::promise<void> prms_vehicle_allowed_to_enter;
    std::future<void> ftr_vehicle_allowed_to_enter = prms_vehicle_allowed_to_enter.get_future();
    _waitingVehicles.pushBack(vehicle, std::move(prms_vehicle_allowed_to_enter));

    // Wait until vehicle is allowed to enter
    ftr_vehicle_allowed_to_enter.wait();

    lck.lock();
    std::cout << "Intersection #" << _id << ": Vehicle #" << vehicle->getID() << " is granted entry." << std::endl;
    lck.unlock();

    // If traffic light is red wait until light turns green
    if(trafficLight_.getCurrentPhase() == red)
        trafficLight_.waitForGreen();
}

void Intersection::vehicleHasLeft(const std::shared_ptr<Vehicle> & vehicle)
{

    std::unique_lock<std::mutex> lck(_mtx);
    std::cout << "Intersection #" << _id << ": Vehicle #" << vehicle->getID() << " has left." << std::endl;
    lck.unlock();

    // Unblock queue processing
    this->setIsBlocked(false);
}

void Intersection::setIsBlocked(bool isBlocked)
{
    _isBlocked = isBlocked;
    std::unique_lock<std::mutex> lck(_mtx);
    std::cout << "Intersection #" << _id << ": isBlocked=" << _isBlocked << std::endl;
    lck.unlock();
}

// Virtual function which is executed in thread
void Intersection::simulate()
{
    // Launch traffic light processing in a thread
    trafficLight_.simulate();

    // Launch vehicle queue processing in a thread
    _threads.emplace_back(std::thread(& Intersection::processVehicleQueue, this));
}

void Intersection::processVehicleQueue()
{
    std::unique_lock<std::mutex> lck(_mtx);
    std::cout << "Intersection #" << _id << "::processVehicleQueue: thread id = " << std::this_thread::get_id() << std::endl;
    lck.unlock();
    
    // Continuously process the vehicle queue
    while(true)
    {
        // Sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        // Proceed only when at least one vehicle is waiting in the queue
        if(_waitingVehicles.getSize() > 0 && !_isBlocked)
        {
            // Set the intersection to block to prevent other vehicle from entering
            this->setIsBlocked(true);
            // Permit entry to first vehicle in the queue (FIFO)
            _waitingVehicles.permitEntryToFirstInQueue();
        }
    }
}

bool Intersection::trafficLightIsGreen() const
{
    return trafficLight_.getCurrentPhase() == green;
}