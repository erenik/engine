/// Emil Hedemalm
/// 2014-12-08
/// HTTP configuration stuffs

#include "Http.h"

#include "Network/CURLManager.h"
#include "Network/NetworkManager.h"

#include "Network/Socket/TcpSocket.h"

#include "Message/Message.h"
#include "Message/MessageManager.h"

#include "String/StringUtil.h"

/// 0 for engine/own, 1 for cURL, default 1?
int httpTool = 1;
bool printHttpOutput = true;

HttpManager * HttpManager::httpManager = NULL;

/// Fetches contents of target url. Results will be sent back to self via HttpMessage/CURLMessage
void HttpGet(String url)
{
	if (httpTool == 0)
	{
		HttpMan.Get(url);
	}
	else 
	{
		CURLMan.HttpGetAsync(url);
	}
}


HttpRequest::HttpRequest(String url) 
	: url(url)
{ 
	status = CONNECTING;
	socket = NULL;
	responseCode = -1;
};

HttpRequest::~HttpRequest()
{ 
	if (socket) 
		delete socket;
}


bool HttpRequest::ConnectToServer()
{
	Http::GetHostNameAndPage(url, hostname, targetPage);
	socket = new TcpSocket();
	bool connected = socket->ConnectTo(hostname, 80);
	if (!connected)
	{
		std::cout<<"\nUnable to connect to target server";
		status = HttpRequest::FAILED;
		return false;
	}
	return true;
}

void HttpRequest::SendGetRequest()
{
	SendGetRequest(url);
}

namespace Http 
{
	bool GetHostNameAndPage(String url, String & hostname, String & page)
	{
		url.Remove("http://");

		List<String> tokens = url.Tokenize("/");
		String urlMinusHostname = url;
		if (url.Contains("http://"))
		{
			hostname = url;
			hostname = hostname.Tokenize("/")[0];
		}
		else 
		{
			hostname = url;
			hostname = hostname.Tokenize("/")[0];
		}
		urlMinusHostname.Remove(hostname);
	
		page = "/";
		if (urlMinusHostname.Length())
			page = urlMinusHostname;
		return true;
	}
};

void HttpRequest::SendGetRequest(String toURL)
{
	url = toURL;
	Http::GetHostNameAndPage(url, hostname, targetPage);
	String requestString = "GET "+targetPage+" HTTP/1.1\r\nHost: "+hostname+"\r\n\r\n  \0\0\0";
	std::cout<<"\nSending request: \n"<<requestString;
	/// Send request.
	int written = socket->Write(requestString.c_str(), requestString.Length() - 2);
	std::cout<<"\nBytes written: "<<written;
}


