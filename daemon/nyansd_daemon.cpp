/*
	nyansd_daemon.cpp - NyanSD server daemon implementation.
	
	2020/04/27, Maya Posch
*/


#include <nyansd.h>

#include <csignal>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <string>
#include <filesystem>	// C++17
#include <cstdlib>

namespace fs = std::filesystem;

#include "INIReader.h"


// Globals.
std::mutex gMutex;
std::condition_variable gCv;


void signal_handler(int signal) {
	gCv.notify_all();
}


#ifdef _WIN32
#include <windows.h>
#include <strsafe.h>

//#define SVCNAME TEXT("NyanSD")
const char* SVCNAME = { "NyanSD" };

SERVICE_STATUS          gSvcStatus; 
SERVICE_STATUS_HANDLE   gSvcStatusHandle; 
HANDLE                  ghSvcStopEvent = 0;

VOID WINAPI SvcCtrlHandler(DWORD);
VOID WINAPI SvcMain(DWORD, LPTSTR*);

VOID ReportSvcStatus(DWORD, DWORD, DWORD);
//VOID SvcInit(DWORD, LPTSTR*); 
VOID SvcReportEvent(LPTSTR);

void svcInit() {
#else
int main() {
#endif
	// Add the service records.
	// Get the list of files in the 'services' folder, parsing all 'service' files'
#ifdef _WIN32
	ghSvcStopEvent = CreateEvent(
							NULL,    // default security attributes
							TRUE,    // manual reset event
							FALSE,   // not signaled
							NULL);   // no name
							
	if (ghSvcStopEvent == NULL) {
        ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
        return;
    }
						 
	char* pdpath = getenv("ProgramData");
	if (pdpath == nullptr) {
		std::cerr << "Could not find 'ProgramData' environment variable." << std::endl;
		ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
		return;
	}
	
	std::string fullpath(pdpath);
	fullpath += "/NyanSD/services";
	fs::path path(fullpath);
#else
	fs::path path = "/etc/nyansd/services/";
#endif

	if (!fs::exists(path)) {
		std::cerr << "Services path " << path << " does not exist." << std::endl;
#ifdef _WIN32
		ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
		return;
#else
		return 1;
#endif
	}
	
	for (fs::directory_entry const &entry : fs::directory_iterator(path)) {
		if (entry.is_regular_file() && entry.path().extension() == ".service") {
			INIReader serviceFile(entry.path().string());	// Read in service file.
			if (serviceFile.ParseError() != 0) {
				std::cerr << "Failed to parse file '" << path << "'." << std::endl;
#ifdef _WIN32
				ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
				return;
#else
				return 1;
#endif
			}
			
			std::set<std::string> sections = serviceFile.Sections();
				
			std::set<std::string>::const_iterator it;
			for (it = sections.cbegin(); it != sections.cend(); ++it) {
				NYSD_service sv;
				sv.service = serviceFile.Get(*it, "name", "");
				if (sv.service.empty()) {
					std::cerr << "Service name was empty. Skipping..." << std::endl;
					continue;
				}
				
				sv.port = serviceFile.GetInteger(*it, "port", 0);
				
				std::string protocol = serviceFile.Get(*it, "protocol", "");
				if (protocol == "tcp") {
					sv.protocol = NYSD_PROTOCOL_TCP;
				}
				else if (protocol == "udp") {
					sv.protocol = NYSD_PROTOCOL_UDP;
				}
				else if (protocol == "all") {
					sv.protocol = NYSD_PROTOCOL_ALL;
				}
				else {
					// Default is to ignore this service.
					std::cerr << "Invalid protocol for service " << sv.service << ". Ignoring..." << std::endl;
					continue;
				}
				
				sv.hostname = serviceFile.Get(*it, "host", "");
				std::string ipv4 = serviceFile.Get(*it, "ipv4", "");
				if (ipv4.empty()) { sv.ipv4 = 0; }
				else { sv.ipv4 = NyanSD::ipv4_stringToUint(ipv4); }
				
				sv.ipv6 = serviceFile.Get(*it, "ipv6", "");
				
				std::cout << "Adding service: " << sv.service << std::endl;
				
				if (!NyanSD::addService(sv)) { 
					std::cerr << "Failed to add service. Aborting..." << std::endl;
#ifdef _WIN32
					ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
					return;
#else
					return 1;
#endif
				}
			}
		}
	}
	
	std::cout << "Added services." << std::endl;
	
	// Set signal handler to allow ending the server with 
	std::signal(SIGINT, signal_handler);
	
	// Start the server.
	std::cout << "Starting server..." << std::endl;
	NyanSD::startListener(11310);
	
#ifdef _WIN32
	ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);
#endif
	
	// Wait for signal.
	std::cout << "Running... terminate with SIGINT (Ctrl+c)." << std::endl;
	std::unique_lock<std::mutex> lk(gMutex);
	gCv.wait(lk);
	
	// Stop the server.
	std::cout << "Stopping server..." << std::endl;
	NyanSD::stopListener();
	
	std::cout << "All done." << std::endl;
	
#ifdef _WIN32
	return;
#else
	return 0;
#endif
}


