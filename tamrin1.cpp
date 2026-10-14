#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

const int NOT_FOUND = -1;
const int REGULAR_RATE = 2;
const int ELECTRIC_RATE = 5;

const string REGULAR_TYPE = "regular";
const string ELECTRIC_TYPE = "electric";

const string ADD_STATION_COMMAND = "add_station";
const string ADD_BIKE_COMMAND = "add_bike";
const string RENT_BIKE_COMMAND = "rent_bike";
const string RETURN_BIKE_COMMAND = "return_bike";
const string STATION_STATUS_COMMAND = "station_status";
const string REPORT_USERS_COMMAND = "report_users";

const string NO_ERROR_MESSAGE = "";
const string OK_MESSAGE = "OK";
const string STATION_EXISTS_ERROR = "Station already exists";
const string STATION_FULL_ERROR = "Station is full";
const string ACTIVE_RIDE_ERROR = "User already has an active ride";
const string NO_BIKE_ERROR = "No available bike of this type";
const string NO_ACTIVE_RIDE_ERROR = "User does not have an active ride";
const string NO_RIDES_MESSAGE = "No rides completed yet";
const string NO_BIKES_MESSAGE = "None";
const string LIST_SEPARATOR = ", ";

struct Bike {
    string id;
    string type;
};

struct Station {
    string id;
    int capacity;
    vector<Bike> bikes;
};

struct Ride {
    Bike bike;
    long long startTime = 0;
};

struct User {
    string id;
    bool hasActiveRide = false;
    Ride currentRide;
    int finishedRides = 0;
    long long totalTime = 0;
    long long totalCost = 0;
};

struct FinishedRide {
    long long duration;
    long long cost;
};

struct BikeHub {
    vector<Station> stations;
    vector<User> users;
};

int findStationIndex(const BikeHub& hub, const string& stationId)
{
    for (size_t i = 0; i < hub.stations.size(); i++) {
        if (hub.stations[i].id == stationId) {
            return static_cast<int>(i);
        }
    }
    return NOT_FOUND;
}

int findUserIndex(const BikeHub& hub, const string& userId)
{
    for (size_t i = 0; i < hub.users.size(); i++) {
        if (hub.users[i].id == userId) {
            return static_cast<int>(i);
        }
    }
    return NOT_FOUND;
}

int findSmallestBikeIndex(const Station& station, const string& type)
{
    int smallest = NOT_FOUND;
    for (size_t i = 0; i < station.bikes.size(); i++) {
        if (station.bikes[i].type != type) {
            continue;
        }
        if (smallest == NOT_FOUND || station.bikes[i].id < station.bikes[smallest].id) {
            smallest = static_cast<int>(i);
        }
    }
    return smallest;
}

Station& getStation(BikeHub& hub, const string& stationId)
{
    return hub.stations[findStationIndex(hub, stationId)];
}

User& getUser(BikeHub& hub, const string& userId)
{
    return hub.users[findUserIndex(hub, userId)];
}

bool stationExists(const BikeHub& hub, const string& stationId)
{
    return findStationIndex(hub, stationId) != NOT_FOUND;
}

bool hasActiveRide(const BikeHub& hub, const string& userId)
{
    int index = findUserIndex(hub, userId);
    return index != NOT_FOUND && hub.users[index].hasActiveRide;
}

bool isStationFull(const Station& station)
{
    return static_cast<int>(station.bikes.size()) >= station.capacity;
}

bool hasBikeOfType(const Station& station, const string& type)
{
    return findSmallestBikeIndex(station, type) != NOT_FOUND;
}

bool hasFinishedRides(const User& user)
{
    return user.finishedRides > 0;
}

int getRate(const string& type)
{
    if (type == ELECTRIC_TYPE) {
        return ELECTRIC_RATE;
    }
    return REGULAR_RATE;
}

int countBikesOfType(const Station& station, const string& type)
{
    int count = 0;
    for (const Bike& bike : station.bikes) {
        if (bike.type == type) {
            count++;
        }
    }
    return count;
}

