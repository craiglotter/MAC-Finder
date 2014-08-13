
// This only works with Windows 98, Windows 2000/XP etc.  For 
// Windows 95, CoCreateGuid is an easy way to get the MAC, if
// you're not concerned about multiple adapters.
//

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>
#include <string.h>
#include <fstream.h>



///////////////////////////////////////////////////////////////////////////////
// You can either link directly to Iphlpapi.lib:
//
//   #include <Iphlpapi.h>
//   #pragma comment(lib, "Iphlpapi.lib")
//
// but instead we will dynamically load GetAdaptersInfo so
// the code will at least compile and load on Windows 95...
// So we have to copy some stuff declared in iptypes.h:

#define MAX_ADAPTER_DESCRIPTION_LENGTH  128 // arb.
#define MAX_ADAPTER_NAME_LENGTH         256 // arb.
#define MAX_ADAPTER_ADDRESS_LENGTH      8   // arb.

//
// IP_ADDRESS_STRING - store an IP address as a dotted decimal string
//

typedef struct {
    char String[4 * 4];
} IP_ADDRESS_STRING, *PIP_ADDRESS_STRING, IP_MASK_STRING, *PIP_MASK_STRING;

//
// IP_ADDR_STRING - store an IP address with its corresponding subnet mask,
// both as dotted decimal strings
//

typedef struct _IP_ADDR_STRING {
    struct _IP_ADDR_STRING* Next;
    IP_ADDRESS_STRING IpAddress;
    IP_MASK_STRING IpMask;
    DWORD Context;
} IP_ADDR_STRING, *PIP_ADDR_STRING;

//
// ADAPTER_INFO - per-adapter information. All IP addresses are stored as
// strings
//

typedef struct _IP_ADAPTER_INFO {
    struct _IP_ADAPTER_INFO* Next;
    DWORD ComboIndex;
    char AdapterName[MAX_ADAPTER_NAME_LENGTH + 4];
    char Description[MAX_ADAPTER_DESCRIPTION_LENGTH + 4];
    UINT AddressLength;
    BYTE Address[MAX_ADAPTER_ADDRESS_LENGTH];
    DWORD Index;
    UINT Type;
    UINT DhcpEnabled;
    PIP_ADDR_STRING CurrentIpAddress;
    IP_ADDR_STRING IpAddressList;
    IP_ADDR_STRING GatewayList;
    IP_ADDR_STRING DhcpServer;
    BOOL HaveWins;
    IP_ADDR_STRING PrimaryWinsServer;
    IP_ADDR_STRING SecondaryWinsServer;
    time_t LeaseObtained;
    time_t LeaseExpires;
} IP_ADAPTER_INFO, *PIP_ADAPTER_INFO;

typedef DWORD (WINAPI* GetAdaptersInfoFunc)(
  PIP_ADAPTER_INFO pAdapterInfo,  // buffer to receive data
  PULONG pOutBufLen               // size of data returned
);

//
// Function to convert retrieved info into a string
//

void MacAddressToString(const LPBYTE Address, LPSTR lpsz, int nAddressLength = 6)
{
	int n;
	LPSTR p = lpsz;
	for (n = 0; n < nAddressLength; n++)
	{
		p += sprintf(p, n ? "-%02x" : "%02x", Address[n]);
	}
}

//
// Crude Error Handler - simply prints message to string and terminates.
//

void error(const char* p, const char* p2 = "")
{
cout << p << " " << p2 << "\n";
exit(1);
}

////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) // command line args
{
    //declare variables necessary for adaptersinfo call
	HMODULE hLib;
	GetAdaptersInfoFunc GetAdaptersInfo = NULL;

	PIP_ADAPTER_INFO pai = NULL;
	DWORD dwSize = 0;
	DWORD dwRet;
	CHAR szMac[64];
	
	//declare variables necessary for the later string manipulation
	CHAR resultstring[64] = "";
    CHAR macfolder[] = "00000000.000";
    char sourcefolder[100];
    char destinationfolder[100];
    char filename[100];
	char* token;
	char seps[]   = "-";

	//control variables
	int firstflagsuccess = 0;

    cout << "\n" << endl;

	//checks that correct number of parameters are present
    if (argc != 4)
	{
		error("Incorrect number of parameters.\nUsage: MAC_Finder <source folder> <destination folder> <file to copy>\nNote: Folders must not have a '\\' character following them. (e.g. C:\\Windows)","");
	}

	//sets the inputted parameters to variables.
	strcpy(sourcefolder,argv[1]);
	strcpy(destinationfolder,argv[2]);
	strcpy(filename,argv[3]);

	//loads the IP helper api library
	hLib = LoadLibrary("Iphlpapi.dll");
	if (!hLib)
	{
		error("Failed to load Iphlpapi.dll","");
		//system ("pause");		
	}

	//Checks that the function exists within the library
	GetAdaptersInfo = (GetAdaptersInfoFunc)GetProcAddress(hLib, "GetAdaptersInfo");
	if (GetAdaptersInfo == NULL)
	{
		error("Failed to load GetAdaptersInfo in Iphlpapi.dll","");
	}

	// Get size of buffer needed:
	GetAdaptersInfo(NULL, &dwSize);

	pai = (PIP_ADAPTER_INFO)GlobalAlloc(GPTR, dwSize);

	//calls the adaptersinfo function
	dwRet = GetAdaptersInfo(pai, &dwSize);

	//checks that no error occurred
	if(dwRet != ERROR_BUFFER_OVERFLOW)
	{
	    if (GetLastError() != 0)
   	    {
   	            printf("GetAdapterInfo error:%d\n", GetLastError());
                exit(1);
        }
    }

    //retrieves the PC's MAC address
	PIP_ADAPTER_INFO p = pai;
	while (p)
	{
		MacAddressToString(p->Address, szMac, p->AddressLength);
		printf("Description: %s\nMAC Address: %s\n", p->Description, szMac);
		printf("\n");
		p = p->Next;
	}

	//Splits up the MAC address so that the string can be manipulated
	//e.g. 002244001145 -> 02244001.145
	token = strtok( szMac, seps );
	if (token != NULL)
	{
	    strncpy(resultstring, token + 1, 1 );
        firstflagsuccess = 0;
    }
    while( token != NULL )
    {
      if (firstflagsuccess == 1)
      	strcat(resultstring, token);      
  	  else
  	    firstflagsuccess = 1;
      token = strtok( NULL, seps );
    }

    strncpy(macfolder,resultstring,8);
    strncpy(macfolder+8,".",1);
    strncpy(macfolder + 9,resultstring + 8,3);

    //create the sourcefile and destinationfile strings
    strcat(sourcefolder,"\\");
    strcat(sourcefolder,macfolder);
    strcat(sourcefolder,"\\");
    strcat(sourcefolder,filename);
    strcat(destinationfolder,"\\");
    strcat(destinationfolder,filename);
    cout << "Copying \"" << sourcefolder << "\" to \"" << destinationfolder << "\""<< endl;

    //copy file
    ifstream from(sourcefolder) ; 
    if (!from) error("Cannot open input file",argv[1]) ;
    ofstream to(destinationfolder) ; 
    if (!to) error("Cannot open output file",argv[2]) ;
    char ch;
    while (from.get(ch)) to.put(ch) ;
    if (!from.eof() || !to) error("Error in copy process") ;

    cout << "Success\nExiting..." << endl;

    //free resources
	GlobalFree(pai);
	FreeLibrary(hLib);

	//exit program
	return 0; 
}


