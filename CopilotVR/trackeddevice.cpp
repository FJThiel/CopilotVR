#include "trackeddevice.h"

TrackedDevice::TrackedDevice()
{

}

void TrackedDevice::setDeviceIndex(int index){
    m_deviceIndex = index;
}

void TrackedDevice::setSerial(char *serial){
    m_serial = std::string(serial);
}

void TrackedDevice::setManufacturer(char *manufacturer){
    m_manufacturer = std::string(manufacturer);
}

void TrackedDevice::setModel(char *model){
    m_model = std::string(model);
}

void TrackedDevice::setDeviceClassStr(std::string deviceClassStr){
    m_deviceClassStr = deviceClassStr;
}

int TrackedDevice::getDeviceIndex(){
    return m_deviceIndex;
}

std::string TrackedDevice::getSerial(){
    return m_serial;
}

std::string TrackedDevice::getManufacturer(){
    return m_manufacturer;
}

std::string TrackedDevice::getModel(){
    return m_model;
}

std::string TrackedDevice::getDeviceClassStr(){
    return m_deviceClassStr;
}