bool compareBikesById(const Bike& first, const Bike& second)
{
    return first.id < second.id;
}

bool compareUsers(const User& first, const User& second)
{
    if (first.totalTime != second.totalTime) {
        return first.totalTime > second.totalTime;
    }
    return first.id < second.id;
}

vector<Bike> getSortedBikes(const Station& station)
{
    vector<Bike> sortedBikes = station.bikes;
    sort(sortedBikes.begin(), sortedBikes.end(), compareBikesById);
    return sortedBikes;
}

vector<User> getUsersWithFinishedRides(const BikeHub& hub)
{
    vector<User> users;
    for (const User& user : hub.users) {
        if (hasFinishedRides(user)) {
            users.push_back(user);
        }
    }
    return users;
}

vector<User> getSortedReportUsers(const BikeHub& hub)
{
    vector<User> users = getUsersWithFinishedRides(hub);
    sort(users.begin(), users.end(), compareUsers);
    return users;
}

string getAddStationError(const BikeHub& hub, const string& stationId)
{
    if (stationExists(hub, stationId)) {
        return STATION_EXISTS_ERROR;
    }
    return NO_ERROR_MESSAGE;
}

string getStationFullError(const Station& station)
{
    if (isStationFull(station)) {
        return STATION_FULL_ERROR;
    }
    return NO_ERROR_MESSAGE;
}

string getRentBikeError(const BikeHub& hub, const string& userId,
                        const Station& station, const string& type)
{
    if (hasActiveRide(hub, userId)) {
        return ACTIVE_RIDE_ERROR;
    }
    if (!hasBikeOfType(station, type)) {
        return NO_BIKE_ERROR;
    }
    return NO_ERROR_MESSAGE;
}

string getReturnBikeError(const BikeHub& hub, const string& userId, const Station& station)
{
    if (!hasActiveRide(hub, userId)) {
        return NO_ACTIVE_RIDE_ERROR;
    }
    return getStationFullError(station);
}

void addStation(BikeHub& hub, const string& stationId, int capacity)
{
    hub.stations.push_back({stationId, capacity, {}});
}

void putBikeInStation(Station& station, const Bike& bike)
{
    station.bikes.push_back(bike);
}

Bike takeBikeFromStation(Station& station, int bikeIndex)
{
    Bike bike = station.bikes[bikeIndex];
    station.bikes.erase(station.bikes.begin() + bikeIndex);
    return bike;
}

void addUserIfMissing(BikeHub& hub, const string& userId)
{
    if (findUserIndex(hub, userId) == NOT_FOUND) {
        User user;
        user.id = userId;
        hub.users.push_back(user);
    }
}

void startRide(User& user, const Bike& bike, long long startTime)
{
    user.hasActiveRide = true;
    user.currentRide = {bike, startTime};
}

FinishedRide calculateFinishedRide(const Ride& ride, long long endTime)
{
    long long duration = endTime - ride.startTime;
    return {duration, duration * getRate(ride.bike.type)};
}

void finishRide(User& user, const FinishedRide& finished)
{
    user.hasActiveRide = false;
    user.finishedRides++;
    user.totalTime += finished.duration;
    user.totalCost += finished.cost;
}

void rentBike(BikeHub& hub, const string& userId, Station& station,
              const string& type, long long time)
{
    Bike bike = takeBikeFromStation(station, findSmallestBikeIndex(station, type));
    addUserIfMissing(hub, userId);
    startRide(getUser(hub, userId), bike, time);
}

FinishedRide returnBike(BikeHub& hub, const string& userId, Station& station, long long time)
{
    User& user = getUser(hub, userId);
    FinishedRide finished = calculateFinishedRide(user.currentRide, time);
    putBikeInStation(station, user.currentRide.bike);
    finishRide(user, finished);
    return finished;
}

void printResult(const string& error)
{
    if (error == NO_ERROR_MESSAGE) {
        cout << OK_MESSAGE << endl;
        return;
    }
    cout << error << endl;
}

