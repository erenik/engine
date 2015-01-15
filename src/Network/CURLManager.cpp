/// Emil Hedemalm
/// 2014-11-14
/// A manager for handling HTTP and other internet-based communications via the cURL library.

#include "Libs.h"

#include "CURLManager.h"

#include <curl/curl.h>
#include "DataStream/DataStream.h"

#include "Message/MessageManager.h"
#include "Message/Message.h"

cURLRequest::cURLRequest()
{

}
cURLRequest::~cURLRequest()
{
#ifdef LIBCURL
	curl_easy_cleanup(curl);
#endif
}
	

CURLManager::CURLManager()
{
	curlm = NULL;
}

CURLManager::~CURLManager()
{
#ifdef LIBCURL
	asyncCurlRequests.ClearAndDelete();
	// Cleanup curlm.
	curl_multi_cleanup(curlm);
#endif // LIBCURL
}

CURLManager * CURLManager::curlManager = NULL;

void CURLManager::Allocate()
{
	assert(curlManager == NULL);
	curlManager = new CURLManager();
}

void CURLManager::Deallocate()
{
	assert(curlManager);
	delete curlManager;
}

CURLManager * CURLManager::Instance()
{
	return curlManager;
}


void CURLManager::Process()
{
#ifdef LIBCURL
	// Process CURL if initialized.
	if (curlm && asyncCurlRequests.Size())
	{
		int activeHandles;
		curl_multi_perform(curlm, &activeHandles);
	//	std::cout<<"\nActive handles: "<<activeHandles;
	}
	// Process messages.
	CURLMsg * msg;
	int msgs_in_queue;
	while (msg = curl_multi_info_read(curlm, &msgs_in_queue))
	{
		// Process it?
		cURLRequest * request = GetRequestByHandle(msg->easy_handle);
		switch(msg->msg)
		{
			case CURLMSG_DONE:
			{
				// Create message with the contents and send it away.
				CURLMessage * curlMessage = new CURLMessage("HttpGet");
				request->inStream.GetDataAsString(curlMessage->contents);
				curlMessage->url = request->url;
				MesMan.QueueMessage(curlMessage);
				// Remove it?
//				CURLMcode result = curl_multi_remove_handle(curlm, msg->easy_handle);
	//			asyncCurlRequests.Remove(request);
	//			delete request;
				break;
			}
			default:
				assert(false);
		}
	}
#endif // LIBCURL
}



DataStream httpGetStream;

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userData)
{
	int bytes = size * nmemb;

	// Find handle corresponding to the user-data?
	for (int i = 0; i < CURLMan.asyncCurlRequests.Size(); ++i)
	{
		CURL * curl = userData;
		cURLRequest * request = CURLMan.GetRequestByHandle(curl);
		if (request)
		{
			// power o.o
			request->inStream.PushBytes((uchar*)ptr, bytes);
//			std::cout<<"\nReceived bytes: "<<bytes;
			return bytes;
		}
	}
	std::cout<<"\nCouldn't find appropriate request. D:";
	assert(false);
	return bytes;
}

/*
/// Fetches a web-page. Default approach uses Curl, waiting until a request is received.
String CURLManager::HttpGet(String url)
{	
	// Do a curl-get o.o
	CURL *curl;
	CURLcode res;
	String contents;

	/// o.o 
	httpGetStream.PopAll();

	curl = curl_easy_init();
	if(curl) 
	{
		std::cout<<"\nSending HTTP request to "<<url;
		/// Setup proxy o.o
		if (proxyUrl.Length())
			curl_easy_setopt(curl, CURLOPT_PROXY, proxyUrl.c_str());
		// Set password if needed too.
		if (proxyUsername.Length())
			curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, (proxyUsername+":"+proxyPassword).c_str());
		/// o.o
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		
		//* example.com is redirected, so we tell libcurl to follow redirection 
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		// Set call-back method for.. stuff.
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		// Set call-back specific data for this curl-element. Use the same curl-element?
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, curl);

		// Perform the request, res will get the return code
		res = curl_easy_perform(curl);

		// Check for errors 
		if(res != CURLE_OK)
		{
			fprintf(stderr, "\ncurl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			String error = curl_easy_strerror(res);
			std::cout<<"\nErroror o.o "<<error;
		}
		else 
		{
			// OK o.o
			uint32 receivedBytes = 0;
			httpGetStream.GetDataAsString(contents);
		//	std::cout<<"\n buf o.o :\n"<<contents;
		}
		// always cleanup 
		curl_easy_cleanup(curl);
	}
	return contents;
}
*/

