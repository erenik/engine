/// Emil Hedemalm
/// 2014-11-14
/// A manager for handling HTTP and other internet-based communications via the cURL library.

#include "String/AEString.h"
#include "DataStream/DataStream.h"

typedef void CURLM;
typedef void CURL;

#define CURLMan (*CURLManager::Instance())

class cURLRequest 
{
public:
	cURLRequest();
	~cURLRequest();
	String url;
	CURL * curl;
	/// Input stream of data from the server the request was sent to.
	DataStream inStream;
	// Total received content.
	String content;
};

class CURLManager 
{
	CURLManager();
	~CURLManager();
	static CURLManager * curlManager;
public:
	static void Allocate();
	static void Deallocate();
	static CURLManager * Instance();
	void Process();
	/// Fetches a web-page. Default approach uses Curl, waiting until a request is received.
//	String HttpGet(String url);
	/** Asynchronous fetch of a web-page. 
		The contents of the page will be queued as a HttpGetMessage via the MessageManager, 
		and can thus be processed by the AppState when it has been successfully received.
	*/
	void HttpGetAsync(String url);
	/// o.o
	void HttpPostAsync(String url, String data);
	/// Sets up proxy for future Http requests.
	void SetProxy(String proxyUrl);
	/// For authentication to use the proxy.
	void SetProxyUsernamePassword(String username, String password);
	/// Getter
	cURLRequest * GetRequestByHandle(CURL * handle);

		/// o.o
	String proxyUrl, proxyUsername, proxyPassword;

	/// CURL Multi-interface.
	CURLM * curlm;
	List<cURLRequest *> asyncCurlRequests;

private:
	/// Cleans up and clears/removes all curl-handles.
	void ClearCurlAsyncs();
};


