#include <iostream>
#include <thread>
#include <condition_variable>

#include "AirportServer.h"


/**
*  Called by an Airplane when it wishes to land on a runway
*/
void AirportServer::reserveRunway(int airplaneNum, AirportRunways::RunwayNumber runway)
{
	// Acquire runway(s)
	{  // Begin critical region

		unique_lock<mutex> runwaysLock(runwaysMutex);

		{
			lock_guard<mutex> lk(AirportRunways::checkMutex);

			cout << endl << "Airplane #" << airplaneNum << " is acquiring any needed runway(s) for landing on Runway "
				 << AirportRunways::runwayName(runway) << endl;
		}

		std::cout << "------------------------------------------" << std::endl;
		for(auto i : runwayInUse){
			std::cout << runwayInUse[i] << std::endl;
		}
		std::cout << "------------------------------------------" << std::endl;

		// called runway that is currently in use
		if(runwayInUse[runway] >= 1){
			std::cout << "c1" << std::endl;
			runways[runway].wait(runwaysLock);
		}
		// called runway 9 but runway 4R is in use, wait
		if(((static_cast<int>(runway)) == static_cast<int>(RUNWAY_9)) && (static_cast<int>(runwayInUse[RUNWAY_4R]) > 0)){
			runways[RUNWAY_4R].wait(runwaysLock);
		}
		// called runway 9 but runway 15R is in use, wait
		if((static_cast<int>(runway) == RUNWAY_9) && (static_cast<int>(runwayInUse[RUNWAY_15R]) > 0)){
			runways[RUNWAY_15R].wait(runwaysLock);
		}
		// called runway 4R but runway 9 is in use, wait
		if(((static_cast<int>(runway)) == static_cast<int>(RUNWAY_4R)) && (static_cast<int>(runwayInUse[RUNWAY_9]) > 0)){
			runways[RUNWAY_9].wait(runwaysLock);
		}
		// called runway 15R but runway 9 is in use, wait
		if((static_cast<int>(runway) == RUNWAY_15R) && (static_cast<int>(runwayInUse[RUNWAY_9]) > 0)){
			runways[RUNWAY_9].wait(runwaysLock);
		}
		// called runway 15L but runway 4L is in use, wait
		if(((static_cast<int>(runway)) == static_cast<int>(RUNWAY_15L)) && (static_cast<int>(runwayInUse[RUNWAY_4L]) > 0)){
			runways[RUNWAY_4L].wait(runwaysLock);
		}
		// called runway 15L but runway 4R is in use, wait
		if((static_cast<int>(runway) == RUNWAY_15L) && (static_cast<int>(runwayInUse[RUNWAY_4R]) > 0)){
			runways[RUNWAY_4R].wait(runwaysLock);
		}
		// called runway 15R but runway 4L is in use, wait
		if(((static_cast<int>(runway)) == static_cast<int>(RUNWAY_15R)) && (static_cast<int>(runwayInUse[RUNWAY_4L]) > 0)){
			runways[RUNWAY_4L].wait(runwaysLock);
		}
		// called runway 15R but runway 4R is in use, wait
		if((static_cast<int>(runway) == RUNWAY_15R) && (static_cast<int>(runwayInUse[RUNWAY_4R]) > 0)){
			runways[RUNWAY_4R].wait(runwaysLock);
		}
		// called runway 4L but runway 15L is in use, wait
		if(((static_cast<int>(runway)) == static_cast<int>(RUNWAY_4L)) && (static_cast<int>(runwayInUse[RUNWAY_15L]) > 0)){
			runways[RUNWAY_15L].wait(runwaysLock);
		}
		// called runway 4L but runway 15R is in use, wait
		if((static_cast<int>(runway) == RUNWAY_4L) && (static_cast<int>(runwayInUse[RUNWAY_15R]) > 0)){
			runways[RUNWAY_15R].wait(runwaysLock);
		}
		// called runway 4R but runway 15L is in use, wait
		if(((static_cast<int>(runway)) == static_cast<int>(RUNWAY_4R)) && (static_cast<int>(runwayInUse[RUNWAY_15L]) > 0)){
			runways[RUNWAY_15L].wait(runwaysLock);
		}
		// called runway 4R but runway 15R is in use, wait
		if((static_cast<int>(runway) == RUNWAY_4R) && (static_cast<int>(runwayInUse[RUNWAY_15R]) > 0)){
			runways[RUNWAY_15R].wait(runwaysLock);
		}

		runwayInUse[runway]++;

		// Check status of the airport for any rule violations
		AirportRunways::checkAirportStatus(runway);

		runwaysLock.unlock();

	} // End critical region

	// obtain a seed from the system clock:
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);

	// Taxi for a random number of milliseconds
	std::uniform_int_distribution<int> taxiTimeDistribution(1, MAX_TAXI_TIME);
	int taxiTime = taxiTimeDistribution(generator);

	{
		lock_guard<mutex> lk(AirportRunways::checkMutex);

		cout << "Airplane #" << airplaneNum << " is taxiing on Runway " << AirportRunways::runwayName(runway)
			 << " for " << taxiTime << " milliseconds\n";
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(taxiTime));

} // end AirportServer::reserveRunway()


 /**
  *  Called by an Airplane when it is finished landing
  */
void AirportServer::releaseRunway(int airplaneNum, AirportRunways::RunwayNumber runway)
{
	// Release the landing runway and any other needed runways
	{ // Begin critical region

		unique_lock<mutex> runwaysLock(runwaysMutex);

		{
			lock_guard<mutex> lk(AirportRunways::checkMutex);

			cout << "Airplane #" << airplaneNum << " is releasing any needed runway(s) after landing on Runway "
				 << AirportRunways::runwayName(runway) << endl;
		}

//
		runwayInUse[runway]--;
		runways[runway].notify_one();
//

		// Update the status of the airport to indicate that the landing is complete
		AirportRunways::finishedWithRunway(runway);

		runwaysLock.unlock();

	} // End critical region

	// obtain a seed from the system clock:
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);

	// Wait for a random number of milliseconds before requesting the next landing for this Airplane
	std::uniform_int_distribution<int> waitTimeDistribution(1, MAX_WAIT_TIME);
	int waitTime = waitTimeDistribution(generator);

	{
		lock_guard<mutex> lk(AirportRunways::checkMutex);

		cout << "Airplane #" << airplaneNum << " is waiting for " << waitTime << " milliseconds before landing again\n";
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));

} // end AirportServer::releaseRunway()
