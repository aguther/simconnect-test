/*
 * Copyright 2020 Andreas Guther
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include <windows.h>
#include "SimConnect.h"
#include <strsafe.h>
#include <iostream>
#include <chrono>
#include <cmath>

using namespace std;

int quit = 0;
HANDLE hSimConnect = nullptr;

enum EVENTS {
  EVENT_FLIGHT_LOADED,
  EVENT_SIM_START,
  EVENT_VIEW,
  EVENT_POSITION_CHANGED,
  EVENT_AIRCRAFT_LOADED,
  EVENT_SIM,
};

struct SimData {
  double simulation_time;
} simData;

struct SimDataWrite {
  double pot;
} simDataWrite;

SimData oldSimData;

int incCounter = 0;
std::chrono::time_point<std::chrono::system_clock> lastIncTime;

int decCounter = 0;
std::chrono::time_point<std::chrono::system_clock> lastDecTime;

std::string getSimConnectExceptionString(
    SIMCONNECT_EXCEPTION exception
) {
  switch (exception) {
    case SIMCONNECT_EXCEPTION_NONE:
      return "NONE";

    case SIMCONNECT_EXCEPTION_ERROR:
      return "ERROR";

    case SIMCONNECT_EXCEPTION_SIZE_MISMATCH:
      return "SIZE_MISMATCH";

    case SIMCONNECT_EXCEPTION_UNRECOGNIZED_ID:
      return "UNRECOGNIZED_ID";

    case SIMCONNECT_EXCEPTION_UNOPENED:
      return "UNOPENED";

    case SIMCONNECT_EXCEPTION_VERSION_MISMATCH:
      return "VERSION_MISMATCH";

    case SIMCONNECT_EXCEPTION_TOO_MANY_GROUPS:
      return "TOO_MANY_GROUPS";

    case SIMCONNECT_EXCEPTION_NAME_UNRECOGNIZED:
      return "NAME_UNRECOGNIZED";

    case SIMCONNECT_EXCEPTION_TOO_MANY_EVENT_NAMES:
      return "TOO_MANY_EVENT_NAMES";

    case SIMCONNECT_EXCEPTION_EVENT_ID_DUPLICATE:
      return "EVENT_ID_DUPLICATE";

    case SIMCONNECT_EXCEPTION_TOO_MANY_MAPS:
      return "TOO_MANY_MAPS";

    case SIMCONNECT_EXCEPTION_TOO_MANY_OBJECTS:
      return "TOO_MANY_OBJECTS";

    case SIMCONNECT_EXCEPTION_TOO_MANY_REQUESTS:
      return "TOO_MANY_REQUESTS";

    case SIMCONNECT_EXCEPTION_WEATHER_INVALID_PORT:
      return "WEATHER_INVALID_PORT";

    case SIMCONNECT_EXCEPTION_WEATHER_INVALID_METAR:
      return "WEATHER_INVALID_METAR";

    case SIMCONNECT_EXCEPTION_WEATHER_UNABLE_TO_GET_OBSERVATION:
      return "WEATHER_UNABLE_TO_GET_OBSERVATION";

    case SIMCONNECT_EXCEPTION_WEATHER_UNABLE_TO_CREATE_STATION:
      return "WEATHER_UNABLE_TO_CREATE_STATION";

    case SIMCONNECT_EXCEPTION_WEATHER_UNABLE_TO_REMOVE_STATION:
      return "WEATHER_UNABLE_TO_REMOVE_STATION";

    case SIMCONNECT_EXCEPTION_INVALID_DATA_TYPE:
      return "INVALID_DATA_TYPE";

    case SIMCONNECT_EXCEPTION_INVALID_DATA_SIZE:
      return "INVALID_DATA_SIZE";

    case SIMCONNECT_EXCEPTION_DATA_ERROR:
      return "DATA_ERROR";

    case SIMCONNECT_EXCEPTION_INVALID_ARRAY:
      return "INVALID_ARRAY";

    case SIMCONNECT_EXCEPTION_CREATE_OBJECT_FAILED:
      return "CREATE_OBJECT_FAILED";

    case SIMCONNECT_EXCEPTION_LOAD_FLIGHTPLAN_FAILED:
      return "LOAD_FLIGHTPLAN_FAILED";

    case SIMCONNECT_EXCEPTION_OPERATION_INVALID_FOR_OBJECT_TYPE:
      return "OPERATION_INVALID_FOR_OBJECT_TYPE";

    case SIMCONNECT_EXCEPTION_ILLEGAL_OPERATION:
      return "ILLEGAL_OPERATION";

    case SIMCONNECT_EXCEPTION_ALREADY_SUBSCRIBED:
      return "ALREADY_SUBSCRIBED";

    case SIMCONNECT_EXCEPTION_INVALID_ENUM:
      return "INVALID_ENUM";

    case SIMCONNECT_EXCEPTION_DEFINITION_ERROR:
      return "DEFINITION_ERROR";

    case SIMCONNECT_EXCEPTION_DUPLICATE_ID:
      return "DUPLICATE_ID";

    case SIMCONNECT_EXCEPTION_DATUM_ID:
      return "DATUM_ID";

    case SIMCONNECT_EXCEPTION_OUT_OF_BOUNDS:
      return "OUT_OF_BOUNDS";

    case SIMCONNECT_EXCEPTION_ALREADY_CREATED:
      return "ALREADY_CREATED";

    case SIMCONNECT_EXCEPTION_OBJECT_OUTSIDE_REALITY_BUBBLE:
      return "OBJECT_OUTSIDE_REALITY_BUBBLE";

    case SIMCONNECT_EXCEPTION_OBJECT_CONTAINER:
      return "OBJECT_CONTAINER";

    case SIMCONNECT_EXCEPTION_OBJECT_AI:
      return "OBJECT_AI";

    case SIMCONNECT_EXCEPTION_OBJECT_ATC:
      return "OBJECT_ATC";

    case SIMCONNECT_EXCEPTION_OBJECT_SCHEDULE:
      return "OBJECT_SCHEDULE";

    default:
      return "UNKNOWN";
  }
}

void DispatchProcedure(
    SIMCONNECT_RECV *pData,
    DWORD *cbData
) {
  HRESULT hr;

  switch (pData->dwID) {
    case SIMCONNECT_RECV_ID_NULL: {
      cout << "SIMCONNECT_RECV_ID_NULL" << endl;
      break;
    }

    case SIMCONNECT_RECV_ID_EXCEPTION: {
      auto exception = static_cast<SIMCONNECT_EXCEPTION>(static_cast<SIMCONNECT_RECV_EXCEPTION *>(pData)->dwException);
      cout << "SIMCONNECT_RECV_ID_EXCEPTION = " << getSimConnectExceptionString(exception) << endl;
      break;
    }

    case SIMCONNECT_RECV_ID_OPEN: {
      cout << "SIMCONNECT_RECV_ID_OPEN" << endl;
      break;
    }

    case SIMCONNECT_RECV_ID_QUIT: {
      quit = 1;
      break;
    }

    case SIMCONNECT_RECV_ID_EVENT_FILENAME: {
      cout << "SIMCONNECT_RECV_ID_EVENT_FILENAME" << endl;
      break;
    }

    case SIMCONNECT_RECV_ID_EVENT: {
      auto *event = (SIMCONNECT_RECV_EVENT *) pData;
      switch (event->uEventID) {
        case EVENT_FLIGHT_LOADED: {
          cout << "EVENT_FLIGHT_LOADED = " << static_cast<long>(event->dwData) << endl;
          break;
        }

        case EVENT_SIM_START: {
          cout << "EVENT_SIM_START = " << static_cast<long>(event->dwData) << endl;
          break;
        }

        case EVENT_VIEW: {
          cout << "EVENT_VIEW = " << static_cast<long>(event->dwData) << endl;
          break;
        }

        case EVENT_POSITION_CHANGED: {
          cout << "EVENT_POSITION_CHANGED = " << static_cast<long>(event->dwData) << endl;
          break;
        }

        case EVENT_AIRCRAFT_LOADED: {
          cout << "EVENT_AIRCRAFT_LOADED = " << static_cast<long>(event->dwData) << endl;
          break;
        }

        case EVENT_SIM: {
          cout << "EVENT_SIM = " << static_cast<long>(event->dwData) << endl;
          break;
        }

        default:
          break;
      }
      break;
    }

    case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE: {
      auto *event = (SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE *) pData;
      simData = *((SimData *) &event->dwData);
      break;
    }

    default:
      cout << "EVENT TYPE = " << pData->dwID << endl;
      break;
  }
}

bool addSystemEvent(
    int eventId,
    const string &eventName,
    bool shouldMask = false
) {
  if (FAILED(SimConnect_SubscribeToSystemEvent(hSimConnect, eventId, eventName.c_str()))) {
    return false;
  }
  if (FAILED(SimConnect_AddClientEventToNotificationGroup(hSimConnect, 0, eventId, shouldMask ? 1 : 0))) {
    return false;
  }
  return true;
}

bool addEvent(
    int eventId,
    const string &eventName,
    bool shouldMask = false
) {
  if (FAILED(SimConnect_MapClientEventToSimEvent(hSimConnect, eventId, eventName.c_str()))) {
    return false;
  }
  if (FAILED(SimConnect_AddClientEventToNotificationGroup(hSimConnect, 0, eventId, shouldMask ? 1 : 0))) {
    return false;
  }
  return true;
}

bool isSimConnectDataTypeStruct(SIMCONNECT_DATATYPE type) {
  switch (type) {
    case SIMCONNECT_DATATYPE_INITPOSITION:
    case SIMCONNECT_DATATYPE_MARKERSTATE:
    case SIMCONNECT_DATATYPE_WAYPOINT:
    case SIMCONNECT_DATATYPE_LATLONALT:
    case SIMCONNECT_DATATYPE_XYZ:
      return true;

    default:
      return false;
  }
  return false;
}

bool addDataDefinition(const HANDLE connectionHandle,
                       const SIMCONNECT_DATA_DEFINITION_ID id,
                       const SIMCONNECT_DATATYPE dataType,
                       const string &dataName,
                       const string &dataUnit) {
  HRESULT result = SimConnect_AddToDataDefinition(
      connectionHandle, id, dataName.c_str(),
      isSimConnectDataTypeStruct(dataType) ? nullptr : dataUnit.c_str(), dataType);
  return (result == S_OK);
}

void processDispatchEvents() {
  DWORD cbData;
  SIMCONNECT_RECV *pData;
  while (SUCCEEDED(SimConnect_GetNextDispatch(hSimConnect, &pData, &cbData))) {
    DispatchProcedure(pData, &cbData);
  }
}

int main(int argc, char *argv[]) {
  if (SUCCEEDED(SimConnect_Open(&hSimConnect, "simconnect-test", nullptr, 0, nullptr, 2))) {
    cout << "Connected" << endl;

    addDataDefinition(hSimConnect, 0, SIMCONNECT_DATATYPE_FLOAT64, "SIMULATION TIME", "NUMBER");
//    addDataDefinition(hSimConnect, 0, SIMCONNECT_DATATYPE_FLOAT64, "GENERAL ENG THROTTLE LEVER POSITION:2", "PERCENT");

//    addDataDefinition(hSimConnect, 1, SIMCONNECT_DATATYPE_FLOAT64, "GENERAL ENG THROTTLE LEVER POSITION:2", "PERCENT");

//    addEvent(EVENT_LIGHT_POTENTIOMETER, "LIGHT_POTENTIOMETER_86_SET", false);

    addSystemEvent(EVENT_FLIGHT_LOADED, "FlightLoaded");
    addSystemEvent(EVENT_SIM_START, "SimStart");
    addSystemEvent(EVENT_VIEW, "View");
    addSystemEvent(EVENT_POSITION_CHANGED, "PositionChanged");
    addSystemEvent(EVENT_AIRCRAFT_LOADED, "AircraftLoaded");
    addSystemEvent(EVENT_SIM, "Sim");

    // set group priority
    SimConnect_SetNotificationGroupPriority(
        hSimConnect,
        0,
        SIMCONNECT_GROUP_PRIORITY_HIGHEST
    );

    // Check for messages only when a Windows event has been received
    while (0 == quit) {
      bool result = true;

      // request data
//      SimConnect_RequestDataOnSimObjectType(
//          hSimConnect,
//          0,
//          0,
//          0,
//          SIMCONNECT_SIMOBJECT_TYPE_USER
//      );

      // read data
      processDispatchEvents();

//      cout << "simulation_time = " << simData.simulation_time << endl;

//      SimConnect_TransmitClientEvent(
//          hSimConnect,
//          0,
//          EVENT_LIGHT_POTENTIOMETER,
//          50,
//          SIMCONNECT_GROUP_PRIORITY_HIGHEST,
//          SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY
//      );

//      simDataWrite.generalEngineThrottleLeverPosition_1 = 50;
//      simDataWrite.generalEngineThrottleLeverPosition_2 = 50;

      // set data
//      result = SimConnect_SetDataOnSimObject(
//          hSimConnect,
//          1,
//          SIMCONNECT_OBJECT_ID_USER,
//          0,
//          0,
//          sizeof(simDataWrite),
//          &simDataWrite
//      );

//      cout << "result = " << result << endl;

      // print out value
//      if (simData.ambientPressure != oldSimData.ambientPressure
//          || simData.pressureAltitude != oldSimData.pressureAltitude
//          || simData.indicatedAltitudeStd != oldSimData.indicatedAltitudeStd) {
//        // print values
//        cout << "P = " << simData.ambientPressure;
//        cout << " , PA(sim) = " << simData.pressureAltitude;
//        cout << " , IA(STD) = " << simData.indicatedAltitudeStd;
//        cout << " , PA(calc) = " << calculateAltitudeFromPressure(simData.ambientPressure, 1013.25, 1013.25);
//        cout << endl;
//        // store current values
//        oldSimData = simData;
//      }

//      if (argc == 2) {
//        int shouldFreeze = atoi(argv[1]) == 1;
//
//        SimConnect_TransmitClientEvent(
//            hSimConnect,
//            0,
//            EVENT_FREEZE_LATITUDE_LONGITUDE_SET,
//            shouldFreeze,
//            SIMCONNECT_GROUP_PRIORITY_HIGHEST,
//            SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY
//        );
//        SimConnect_TransmitClientEvent(
//            hSimConnect,
//            0,
//            EVENT_FREEZE_ALTITUDE_SET,
//            shouldFreeze,
//            SIMCONNECT_GROUP_PRIORITY_HIGHEST,
//            SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY
//        );
//        SimConnect_TransmitClientEvent(
//            hSimConnect,
//            0,
//            EVENT_FREEZE_ATTITUDE_SET,
//            shouldFreeze,
//            SIMCONNECT_GROUP_PRIORITY_HIGHEST,
//            SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY
//        );
//
//        // exit
//        //return 0;
//      }

      // sleep some time
      Sleep(10);
    }

  }

  SimConnect_Close(hSimConnect);
}