/** Asynchronous fetch of a web-page. 
	The contents of the page will be queued as a HttpGetMessage via the MessageManager, 
	and can thus be processed by the AppState when it has been successfully received.
*/
void CURLManager::HttpGetAsync(String url)
{
#ifdef LIBCURL
	// Setup curlm as needed.
	if (curlm == NULL)
		curlm = curl_multi_init();

	// Do a curl-get o.o
	CURL *curl;
	CURLcode res;
	String contents;

	/// o.o 
	httpGetStream.PopAll();

	curl = curl_easy_init();
	if(curl) 
	{
		int error = 0;
		std::cout<<"\nSending HTTP request to "<<url;
		/// Setup proxy o.o
		if (proxyUrl.Length())
		{
			error = curl_easy_setopt(curl, CURLOPT_PROXY, proxyUrl.c_str());
			assert(error == CURLE_OK);
		}
		// Set password if needed too.
		if (proxyUsername.Length())
		{
			error = curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, (proxyUsername+":"+proxyPassword).c_str());
			assert(error == CURLE_OK);
		}
		/// o.o
		error = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		assert(error == CURLE_OK);
		
		/* example.com is redirected, so we tell libcurl to follow redirection */
		error = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		// Set call-back method for.. stuff.
		error = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		assert(error == CURLE_OK);
		// Set call-back specific data for this curl-element. Use the same curl-element?
		error = curl_easy_setopt(curl, CURLOPT_WRITEDATA, curl);
		assert(error == CURLE_OK);
		
		curl_multi_add_handle(curlm, curl);

		cURLRequest * request = new cURLRequest();
		request->curl = curl;
		request->url = url;
		/// Add to list of active async handles.
		asyncCurlRequests.Add(request);
	}
#endif // LIBCURL
}

/** Asynchronous fetch of a web-page. 
	The contents of the page will be queued as a HttpGetMessage via the MessageManager, 
	and can thus be processed by the AppState when it has been successfully received.
*/
void CURLManager::HttpPostAsync(String url, String data)
{
#ifdef LIBCURL
	// Setup curlm as needed.
	if (curlm == NULL)
		curlm = curl_multi_init();

	// Do a curl-get o.o
	CURL *curl;
	CURLcode res;
	String contents;

	/// o.o 
	httpGetStream.PopAll();

	curl = curl_easy_init();
	if(curl) 
	{
		std::cout<<"\nSending HTTP request to "<<url;
		/// Setup proxy o.o
		if (proxyUrl.Length())
			curl_easy_setopt(curl, CURLOPT_PROXY, proxyUrl.c_str());
		// Set password if needed too.
		if (proxyUsername.Length())
			curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, (proxyUsername+":"+proxyPassword).c_str());


//		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.Length());
		// Set post-data o.o
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
		
		/// o.o
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		
		/* example.com is redirected, so we tell libcurl to follow redirection */
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		// Set call-back method for.. stuff.
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		// Set call-back specific data for this curl-element. Use the same curl-element?
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, curl);


		curl_multi_add_handle(curlm, curl);

		cURLRequest * request = new cURLRequest();
		request->curl = curl;
		request->url = url;
		/// Add to list of active async handles.
		asyncCurlRequests.Add(request);
	}
#endif // LIBCURL
}


/// Sets up proxy for future Http requests.
void CURLManager::SetProxy(String newProxyUrl)
{
	proxyUrl = newProxyUrl;
}

/// For authentication to use the proxy.
void CURLManager::SetProxyUsernamePassword(String username, String password)
{
	proxyUsername = username;
	proxyPassword = password;
}

cURLRequest * CURLManager::GetRequestByHandle(CURL * handle)
{
	for (int i = 0; i < asyncCurlRequests.Size(); ++i)
	{
		cURLRequest * req = asyncCurlRequests[i];
		if (req->curl == handle)
			return req;
	}
	return NULL;
}
