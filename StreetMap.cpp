// Dean Jones
// 005-299-127

#include "provided.h"
#include <string>
#include <list>
#include <functional>
#include <fstream>
#include "ExpandableHashMap.h"
using namespace std;

// Hash function for GeoCoord key
unsigned int hasher(const GeoCoord& g)
{
	return std::hash<string>()(g.latitudeText + g.longitudeText);
}

// StreetMap implementation
class StreetMapImpl
{
public:
	StreetMapImpl(); // Construct and
	~StreetMapImpl(); // destruct m_map
	bool load(string mapFile); // Load all data from indicated file
	bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const; 
	// Use m_map to get value for key (all street segments from that point)
private:
	// m_map will hold all GeoCoords and the street segments connected to it
	ExpandableHashMap<GeoCoord, vector<StreetSegment>>* m_map;
};

// Initialize m_map
StreetMapImpl::StreetMapImpl(): m_map(new ExpandableHashMap<GeoCoord, vector<StreetSegment>>)
{
}

// Delete m_map and all StreetSegment pointers of m_segmentsList
StreetMapImpl::~StreetMapImpl()
{
	delete m_map;
}

// Read data from mapFile
bool StreetMapImpl::load(string mapFile)
{
	// Try to read mapFile...
	ifstream infile(mapFile);
	// If it's invalid...
	if (!infile) {
		cerr << "Error: Cannot open mapdata.txt!" << endl;
		return false;
	}
	
	// Otherwise, for each street in the file...
	string street;
	// Read street names until you reach the end of the file
	while (getline(infile, street)) {
		// Get the number of segments for the street
		int segNum;
		infile >> segNum;
		infile.ignore(10000, '\n');

		// For each segment
		for (int i = 0; i < segNum; ++i) {
			// Get the line with the start and end coordinates
			string line;
			getline(infile, line);
			istringstream iss(line);
			string lat1, long1, lat2, long2;

			// If it isn't two coordinates...
			if (!(iss >> lat1 >> long1 >> lat2 >> long2)) {
				cerr << "Ignoring badly-formatted street segment line: " << line << endl;
				continue;
			}

			// Otherwise...
			GeoCoord start(lat1, long1);
			GeoCoord end(lat2, long2);

			// Insert a street segment from the start to the end coordinate into the map
			vector<StreetSegment>* segmentsPtr = m_map->find(start);
			StreetSegment segment(start, end, street);
			if (segmentsPtr != nullptr) {
				segmentsPtr->push_back(segment);
			}
			else {
				vector<StreetSegment> segments;
				segments.push_back(segment);
				m_map->associate(start, segments);
			}

			// Insert a street segment from the end to the start coordiante into the map
			segmentsPtr = m_map->find(end);
			StreetSegment reverseSegment(end, start, street);
			if (segmentsPtr != nullptr) {
				segmentsPtr->push_back(reverseSegment);
			}
			else {
				vector<StreetSegment> segments;
				segments.push_back(reverseSegment);
				m_map->associate(end, segments);
			}
		}
	}
	// If everything succeeded, return true
	return true;
}

// Retrieves all StreetSegments (reversed too) whose start location matches gc, puts them in segs
bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
	// Get the pointer
	vector<StreetSegment>* segmentsPtr = m_map->find(gc);

	// If it's nullptr, the gc is invalid for this map
	if (segmentsPtr == nullptr)
		return false; 
	// Otherwise...
	segs = *segmentsPtr;
	return true;
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
	m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
	delete m_impl;
}

bool StreetMap::load(string mapFile)
{
	return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
	return m_impl->getSegmentsThatStartWith(gc, segs);
}