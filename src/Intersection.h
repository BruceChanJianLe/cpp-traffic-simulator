#ifndef INTERSECTION_H
#define INTERSECTION_H

#include <vector>
#include <future>
#include <mutex>
#include <memory>

#include "TrafficObject.h"
#include "TrafficLight.h"

// forward declarations to avoid include cycle
class Street;
class Vehicle;

class WaitingVehicles
{
    public:
        // Getter / Setter
        int getSize();

        // Typical behaviour methods
        void pushBack(const std::shared_ptr<Vehicle> & vehicle, std::promise<void> && promise);
        void permitEntryToFirstInQueue();

    private:
        // List of all vehicles waiting to enter this intersection
        std::vector<std::shared_ptr<Vehicle>> _vehicle;
        // List of associated promises
        std::vector<std::promise<void>> _promises;
        std::mutex _mutex;
};

class Intersection : public TrafficObject
{
public:
    // constructor / desctructor
    Intersection();

    // getters / setters
    void setIsBlocked(bool isBlocked);

    // typical behaviour methods
    void addVehicleToQueue(const std::shared_ptr<Vehicle> & vehicle);
    void addStreet(const std::shared_ptr<Street> & street);
    std::vector<std::shared_ptr<Street>> queryStreets(const std::shared_ptr<Street> & incoming); // return pointer to current list of all outgoing streets
    void simulate() override;
    void vehicleHasLeft(const std::shared_ptr<Vehicle> & vehicle);
    bool trafficLightIsGreen() const;

private:

    // typical behaviour methods
    void processVehicleQueue();

    // Private members
    TrafficLight trafficLight_;                      // TrafficLight thread
    std::vector<std::shared_ptr<Street>> _streets;   // list of all streets connected to this intersection
    WaitingVehicles _waitingVehicles;                // List of all vehicles and thier associated promises waiting to enter the intersection
    bool _isBlocked;                                 // Flag indicating whether the intersection is blocked by a vehicle
};

#endif