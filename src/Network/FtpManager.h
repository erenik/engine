// Aron Wåhlberg
// 2013-07-04

#include "Network/NetworkSettings.h"

// If not using network, uncomment all rows using the network-manager... if possible.
#if ! defined(USE_NETWORK) || ! defined(USE_FTP)
#define Ftp		NULL //Network disabled.
#endif

// IF using network, do awesome stuff! o-o
#if defined(USE_NETWORK) && defined(USE_FTP)
#define Ftp		(*FtpManager::Instance())

#ifndef FTPMANAGER_H
#define FTPMANAGER_H

/************************************************************************/
/* Includes                                                             */
/************************************************************************/
#include "OS/OS.h"
#if PLATFORM == PLATFORM_WINDOWS
	// Only for Windows
	#include <WS2tcpip.h>
	#include <WinSock2.h>	// For sockets in Windows
	#include <Windows.h>	// Windows stuff
	#include <process.h>	// For multi threading
	#pragma comment(lib, "Ws2_32.lib")
	#ifdef _MSC_VER
		#define sockerrno WSAGetLastError()
	#else
		#include <ws2tcpip.h>	// More windows socket
		#define sockerrno errno
	#endif
#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
	// Only for MAC and Unix
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#if PLATFORM == PLATFORM_MAC
		#define SOL_IP IPPROTO_IP
	#endif
	#ifndef INVALID_SOCKET
		#define INVALID_SOCKET -1
	#endif
	#define sockerrno errno
#endif
#include "Globals.h"
#include <iostream>
#include <string>
#include <errno.h>
#include "Util/Queue/Queue.h"
#include "Util/Timer/Timer.h"
#include "Network/Packet.h"
// Must pre-define the packets we're using, evil includception
struct PacketMakeFtpRequest;
struct PacketAnswerFtpRequest;
struct PacketFtpData;
struct PacketFtpDataReply;
struct PacketFtpFinished;

/** FtpFile
	Information about a file that is to be sent through the ftp request */
struct FtpFile
{
	char filename[MAX_FILENAME_LENGTH];
	unsigned int filesize;
	int packets;
};

/** FtpRequest
	A FTP request to be made, and information about it */
struct FtpRequest
{
	FtpRequest(int id);
	int id;							// Id for this specific request
	FtpFile *files[MAX_FTP_FILES];	// Files to be sent
};

/** FtpTransfer
	Keeps track of what files to send and to whom */
struct FtpTransfer
{
	FtpTransfer(FtpRequest *request, int target);
	FtpRequest *request;
	int statusSize;						// Length of status array
	int *status;						// Sent - received
	int currentFile;					// Current file we're sending
	int currentPart;					// Current part we're sending
	int statusIndex;					// Index we're at in the status
	long long *timeSent;				// Time when each part was sent
	int target;							// Target of the FTP transfer
};

/**
	*/
struct FtpReceive
{
	FtpReceive(int slotID, int ftpID, int files, char filenames[MAX_FTP_FILES][MAX_FILENAME_LENGTH], unsigned int filesizes[MAX_FTP_FILES], int parts[MAX_FTP_FILES]);
	int id;												// This receive id, not the same as ftpID
	int ftpID;											// Id of the FTP request to receive from
	int files;											// Amount of files that are to be received
	char filenames[MAX_FTP_FILES][MAX_FILENAME_LENGTH];	// Name of the files, with extension and all
	unsigned int filesizes[MAX_FTP_FILES];				// Size of the files in bytes
	int parts[MAX_FTP_FILES];							// Amount of packets each file is split into
	int *status;										// received - complete
};

/**	FtpManager
	Handles the sending of files over the network! */
class FtpManager{
private:
	FtpManager();
	static FtpManager * ftp;
public:
	static void Allocate();
	static FtpManager * Instance();
	static void Deallocate();
	~FtpManager();
	/// Initialized
	void Initialize();
	/// Clear all requests/transfers/receives
	void ClearFtp();
	/// Clear all requests/transfers/receives associated with a client
	void ClearClientFromFtp(int clientIndex);

	/// Get free ftp request slot if any
	int GetFreeFtpRequestSlot();
	/// Get Ftp request in selected slot
	FtpRequest * GetFtpRequest(int slot);
	/// Get Ftp request by id
	FtpRequest * GetFtpRequestById(int id);
	/// Create FTP request
	FtpRequest * CreateFtpRequest();
	/// Tries to add a file to a FTP request
	bool AddFileToFtpRequest(FtpRequest *request, const char *filename);
	// Sends out the ftp request
	bool MakeFtpRequest(FtpRequest *request, int target);

	/// Get free transfer slot if any
	int GetFreeTransferSlot();
	/// Get Ftp transfer in selected slot
	FtpTransfer * GetFtpTransfer(int slot);
	/// Get Ftp transfers slot
	int GetFtpTransferSlot(FtpTransfer *transfer);
	/// Get Ftp transfer with selected ftp id
	FtpTransfer * GetFtpTransferFromFtpID(int ftpID, int target);
	/// Create a transfer structure
	bool CreateFtpTransfer( FtpRequest * request, int target );
	/// Deletes a transfer
	void DeleteTransfer( FtpTransfer *transfer );
	/// Deletes a transfer
	void DeleteTransfer( int slot );

	/// Get free receive slot if any
	int GetFreeReveiveSlot();
	/// Get Ftp receive in selected slot
	FtpReceive * GetFtpReceive(int slot);
	/// Get Ftp receive slot
	int GetReceiveSlot( FtpReceive *receive );
	/// Get Ftp receive with selected ftp id
	FtpReceive * GetFtpReceiveFromFtpID(int slot);
	/// Create a receive structure
	bool CreateFtpReceive( int ftpID, int files, char filenames[MAX_FTP_FILES][MAX_FILENAME_LENGTH], unsigned int filesizes[MAX_FTP_FILES], int parts[MAX_FTP_FILES] );
	/// Deletes a receive
	void DeleteReceive( FtpReceive *receive );
	/// Deletes a receive
	void DeleteReceive( int slot );

	/// Receives a FTP request, checks content and supplies answer
	void ReceiveFtpRequest(PacketMakeFtpRequest *pkt);
	/// Checks whether a client accepted or declined a FTP request
	void CheckFtpRequest(PacketAnswerFtpRequest *pkt);
	/// Receive a data part of a file from the FTP request
	void ReceiveFtpData(PacketFtpData *pkt);
	/// Receives confirmation from FTP request target that a data part is received
	void ReceiveFtpDataPartConfirmation(PacketFtpDataReply *pkt);
	/// Receptionist of a FTP request gets told that the transfer is complete
	void FinishFtpRequest(PacketFtpFinished *pkt);

	/// Check if we need/can send new Ftp data
	void SendFtp();

	/// Boolean if manager should clear any data in the beginning of next SendFtp
	bool clearData;
	/// Clear the whole manager
	bool clearAll;
	/// Clear a specific client
	bool clearClients[MAX_CLIENTS];
private:
	bool initialized;
	/// Time between ftp sends
	long long ftpTime;
	/// Keeps track of all the active ftp requests
	FtpRequest *ftpRequests[MAX_FTP_REQUESTS];
	/// Keeps track of all the active ftp transfers
	FtpTransfer *ftpTransfers[MAX_FTP_TRANSFERS];
	/// Keeps track of all the active ftp receives
	FtpReceive *ftpReceives[MAX_FTP_REQUESTS];

	// Do the clearing of the ftp manager
	void DoClearAll();
	// Do the clearing of a client from the ftp
	void DoClearClient(int clientIndex);
};

#endif

#endif // USE_NETWORK
