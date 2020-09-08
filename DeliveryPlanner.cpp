// Dean Jones
// 005-299-127

#include "provided.h"
#include <vector>
using namespace std;

class DeliveryPlannerImpl
{
public:
	// Constructor takes const StreetMap pointer and stores it
	DeliveryPlannerImpl(const StreetMap* sm);
	~DeliveryPlannerImpl();
	// Generates delivery plan from depot to all deliveries
	DeliveryResult generateDeliveryPlan(
		const GeoCoord& depot,
		const vector<DeliveryRequest>& deliveries,
		vector<DeliveryCommand>& commands,
		double& totalDistanceTravelled) const;
private:
	const StreetMap* m_sm;
};

// Records StreetMap* pointer
DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm): m_sm(sm)
{
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}

// Stores delivery commands and total distance travelled for valid inputs
DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
	const GeoCoord& depot,
	const vector<DeliveryRequest>& deliveries,
	vector<DeliveryCommand>& commands,
	double& totalDistanceTravelled) const
{
	// Optimized the delivery order with the DeliveryOptimizer class
	vector<DeliveryRequest> deliveriesCopy = deliveries;
	double originalCrowDistance, newCrowDistance;
	DeliveryOptimizer delOp(m_sm);
	delOp.optimizeDeliveryOrder(depot, deliveriesCopy, originalCrowDistance, newCrowDistance);

	// Get routes for each part of the trip
	list<list<StreetSegment>> routes;
	PointToPointRouter router(m_sm);
	list<StreetSegment> route;
	double routeDistance;
	DeliveryResult dr;

	double total = 0;


	if (deliveries.size() == 0) {
		commands.clear();
		totalDistanceTravelled = total;
		return DELIVERY_SUCCESS;
	}

	// Add route from depot to the first location
	dr = router.generatePointToPointRoute(depot, deliveriesCopy[0].location, route, routeDistance);
	// If not successful...
	if (dr != DELIVERY_SUCCESS) {
		return dr;
	}
	// Add route and distance travelled for this part of the trip
	routes.push_back(route);
	total += routeDistance;
	
	// Add routes between all deliveries
	for (int delivery = 0; delivery < deliveries.size() - 1; ++delivery) {
		dr = router.generatePointToPointRoute(deliveriesCopy[delivery].location, deliveriesCopy[delivery + 1].location, route, routeDistance);
		// If not successful...
		if (dr != DELIVERY_SUCCESS) {
			return dr;
		}
		routes.push_back(route);
		total += routeDistance;
	}

	// Add route from last delivery to depot
	dr = router.generatePointToPointRoute(deliveriesCopy[deliveries.size()-1].location, depot, route, routeDistance);
	if (dr != DELIVERY_SUCCESS) {
		return dr;
	}
	routes.push_back(route);
	total += routeDistance;

	// Update total distance travelled
	totalDistanceTravelled = total;


	// At this point, the input depot and deliveries must have been valid
	int delivery = 0;
	commands.clear();

	// For each route...
	for (list<list<StreetSegment>>::iterator ti = routes.begin(); ti != routes.end(); ++ti) {
		for (list<StreetSegment>::iterator ri = ti->begin(); ri != ti->end(); ++ri) {
			// Get this road's cardinal direction and its name
			double distanceRoad = 0;
			double degrees = angleOfLine(*ri);
			string dir;
			if (degrees < 22.5 || degrees >= 337.5) {
				dir = "east";
			}
			else if (degrees < 67.5) {
				dir = "northeast";
			}
			else if (degrees < 112.5) {
				dir = "north";
			}
			else if (degrees < 157.5) {
				dir = "northwest";
			}
			else if (degrees < 202.5) {
				dir = "west";
			}
			else if (degrees < 247.5) {
				dir = "southwest";
			}
			else if (degrees < 292.5) {
				dir = "south";
			}
			else {
				dir = "southeast";
			}
			string name = ri->name;

			// Until we reach a new street or the destination...
			for (;;) {
				// Get the distance for this particular road segment
				distanceRoad += distanceEarthMiles(ri->start, ri->end);
				++ri;
				// If the next segment is the end...
				if (ri == ti->end()) {
					// Proceed command is finalized and ready
					DeliveryCommand proceed;
					proceed.initAsProceedCommand(dir, name, distanceRoad);
					commands.push_back(proceed);
					--ri;
					break;
				}
				// If the next segment is a new street...
				if (ri->name != name) {

					// Proceed command is finalized and ready
					DeliveryCommand proceed;
					proceed.initAsProceedCommand(dir, name, distanceRoad);
					commands.push_back(proceed);

					// Get name of new street for turn command
					name = ri->name;
					// Get angle between streets
					StreetSegment secondStreet = *ri;
					--ri;
					StreetSegment firstStreet = *ri;
					double angle = angleBetween2Lines(firstStreet, secondStreet);
					// Turn left
					if (angle >= 1 && angle < 180) {
						DeliveryCommand turnLeft;
						turnLeft.initAsTurnCommand("left", name);
						commands.push_back(turnLeft);
					}
					// Turn right
					else if (angle >= 180 && angle <= 359) {
						DeliveryCommand turnRight;
						turnRight.initAsTurnCommand("right", name);
						commands.push_back(turnRight);
					}
					break;
				}
			}
		}
		// If this was a delivery...
		if (delivery < deliveriesCopy.size()) {
			// Delivery command can be added
			DeliveryCommand deliver;
			deliver.initAsDeliverCommand(deliveriesCopy[delivery].item);
			commands.push_back(deliver);
		}
		delivery++;
	}
	return DELIVERY_SUCCESS;
}

//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
	m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
	delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
	const GeoCoord& depot,
	const vector<DeliveryRequest>& deliveries,
	vector<DeliveryCommand>& commands,
	double& totalDistanceTravelled) const
{
	return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}