/// For requests which take some time to deliver all info.
void HttpRequest::Process()
{
	char buffer[5000];
	memset(buffer, 0, 5000);
	int bytesRead = 1; 
	bytesRead = socket->Read(buffer, 4999);
	while(bytesRead > 0)
	{
		if (printHttpOutput)
			std::cout<<"\nBuffer:\n"<<buffer<<"\nbytes: "<<bytesRead;
		/*
		for (int i = 0; i < bytesRead; ++i)
		{
			char c = buffer[i];
			if (c < 0)
				std::cout<<"\nUnicode character incoming?";
			std::cout<<"\nc "<<i<<": "<<(int)c;
		}*/
		/// o.o
		String str = buffer;
		response += str;
		if (printHttpOutput)
			std::cout<<"\nResponse so far: \n"<<response;
		std::cout<<"\nReceived "<<str.Length()<<" bytes from "<<url;
		memset(buffer, 0, 5000);
		bytesRead = socket->Read(buffer, 4999);
	}
	// Check if the connection closed?
	if (bytesRead < 0)
	{
		// Error if bytes read is negative, ye?
		socket->Close();
		status = HttpRequest::DONE;
		std::cout<<"\nPort was closed unexpectedly D:";
	}

	/// Check if the response has ended (double /r/n)
	/// Just assume ended if we got 0 bytes read, yo?
	if (bytesRead == 0 && response.Length())
		status = HttpRequest::DONE;

	/// Inform of reply, if we got any.
	if (status == HttpRequest::DONE ||
		status == HttpRequest::FAILED)
	{
		if (!response.Length())
		{
			std::cout<<"\nNo decent reply from server.";
		}
		else
		{
			if (printHttpOutput)
				std::cout<<"\nResponse:\n"<<response;

			// Extract contents?
			List<String> lines = response.GetLines();
			List<String> bodyLines;
			float httpVersion;
			responseCode = -1;
			int length = 0;
			/// For re-direction responses.
			String location;
			// For chunked transfer encoding
			bool chunked = false;
			enum {
				HEADER,
				BODY,
				ENDING
			};
			int state = 0;
			for (int i = 0; i < lines.Size(); ++i)
			{
				String line = lines[i];
				if (line.Contains("HTTP/"))
				{
					httpVersion = line.Tokenize(" /")[1].ParseFloat();
					responseCode = line.Tokenize(" ")[1].ParseInt();
				}
				if (line.Contains("Date:"))
				{
					// Do stuff?
				}
				if (line.Contains("Server:"))
				{
				}
				if (line.Contains("Location:"))
				{
					location = line;
					location.Remove("Location:");
					location.RemoveInitialWhitespaces();
				}
				if (line.Contains("Vary:"))
				{
					// What is vary..? accept-language, accept-charset?
				}
				if (line.Contains("Accept-Ranges:"))
				{
					// bytes..?
				}
				if (line.Contains("Transfer-Encoding:"))
				{
					if (line.Contains("chunked"))
						chunked = true;
				}
				if (line.Contains("Content-Type:"))
				{
					// text/html; charset=utf-8
				}
				if (line.Contains("Content-Language: en"))
				{
					// en
				}
				if (line.Contains("Content-Length:"))
				{
					length = line.Tokenize(":")[1].ParseInt();
				}
				/// End after all header data has been parsed?
				if (line == "")
				{
					if (chunked)
						break;
					state++;
					if (state == ENDING)
						std::cout<<"\n<Message ending>";
				}
				if (state == HEADER && printHttpOutput)
					std::cout<<"\nHeader Line "<<i<<": "<<lines[i];
			}

			// Fetch the body?
			String body;
			if (chunked)
			{
				// Re-parse the message, taking into account chunking.
				int headerEnd = response.Find("\r\n\r\n");
				String chunkSizeStr = "0x";
				for (int i = headerEnd + 4; i < response.Length(); ++i)
				{
					// Find next header.
					char c = response.c_str()[i];
					if (c == '\r')
					{
						// Move i 2 steps forward to skip the \r\n
						i += 2;
						// Evaluate chunk-size.
						int chunkSize = chunkSizeStr.ParseHex();
						if (chunkSize == 0)
							break;
						String part = response.Part(i, i + chunkSize);
						if (printHttpOutput)
							std::cout<<"\nPart: "<<part;
//						part.PrintData();
						body += part;
						chunkSizeStr = "0x";
						// Move forward the chunk + the ending \r\n (skip 1 though, as i will be incremented in the for-loop!)
						i += chunkSize + 1;
					}
					else 
						chunkSizeStr += c;
				}
			}
			else if (length > 0)
			{
				// Re-parse the message, taking into account message body length.
				int headerEnd = response.Find("\r\n\r\n");
				int bodyStart = headerEnd + 4;
				body = response.Part(bodyStart, bodyStart + length);
			}			
			if (printHttpOutput)
				std::cout<<"\nEntire body: "<<body<<"\nLength: "<<body.Length();
			
			/// Check response, for re-directions.
			switch(responseCode)
			{
				// Found it, but it resides elsewhere o.o
				case Http::FOUND:
				{
					/// Post a new request with the re-directed url?
					redirectURL = location;
					this->SendGetRequest(redirectURL);
					status = HttpRequest::REDIRECTING;
					// Delete previously recorded responses?
					response = "";
					body = "";
					return;
				}
			};
			
			HttpMessage * httpMessage = new HttpMessage("HttpGet", responseCode);
			httpMessage->contents = body;
			httpMessage->url = url;
			MesMan.QueueMessage(httpMessage);
		}
	}
}

HttpManager::HttpManager()
{
}
HttpManager::~HttpManager()
{

}

void HttpManager::Allocate()
{
	assert(httpManager == NULL);
	httpManager = new HttpManager();
}
void HttpManager::Deallocate()
{
	assert(httpManager);delete httpManager; httpManager = NULL;
}

HttpManager * HttpManager::Instance()
{
	return httpManager;
}

/// HTTP stuffs.
void HttpManager::Get(String url)
{
	HttpRequest * req = new HttpRequest(url);
	bool connected = req->ConnectToServer();
	if (!connected)
	{
		HttpMessage * msg = new HttpMessage("HttpGet", Http::UNABLE_TO_CONNECT_TO_SERVER);
		msg->url = url;
		MesMan.QueueMessage(msg);
		httpRequests.Remove(req);
		SAFE_DELETE(req);
		return;
	}
	req->SendGetRequest();
	httpRequests.Add(req);
}

void HttpManager::Process()
{
	for (int i = 0; i < httpRequests.Size(); ++i)
	{
		HttpRequest * req = httpRequests[i];
		req->Process();
		if (req->status == HttpRequest::DONE ||
			req->status == HttpRequest::FAILED)
		{
			httpRequests.Remove(req);
			delete req;
			--i;
		}
	}
}