#ifdef _WIN32
//
// Purpose: 
//   Entry point for the process
//
// Parameters:
//   None
// 
// Return value:
//   None, defaults to 0 (zero)
//
int __cdecl main(int argc, TCHAR *argv[])  { 
    // If command-line parameter is "install", install the service. 
    // Otherwise, the service is probably being started by the SCM.
    /* if (lstrcmpi( argv[1], TEXT("install")) == 0) {
        SvcInstall();
        return;
    } */

    // TO_DO: Add any additional services for the process to this table.
    SERVICE_TABLE_ENTRY DispatchTable[] =  { 
        { (LPTSTR) SVCNAME, (LPSERVICE_MAIN_FUNCTION) SvcMain }, 
        { NULL, NULL } 
    }; 
 
    // This call returns when the service has stopped. 
    // The process should simply terminate when the call returns.
    if (!StartServiceCtrlDispatcher(DispatchTable)) { 
		const char* ssvc = { "StartServiceCtrlDispatcher" };
        SvcReportEvent((LPTSTR) ssvc);
    } 
	
	return 0;
} 


//
// Purpose: 
//   Entry point for the service
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None.
//
void WINAPI SvcMain( DWORD dwArgc, LPTSTR* lpszArgv) {
    // Register the handler function for the service

    gSvcStatusHandle = RegisterServiceCtrlHandler(SVCNAME, SvcCtrlHandler);

    if (!gSvcStatusHandle) { 
		const char* rsvc = { "RegisterServiceCtrlHandler" };
        SvcReportEvent((LPTSTR) rsvc); 
        return; 
    } 

    // These SERVICE_STATUS members remain as set here
    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
    gSvcStatus.dwServiceSpecificExitCode = 0;    

    // Report initial status to the SCM
    ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    // Perform service-specific initialization and work.
    //SvcInit( dwArgc, lpszArgv );
	svcInit();
	
	ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
}


//
// Purpose: 
//   Called by SCM whenever a control code is sent to the service
//   using the ControlService function.
//
// Parameters:
//   dwCtrl - control code
// 
// Return value:
//   None
//
void WINAPI SvcCtrlHandler(DWORD dwCtrl) {
   // Handle the requested control code. 

   switch(dwCtrl) {  
      case SERVICE_CONTROL_STOP: 
         ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

         // Signal the service to stop.
         //SetEvent(ghSvcStopEvent);
		 gCv.notify_all();
         ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);
         
         return;
 
      case SERVICE_CONTROL_INTERROGATE: 
         break; 
 
      default: 
         break;
   }
}


//
// Purpose: 
//   Sets the current service status and reports it to the SCM.
//
// Parameters:
//   dwCurrentState - The current state (see SERVICE_STATUS)
//   dwWin32ExitCode - The system error code
//   dwWaitHint - Estimated time for pending operation, 
//     in milliseconds
// 
// Return value:
//   None
//
VOID ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint) {
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.
    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING) { gSvcStatus.dwControlsAccepted = 0; }
    else { gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP; }

    if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED)) {
        gSvcStatus.dwCheckPoint = 0;
	}
    else {
		gSvcStatus.dwCheckPoint = dwCheckPoint++;
	}

    // Report the status of the service to the SCM.
    SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}


#define SVC_ERROR                        ((DWORD)0xC0020001L)


//
// Purpose: 
//   Logs messages to the event log
//
// Parameters:
//   szFunction - name of function that failed
// 
// Return value:
//   None
//
// Remarks:
//   The service must have an entry in the Application event log.
//
VOID SvcReportEvent(LPTSTR szFunction) { 
    HANDLE hEventSource;
    LPCTSTR lpszStrings[2];
    TCHAR Buffer[80];

    hEventSource = RegisterEventSource(NULL, SVCNAME);

    if (NULL != hEventSource) {
        StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

        lpszStrings[0] = SVCNAME;
        lpszStrings[1] = Buffer;

        ReportEvent(hEventSource,        // event log handle
                    EVENTLOG_ERROR_TYPE, // event type
                    0,                   // event category
                    SVC_ERROR,           // event identifier
                    NULL,                // no security identifier
                    2,                   // size of lpszStrings array
                    0,                   // no binary data
                    lpszStrings,         // array of strings
                    NULL);               // no binary data

        DeregisterEventSource(hEventSource);
    }
}

#endif
