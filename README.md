# Goober-Eats
My implementation of Project 4 for CS 32 - Goober Eats, making deliveries to Westwood residents

For any number of valid geocoordinates in mapdata.txt, Goober Eats gives directions beginning from a "robot worker depot" to delivery locations within Westwood.

To see this in action, open P4.sln in Visual Studio and Run it. This will most likely take a few moments as the example delivery order for main.cpp has 7 stops.

To understand the algorithms designed, it's useful to dive into mapdata.txt first.

The file is a series of stanzas for Westwood streets. The first line is a street name in Westwood, the second is the number of linear segments for that street, and the remaining lines before the next street name are the geocoordinate point (latitude longitude) pairs making up those segments.

For example, consider 10th Helena Drive with only 1 segment. Plotting this on Google Maps, we can see the following.
<p float="left">
	<img src="https://i.imgur.com/ljb9uEb.png" width="400"/>
	<img src="https://i.imgur.com/QBSoUdS.png" width="180"/>
</p>

Altair Drive, with 5 segments, looks like this.
<p float="left">
	<img src="https://i.imgur.com/w0MCwHW.png" width="400"/>
	<img src="https://i.imgur.com/fMZgMxH.png" width="400"/>
</p>

This simple format thus cleverly captures Westwood streets' information in a way that the computer can understand. "Traveling" down a street simply means following sequential segments, while intersections are marked by the same geocoordinate appearing for mutliple streets in mapdata.txt.

For a fuller explanation of function implementations, read report.docx. In short, a StreetMap is constructed by loading mapdata.txt, constructing a hashtable with geocoordinate keys and values of StreetSegments - objects with the segment's street name and two geoocordinates. The PointToPointRouter generates the most efficient route between two points found by the A* algorithm ([read more here](https://www.geeksforgeeks.org/a-search-algorithm/)). Finally, DeliveryOptimizer uses simulated annealing to attempt different delivery orders until the optimal is found.

If you're touring Westwood soon, hopefully this can help!
