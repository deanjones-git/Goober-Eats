// Dean Jones
// 005-299-127

#include "provided.h"
#include <vector>
#include <math.h>
#include <cstdlib>
using namespace std;

class DeliveryOptimizerImpl
{
public:
	// Constructor takes const StreetMap pointer, which is really unnecessary however
	DeliveryOptimizerImpl(const StreetMap* sm);
	~DeliveryOptimizerImpl();
	// Gets a "good enough" order of deliveries to reduce travel distance
	void optimizeDeliveryOrder(
		const GeoCoord& depot,
		vector<DeliveryRequest>& deliveries,
		double& oldCrowDistance,
		double& newCrowDistance) const;
private:
	// Calculates crow distance with depot and deliveries
	double calculateCrowDistance(const GeoCoord& depot, vector<DeliveryRequest>& deliveries) const;
	// Reverses deliveries falling between first index and second index 
	vector<DeliveryRequest> reverseBetween(int firstIndex, int secondIndex, vector<DeliveryRequest>& deliveries) const;
};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
{
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{
}

// Uses simulated annealing to get better delivery order than original input
void DeliveryOptimizerImpl::optimizeDeliveryOrder(
	const GeoCoord& depot,
	vector<DeliveryRequest>& deliveries,
	double& oldCrowDistance,
	double& newCrowDistance) const
{
	// Get old crow distance with the depot and initial delivery vector
	oldCrowDistance = calculateCrowDistance(depot, deliveries);
	newCrowDistance = oldCrowDistance;

	// Simulated annealing parameters
	double temp = 3;
	double coolingRate = 0.1;

	// Simulated annealing to attempt to get better route
	// Algorithm runs in O(N^3.5)
	for (int iteration = 0; iteration < pow(deliveries.size(), 2.5); ++iteration) {

		// Getting random first and second index to reverse all elements between them
		int firstIndex, secondIndex;
		// First index can be everything from -1 to n - 3 (where n is last index) so that there's a minimum 2 elements between it and second index
		firstIndex = rand() % (deliveries.size() - 2) - 1;
		// Second index can be everything from first index + 3 to n + 1
		secondIndex = deliveries.size() - (rand() % (deliveries.size() - 2 - firstIndex));
		// Reverse everything between the indexes
		vector<DeliveryRequest> newDeliveries = reverseBetween(firstIndex, secondIndex, deliveries);
		
		// Get the new crow distance from this reordered deliveries
		double newDistance = calculateCrowDistance(depot, newDeliveries);
		
		// If new crow distance is better than the last iteration, accept it unconditionally
		if (newDistance < newCrowDistance) {
			newCrowDistance = newDistance;
			deliveries = newDeliveries;
		}
		// If it's longer...
		else if (newDistance > newCrowDistance) {
			// Accept Probability
			double acceptProb = exp((newCrowDistance - newDistance) / temp);
			int accept = acceptProb * 10000;

			// Use randomness to determine whether this longer route should be accepted
			int random = rand() % 10000;
			if (random <= accept) {
				newCrowDistance = newDistance;
				deliveries = newDeliveries;
			}
		}
		// Reduce temperature before next iteration
		temp *= (1.0-coolingRate);
	}
}

double DeliveryOptimizerImpl::calculateCrowDistance(const GeoCoord& depot, vector<DeliveryRequest>& deliveries) const {
	// Initial distance is 0
	double distance = 0;

	if (deliveries.size() == 0) {
		return distance;
	}
	// Add distance from depot to first delivery
	distance += distanceEarthMiles(depot, deliveries[0].location);

	// Add distance from each delivery to next
	for (int delivery = 0; delivery < deliveries.size() - 1; ++delivery) {
		distance += distanceEarthMiles(deliveries[delivery].location, deliveries[delivery + 1].location);
	}

	// Add distance from last delivery to depot
	distance += distanceEarthMiles(deliveries[deliveries.size() - 1].location, depot);
	return distance;
}

// Returns delivery vector reversing everything between first and second index in deliveries
vector<DeliveryRequest> DeliveryOptimizerImpl::reverseBetween(int firstIndex, int secondIndex, vector<DeliveryRequest>& deliveries) const {
	vector<DeliveryRequest> newDeliveries = deliveries;
	// Move to midpoint between firstIndex and secondIndex and swap everything on the way
	for (int swap = 1; swap < (secondIndex - firstIndex) / 2 + (secondIndex - firstIndex) % 2; ++swap) {
		newDeliveries[firstIndex + swap] = deliveries[secondIndex - swap];
		newDeliveries[secondIndex - swap] = deliveries[firstIndex + swap];
	}
	return newDeliveries;
}


//******************** DeliveryOptimizer functions ****************************

// These functions simply delegate to DeliveryOptimizerImpl's functions.
// You probably don't want to change any of this code.

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
	m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
	delete m_impl;
}

void DeliveryOptimizer::optimizeDeliveryOrder(
	const GeoCoord& depot,
	vector<DeliveryRequest>& deliveries,
	double& oldCrowDistance,
	double& newCrowDistance) const
{
	return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}
