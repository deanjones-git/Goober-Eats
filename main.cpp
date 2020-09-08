#include "ExpandableHashMap.h"
#include "provided.h"
#include <vector>
#include <list>
#include <string>
#include <initializer_list>
#include <functional>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <thread>
#include <future>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <cassert>
//#include <unistd.h>

using namespace std;


int main() {

	// Initialize our StreetMap with the geocoordinates for Westwood
	StreetMap sm;
	sm.load("mapdata.txt");

	// Geocoordinates for our depot and delivery stops...
	GeoCoord depot("34.0625329", "-118.4470263");
	GeoCoord d1("34.0712323", "-118.4505969");
	GeoCoord d2("34.0687443", "-118.4449195");
	GeoCoord d3("34.0685657", "-118.4489289");
	GeoCoord d4("34.0687443", "-118.4449195");
	GeoCoord d5("34.0715554", "-118.4523116");
	GeoCoord d6("34.0734467", "-118.4407820");
	GeoCoord d7("34.0736044", "-118.4530482");

	// Add these to our delivery vector
	vector<DeliveryRequest> deliveries;
	deliveries.push_back(DeliveryRequest{"Chicken tenders", d1 });
	deliveries.push_back(DeliveryRequest{ "B-Plate salmon", d2 });
	deliveries.push_back(DeliveryRequest{ "Blue Ribbon beer", d3 });
	deliveries.push_back(DeliveryRequest{ "ENG VI computers", d4 });
	deliveries.push_back(DeliveryRequest{ "FEAST sushi", d5 });
	deliveries.push_back(DeliveryRequest{ "Royce programs", d6 });
	deliveries.push_back(DeliveryRequest{ "Hedrick hamburgers", d7 });
	double distance;

	// Create the Delivery Planner with our Westwood StreetMap and store the direction commands for the best route found
	DeliveryPlanner delp(&sm);
	vector<DeliveryCommand> commands;
	delp.generateDeliveryPlan(depot, deliveries, commands, distance);
	for (vector<DeliveryCommand>::iterator ci = commands.begin(); ci != commands.end(); ++ci) {
		cerr << ci->description() << endl;
	}

	// Trip length
	cerr << endl << "Total Distance travelled: " << distance << endl;
}