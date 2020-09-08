// Dean Jones
// 005-299-127

#include "provided.h"
#include <list>
#include <iostream>
#include "ExpandableHashMap.h"
using namespace std;

// PointToPointRouter implementation
class PointToPointRouterImpl
{
public:
	// Constructs object with const pointer to StreetMap
	PointToPointRouterImpl(const StreetMap* sm);
	~PointToPointRouterImpl();
	// Generates route found by A* algorithm between start and end
	DeliveryResult generatePointToPointRoute(
		const GeoCoord& start,
		const GeoCoord& end,
		list<StreetSegment>& route,
		double& totalDistanceTravelled) const;
private:
	// Takes pointer passed to constructor
	const StreetMap* m_sm;
	// Private struct for A* algorithm use
	struct PointNode {
		GeoCoord gc;
		double f_cost;
		double g_cost;
		double h_cost;
		PointNode* parent;
		string name;
	};	
};

// Passes const StreetMap* to m_sm
PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm) : m_sm(sm)
{
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
}

// Route computed by A* between start and end is stored and total distance is recorded
DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
	const GeoCoord& start,
	const GeoCoord& end,
	list<StreetSegment>& route,
	double& totalDistanceTravelled) const
{
	// Check that the start and end coordinates are valid
	vector<StreetSegment> segs;
	// If no segments connect to the start or end, bad coordinates were passed
	if (!m_sm->getSegmentsThatStartWith(start, segs) || !m_sm->getSegmentsThatStartWith(end, segs)) {
		return BAD_COORD;
	}

	// If the start matches the end...
	if (start == end) {
		route.clear();
		totalDistanceTravelled = 0;
		return DELIVERY_SUCCESS;
	}

	// Initialized the start node and the open/closed lists (open gets start)
	// Open list refers to the points that are yet to be explored (neighbors of what we've explored)
	// Closed list refers to points already explored
	PointNode* startNode = new PointNode{start, 0, 0, 0, nullptr, ""};

	list<PointNode*> open;
	list<PointNode*> closed;

	open.push_back(startNode);

	// While the open list isn't empty,
	while (!open.empty()) {
		// Find the node with the smallest f
		double min_cost = 12500; // Approximately the longest possible distance between two points on Earth
		list<PointNode*>::iterator min_i;
		for (list<PointNode*>::iterator oi = open.begin(); oi != open.end(); ++oi) {
			if ((*oi)->f_cost < min_cost) {
				min_cost = (*oi)->f_cost;
				min_i = oi;
			}
		}
		// Node will be called parent from now on and removed from the open list
		PointNode* parent = (*min_i);
		open.erase(min_i);

		// Closed takes the parent popped from open
		closed.push_back(parent);

		// Get the segments from this point
		m_sm->getSegmentsThatStartWith(parent->gc, segs);

		// For all adjacent points...
		for (vector<StreetSegment>::iterator vi = segs.begin(); vi != segs.end(); ++vi) {
			GeoCoord next{ vi->end.latitudeText, vi->end.longitudeText };
			string nextName = vi->name;

			// If the point is the goal, stop the search
			if (next == end) {
				// Empty the route of any existing street segments
				route.clear();

				// Total distance is distance from the end plus the g_cost of the parent (g_cost is distance from the start to the parent)
				totalDistanceTravelled = distanceEarthMiles(end, parent->gc) + parent->g_cost;

				// Last segment is between the parent and end, with its name conveniently in nextName
				StreetSegment lastSeg{ parent->gc, end, nextName };
				route.push_front(lastSeg);

				// While there's a previous coordinate...
				while (parent->parent != nullptr) {
					// Add the segment to the route
					StreetSegment newSeg{ parent->parent->gc, parent->gc, parent->name };
					route.push_front(newSeg);
					parent = parent->parent;
				}
				// Free dynamically allocated array in open and closed arrays
				for (list<PointNode*>::iterator oi = open.begin(); oi != open.end(); ++oi) {
					delete (*oi);
				}

				for (list<PointNode*>::iterator ci = closed.begin(); ci != closed.end(); ++ci) {
					delete (*ci);
				}

				return DELIVERY_SUCCESS;
			}

			// Otherwise...
			// G cost is the parent's g cost + distance between parent and adjacent node
			double g_cost = distanceEarthMiles(next, parent->gc) + parent->g_cost;
			// H cost is the distance from the point to the end (heuristic for what it will take to get to the end)
			double h_cost = distanceEarthMiles(next, end);
			// F cost is G cost + H cost
			double f_cost = g_cost + h_cost;

			// Build a new PointNode with the parameters
			PointNode* newNextNode = new PointNode{ next, f_cost, g_cost, h_cost, parent, nextName};

			bool addNode = true;
			bool keep = false;
			// Check for node with same position as successor in open list
			for (list<PointNode*>::iterator oi = open.begin(); oi != open.end(); ++oi) {
				if ((*oi)->gc == next) {
					addNode = false;
					// If it's already in the list with a greater f cost
					if ((*oi)->f_cost > f_cost) {
						delete (*oi);
						*oi = newNextNode;
						keep = true;
					}
					break;
				}
			}

			// Check for node with same position as successor in closed list
			for (list<PointNode*>::iterator ci = closed.begin(); ci != closed.end(); ++ci) {
				if ((*ci)->gc == next) {
					addNode = false;
					// If it's already there with bigger f
					if ((*ci)->f_cost > f_cost) {
						delete (*ci);
						*ci = newNextNode;
						keep = true;
					}
					break;
				}
			}

			// Node wasn't in closed/open lists and should be added
			if (addNode) {
				keep = true;
				open.push_back(newNextNode);
			}
			if (!keep) {
				delete newNextNode;
			}
		}
	}
	// Free everything in the closed list (open list is already empty if we're here)
	for (list<PointNode*>::iterator ci = closed.begin(); ci != closed.end(); ++ci) {
		delete (*ci);
	}
	// No route was found
	return NO_ROUTE;
}

//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
	m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
	delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
	const GeoCoord& start,
	const GeoCoord& end,
	list<StreetSegment>& route,
	double& totalDistanceTravelled) const
{
	return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}