void printFinishedRide(const FinishedRide& finished)
{
    cout << "Ride finished: duration = " << finished.duration
         << " mins, cost = " << finished.cost << endl;
}

void printBikeList(const vector<Bike>& bikes)
{
    cout << "Bikes: ";
    if (bikes.empty()) {
        cout << NO_BIKES_MESSAGE << endl;
        return;
    }
    for (size_t i = 0; i < bikes.size(); i++) {
        if (i > 0) {
            cout << LIST_SEPARATOR;
        }
        cout << bikes[i].id;
    }
    cout << endl;
}

void printStationStatus(const Station& station)
{
    int total = static_cast<int>(station.bikes.size());
    cout << "Station " << station.id << ":" << endl;
    cout << "Capacity: " << station.capacity << endl;
    cout << "Available bikes: " << total
         << " (Regular: " << countBikesOfType(station, REGULAR_TYPE)
         << ", Electric: " << countBikesOfType(station, ELECTRIC_TYPE) << ")" << endl;
    cout << "Empty slots: " << station.capacity - total << endl;
    printBikeList(getSortedBikes(station));
}

void printUserReport(const vector<User>& users)
{
    if (users.empty()) {
        cout << NO_RIDES_MESSAGE << endl;
        return;
    }
    for (const User& user : users) {
        cout << "User " << user.id << ": Total time = " << user.totalTime
             << " mins, Total cost = " << user.totalCost << endl;
    }
}

void handleAddStation(BikeHub& hub)
{
    string stationId;
    int capacity;
    cin >> stationId >> capacity;
    string error = getAddStationError(hub, stationId);
    if (error == NO_ERROR_MESSAGE) {
        addStation(hub, stationId, capacity);
    }
    printResult(error);
}

void handleAddBike(BikeHub& hub)
{
    string bikeId, stationId, type;
    cin >> bikeId >> stationId >> type;
    Station& station = getStation(hub, stationId);
    string error = getStationFullError(station);
    if (error == NO_ERROR_MESSAGE) {
        putBikeInStation(station, {bikeId, type});
    }
    printResult(error);
}

void handleRentBike(BikeHub& hub)
{
    string userId, stationId, type;
    long long time;
    cin >> userId >> stationId >> type >> time;
    Station& station = getStation(hub, stationId);
    string error = getRentBikeError(hub, userId, station, type);
    if (error == NO_ERROR_MESSAGE) {
        rentBike(hub, userId, station, type, time);
    }
    printResult(error);
}

void handleReturnBike(BikeHub& hub)
{
    string userId, stationId;
    long long time;
    cin >> userId >> stationId >> time;
    Station& station = getStation(hub, stationId);
    string error = getReturnBikeError(hub, userId, station);
    if (error != NO_ERROR_MESSAGE) {
        cout << error << endl;
        return;
    }
    printFinishedRide(returnBike(hub, userId, station, time));
}

void handleStationStatus(BikeHub& hub)
{
    string stationId;
    cin >> stationId;
    printStationStatus(getStation(hub, stationId));
}

void handleReportUsers(const BikeHub& hub)
{
    printUserReport(getSortedReportUsers(hub));
}

void executeCommand(BikeHub& hub, const string& command)
{
    if (command == ADD_STATION_COMMAND) {
        handleAddStation(hub);
    } else if (command == ADD_BIKE_COMMAND) {
        handleAddBike(hub);
    } else if (command == RENT_BIKE_COMMAND) {
        handleRentBike(hub);
    } else if (command == RETURN_BIKE_COMMAND) {
        handleReturnBike(hub);
    } else if (command == STATION_STATUS_COMMAND) {
        handleStationStatus(hub);
    } else if (command == REPORT_USERS_COMMAND) {
        handleReportUsers(hub);
    }
}

int main()
{
    BikeHub hub;
    string command;
    while (cin >> command) {
        executeCommand(hub, command);
    }
    return 0;
}
