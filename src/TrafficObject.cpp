#include <iostream>
#include <chrono>
#include <algorithm>
#include "TrafficObject.h"

// init static variable
int TrafficObject::_idCnt = 0;
std::mutex TrafficObject::_mtx;

void TrafficObject::setPosition(double x, double y)
{
    _posX = x;
    _posY = y;
}

void TrafficObject::getPosition(double &x, double &y)
{
    x = _posX;
    y = _posY;
}

TrafficObject::TrafficObject()
{
    _type = ObjectType::noObject;
    _id = _idCnt++;
}

TrafficObject::~TrafficObject()
{
    // Setup thread barrier before this object is destroyed
    std::for_each(_threads.begin(), _threads.end(), [](std::thread &t){
        t.join();
    });
}
