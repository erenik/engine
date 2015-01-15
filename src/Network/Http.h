/// Emil Hedemalm
/// 2014-12-08
/// HTTP configuration stuffs

#ifndef ENGINE_HTTP_H
#define ENGINE_HTTP_H

#include "String/AEString.h"

class TcpSocket;

/// 0 for engine/own, 1 for cURL, default 1?
extern int httpTool;
extern bool printHttpOutput;

/// Fetches contents of target url. Results will be sent back to self via HttpMessage/CURLMessage
void HttpGet(String url);


namespace Http {
	bool GetHostNameAndPage(String fromURL, String & hostname, String & page);

enum responseCodes {
	NO_RESPONSE = 0,
	UNABLE_TO_CONNECT_TO_SERVER,
	OK = 200,
	FOUND = 302, // Found it, but it resides in another location.
	BAD_REQUEST = 400,
	NOT_FOUND = 404,
	INTERNAL_SERVER_ERROR = 500,
};};


class HttpRequest 
{
public:
	enum {
		NONE,
		CONNECTING,
		REDIRECTING,
		FAILED,
		DONE,
	};

	HttpRequest(String url);
	~HttpRequest();
	bool ConnectToServer();
	void SendGetRequest();
	/// For requests which take some time to deliver all info.
	void Process();

	/// See Http above.
	int responseCode;
	String url;
	String redirectURL;
	/// Entire response.
	String response;
	TcpSocket * socket;
	int status;

private:

	void SendGetRequest(String toURL);

	String urlWithHttp;
	String hostname;
	String targetPage;
};

#define HttpMan (*HttpManager::Instance())

class HttpManager 
{
	HttpManager();
	~HttpManager();
	static HttpManager * httpManager;
public:
	static void Allocate();
	static void Deallocate();
	static HttpManager * Instance();

	/// Posts a Http Get request.
	void Get(String url);
	/// Processes all requests.
	void Process();

	List<HttpRequest*> httpRequests;
};


#endif

