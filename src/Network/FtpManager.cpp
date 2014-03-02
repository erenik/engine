// Aron Wåhlberg
// 2013-07-03

#include "Network/NetworkManager.h"
#include <cstring>
#include "NetworkClient.h"

#define print(arg) std::cout << arg << "\n";

#if defined(USE_NETWORK) && defined(USE_FTP)

// ------------------- Managers -------------------
#include "FtpManager.h"
#include "NetworkManager.h"
#include "Message/MessageManager.h"
FtpManager * FtpManager::ftp = NULL;			// Global Manager

// ------------------- Includes -------------------
#include <iostream>
#include <fstream>
#include "String/AEString.h"
#include "Util/Timer/Timer.h"
#include "Network/Packet.h"
#include "Network/Packet/Packets.h"
extern int packet_size[PACKET_TYPES];

// ------------------- Functions -------------------
void FtpManager::Allocate(){
	assert(ftp == NULL);
	ftp = new FtpManager();
}
FtpManager * FtpManager::Instance(){
	assert(ftp);
	return ftp;
}
void FtpManager::Deallocate(){
	assert(ftp);
	delete(ftp);
	ftp = NULL;
}

FtpManager::FtpManager(){
	for(int i = 0; i < MAX_FTP_REQUESTS; ++i){
		ftpRequests[i] = NULL;
		ftpReceives[i] = NULL;
	}
	for(int i = 0; i < MAX_FTP_TRANSFERS; ++i)
		ftpTransfers[i] = NULL;
	ftpTime = 0;
	clearData = false;
	clearAll = false;
	for(int i = 0; i < MAX_CLIENTS; ++i)
		clearClients[i] = false;
}

FtpManager::~FtpManager(){
	for(int i = 0; i < MAX_FTP_TRANSFERS; ++i){
		DeleteTransfer(ftpTransfers[i]);
	}
	for(int i = 0; i < MAX_FTP_REQUESTS; ++i){
		if(ftpRequests[i]){
			for(int j = 0; j < MAX_FTP_FILES; ++j){
				if(ftpRequests[i]->files[j]){
					delete ftpRequests[i]->files[j];
					ftpRequests[i]->files[j] = NULL;
				}
			}
			delete ftpRequests[i];
			ftpRequests[i] = NULL;
		}
		if(ftpReceives[i]){
			delete ftpReceives[i];
			ftpReceives[i] = NULL;
		}
	}
	initialized = false;
}

void FtpManager::Initialize(){
	initialized = true;
	print("\nFtpManager Initialized.");
}
void FtpManager::ClearFtp()
{
	clearData = true;
	clearAll = true;
}
void FtpManager::DoClearAll()
{
	for(int i = 0; i < MAX_FTP_TRANSFERS; ++i){
		DeleteTransfer(ftpTransfers[i]);
	}
	for(int i = 0; i < MAX_FTP_REQUESTS; ++i){
		if(ftpRequests[i]){
			for(int j = 0; j < MAX_FTP_FILES; ++j){
				if(ftpRequests[i]->files[j]){
					delete ftpRequests[i]->files[j];
					ftpRequests[i]->files[j] = NULL;
				}
			}
			delete ftpRequests[i];
			ftpRequests[i] = NULL;
		}
		if(ftpReceives[i]){
			delete ftpReceives[i];
			ftpReceives[i] = NULL;
		}
	}
}
void FtpManager::ClearClientFromFtp(int clientIndex)
{
	if(clientIndex == -1)
		return;
	clearData = true;
	clearClients[clientIndex] = true;
}
void FtpManager::DoClearClient(int clientIndex)
{
	for(int i = 0; i < MAX_FTP_TRANSFERS; ++i){
		if(ftpTransfers[i]){
			if(ftpTransfers[i]->target == clientIndex){
				DeleteTransfer(ftpTransfers[i]);
			}
		}
	}
	for(int i = 0; i < MAX_FTP_REQUESTS; ++i){
		if(ftpRequests[i]){
			// We should remove any requests we have created
			if(clientIndex == Network.info.yourClientIndex){
				for(int j = 0; j < MAX_FTP_FILES; ++j){
					if(ftpRequests[i]->files[j]){
						delete ftpRequests[i]->files[j];
						ftpRequests[i]->files[j] = NULL;
					}
				}
				delete ftpRequests[i];
				ftpRequests[i] = NULL;
			}
		}
	}
}

int FtpManager::GetFreeFtpRequestSlot()
{
	for(int i = 0; i < MAX_FTP_REQUESTS; ++i)
	{
		if (ftpRequests[i] == NULL)
			return i;
	}
	return -1;
}
FtpRequest * FtpManager::GetFtpRequest( int slot )
{
	return ftpRequests[slot];
}
FtpRequest * FtpManager::GetFtpRequestById( int id )
{
	for(int i = 0; i < MAX_FTP_REQUESTS; ++i)
		if(ftpRequests[i]->id == id)
			return ftpRequests[i];
}
FtpRequest * FtpManager::CreateFtpRequest()
{
	int slot = GetFreeFtpRequestSlot();
	if(slot == -1){
		print("Can't create any more Ftp Requests at the moment!");
		return NULL;
	}

	static int ftpIdCounter = 0;
	ftpRequests[slot] = new FtpRequest(ftpIdCounter++);
	return GetFtpRequest(slot);
}
bool FtpManager::AddFileToFtpRequest(FtpRequest *request, const char *filename)
{
	// First find a free file slot
	FtpFile *file = NULL;
	for(int i = 0; i < MAX_FTP_FILES; ++i)
	{
		if(request->files[i] == NULL)
		{
			request->files[i] = new FtpFile();
			file = request->files[i];
			break;
		}
	}
	if(file == NULL)
		return false;

	// Try to open the file and gather some information
	strncpy(file->filename, filename, MAX_FILENAME_LENGTH);

	std::ifstream filetest;
	filetest.open(filename, std::ios::in | std::ios::ate | std::ios::binary);
	if(filetest.is_open())
	{
		file->filesize = (unsigned int)filetest.tellg();
		for(unsigned int i = 0; i < file->filesize; i += MAX_FTP_DATA)
			++file->packets;
		filetest.close();
	}
	else
	{
		delete file;
		file = NULL;
		return false;
	}

	return true;
}

int FtpManager::GetFreeTransferSlot()
{
	for(int i = 0; i < MAX_FTP_TRANSFERS; ++i)
	{
		if(ftpTransfers[i] == NULL)
			return i;
	}
	return -1;
}
FtpTransfer * FtpManager::GetFtpTransfer( int slot )
{
	return ftpTransfers[slot];
}
int FtpManager::GetFtpTransferSlot( FtpTransfer *transfer )
{
	for(int i = 0; i < MAX_FTP_TRANSFERS; ++i)
	{
		if(ftpTransfers[i] == NULL)
			continue;

		if(ftpTransfers[i] == transfer)
			return i;
	}
	return -1;
}
FtpTransfer * FtpManager::GetFtpTransferFromFtpID( int ftpID, int target )
{
	for(int i = 0; i < MAX_FTP_TRANSFERS; ++i)
	{
		if(ftpTransfers[i] == NULL)
			continue;

		if(ftpTransfers[i]->request->id == ftpID && ftpTransfers[i]->target == target)
		{
			return ftpTransfers[i];
		}
	}
	return NULL;
}
bool FtpManager::CreateFtpTransfer( FtpRequest * request, int target )
{
	int slot = GetFreeTransferSlot();
	if(slot == -1){
		print("Can't create any more Ftp Transfers at the moment");
		return false;
	}

	ftpTransfers[slot] = new FtpTransfer(request, target);
	return true;
}
void FtpManager::DeleteTransfer( FtpTransfer *transfer )
{
	int slot = GetFtpTransferSlot(transfer);
	if(slot == -1)
		return;
	if(ftpTransfers[slot]){

		if(ftpTransfers[slot]->request){
			/* DON*T DELETE REQUESTS
			for(int i = 0; i < MAX_FTP_FILES; ++i){
				if(ftpTransfers[slot]->request->files){
					delete[] ftpTransfers[slot]->request->files[i];
					ftpTransfers[slot]->request->files[i] = NULL;
				}
			}
			delete[] ftpTransfers[slot]->request;
			ftpTransfers[slot]->request = NULL;*/
		}
		if(ftpTransfers[slot]->status){
			delete[] ftpTransfers[slot]->status;
			ftpTransfers[slot]->status = NULL;
		}
		if(ftpTransfers[slot]->timeSent){
			delete[] ftpTransfers[slot]->timeSent;
			ftpTransfers[slot]->timeSent = NULL;
		}
		delete ftpTransfers[slot];
		ftpTransfers[slot] = NULL;
	}
	transfer = NULL;
}
void FtpManager::DeleteTransfer( int slot )
{
	if(slot == -1)
		return;
	if(ftpTransfers[slot]){
		if(ftpTransfers[slot]->request){
			/* DON*T DELETE REQUESTS
			for(int i = 0; i < MAX_FTP_FILES; ++i){
				if(ftpTransfers[slot]->request->files){
					delete[] ftpTransfers[slot]->request->files[i];
					ftpTransfers[slot]->request->files[i] = NULL;
				}
			}
			delete[] ftpTransfers[slot]->request;
			ftpTransfers[slot]->request = NULL;*/
		}
		if(ftpTransfers[slot]->status){
			delete[] ftpTransfers[slot]->status;
			ftpTransfers[slot]->status = NULL;
		}
		if(ftpTransfers[slot]->timeSent){
			delete[] ftpTransfers[slot]->timeSent;
			ftpTransfers[slot]->timeSent = NULL;
		}
		delete ftpTransfers[slot];
		ftpTransfers[slot] = NULL;
	}
}

int FtpManager::GetFreeReveiveSlot()
{
	for(int i = 0; i < MAX_FTP_REQUESTS; ++i)
	{
		if(ftpReceives[i] == NULL)
			return i;
	}
	return -1;
}
FtpReceive * FtpManager::GetFtpReceive( int slot )
{
	return ftpReceives[slot];
}
int FtpManager::GetReceiveSlot( FtpReceive *receive )
{
	for(int i = 0; i < MAX_FTP_REQUESTS; ++i)
	{
		if(ftpReceives[i] == NULL)
			continue;

		if(ftpReceives[i] == receive)
			return i;
	}
	return -1;
}
FtpReceive * FtpManager::GetFtpReceiveFromFtpID( int ftpID )
{
	for(int i = 0; i < MAX_FTP_REQUESTS; ++i)
	{
		if(ftpReceives[i] == NULL)
			continue;

		if(ftpReceives[i]->ftpID == ftpID )
		{
			return ftpReceives[i];
		}
	}
	return NULL;
}
bool FtpManager::CreateFtpReceive( int ftpID, int files, char filenames[MAX_FTP_FILES][MAX_FILENAME_LENGTH], unsigned int filesizes[MAX_FTP_FILES], int parts[MAX_FTP_FILES] )
{
	int slot = GetFreeReveiveSlot();
	if(slot == -1){
		print("Can't create any more Ftp Receive at the moment");
		return false;
	}

	ftpReceives[slot] = new FtpReceive(slot, ftpID, files, filenames, filesizes, parts);
	return true;
}
void FtpManager::DeleteReceive( FtpReceive *receive )
{
	int slot = GetReceiveSlot(receive);
	if(slot == -1)
		return;
	if(ftpReceives[slot]){
		delete ftpReceives[slot];
		ftpReceives[slot] = NULL;
	}
	receive = NULL;
}
void FtpManager::DeleteReceive( int slot )
{
	if(slot == -1)
		return;
	if(ftpReceives[slot]){
		delete ftpReceives[slot];
		ftpReceives[slot] = NULL;
	}
}

void FtpManager::ReceiveFtpRequest(PacketMakeFtpRequest *pkt)
{
	// Write out requests data
	print("Received FTP request number " << pkt->ftpID);
	print("Total of " << pkt->files << " file(s).");
	for(int i = 0; i < pkt->files; ++i)
	{
		print("\n* File " << i+1);
		print("Filename: " << pkt->filenames[i]);
		print("Filesize: " << pkt->filesizes[i] << " bytes");
		print("Divided into " << pkt->packets[i] << " packets");
	}

	// Check if the any file exists, then add a number after them
	for(int i = 0; i < pkt->files; ++i)
	{
		String filename(pkt->filenames[i]);
		std::ifstream ifile(filename.c_str());
		// If the file already exists then add a number!
		int num = 0;
		while(ifile)
		{
			++num;
			// Create a string with just the filename without the extension
			filename = pkt->filenames[i];
			List<String> filetok = filename.Tokenize(".");
			if(filetok.Size() > 1)
			{
				filename = "";
				for(int i = 0; i < filetok.Size()-1; ++i)
					filename += filetok[i];
				filename.Add("(" + String::ToString(num) + ").");
				// Add the extension
				filename += filetok[filetok.Size()-1];
			}
			ifile.close();
			ifile.open(filename.c_str());
		}
		ifile.close();
		// Create the file
		std::ofstream datafile;
		datafile.open(filename.c_str(), std::ios::app | std::ios::binary);
		if(datafile.is_open())
		{
			datafile.close();
		}

		// If we had to change the filename
		if(!(filename == pkt->filenames[i]))
			strncpy(pkt->filenames[i], filename, MAX_FILENAME_LENGTH);
	}

	// Remember what to receive
	Ftp.CreateFtpReceive( pkt->ftpID, pkt->files, pkt->filenames, pkt->filesizes, pkt->packets);

	// Then send answer
	print("\nMagically accept all FTP request atm!");
	Network.QueuePacket(new PacketAnswerFtpRequest(pkt->ftpID, true), pkt->sender);
}
void FtpManager::CheckFtpRequest(PacketAnswerFtpRequest *pkt)
{
	String str;
	str.Add(Network.GetClient(pkt->sender)->name);
	str.Add(((pkt->answer)?" accepted ":" declined "));
	str.Add("the FTP request number ");
	str.ToString(pkt->ftpID);
	// If client accepted the request
	if(pkt->answer)
	{
		// Start a FTP transfer to that client
		Ftp.CreateFtpTransfer(Ftp.GetFtpRequestById(pkt->ftpID), pkt->sender);
	}

	print(str.c_str());
}
void FtpManager::ReceiveFtpData(PacketFtpData *pkt)
{
	// Find receive request
	FtpReceive *receive = Ftp.GetFtpReceiveFromFtpID(pkt->ftpID);
	if(!receive){
		print("Could not find receive information for received data packet.");
		return;
	}
	print("FTP: " << pkt->ftpID << " File: " << (pkt->fileIndex + 1) << "/" << receive->files << " Name: " << receive->filenames[pkt->fileIndex] << " Part: " << (pkt->packetNumber+1) << "/" << receive->parts[pkt->fileIndex]);

	// Calculate data parts index
	int index = pkt->packetNumber;
	for(int i = 0; i < pkt->fileIndex; ++i)
		index += receive->parts[i];

	// Simple checking so we handle the packets in order
	// Make sure we don't handle it before previous packet
	if(index != 0 && receive->status[(index-1)*2 + 0] == 0)
		return;
	// We don't want to try to read data from a packet that is incorrect
	if(pkt->sizeToRead > MAX_FTP_DATA)
		return;

	// If we haven't received this piece of data before
	if(receive->status[index*2 + 0] == 0)
	{
		receive->status[index*2 + 0] = 1;
		if(Network.GetConnectionType() == SOCK_STREAM)
		{
			// Read and send that part of the data
			std::ofstream datafile;
			datafile.open(receive->filenames[pkt->fileIndex], /*std::ios::in |*/ std::ios::app | std::ios::binary);
			if(datafile.is_open())
			{
				datafile.write(pkt->data, pkt->sizeToRead);
				datafile.close();
			}
		}
		else
		{
			// TODO: Implement receiving FtpData in UDP connection mode, after implementing UDP connection mode ofc!
		}
	}

	// Reply that we received the data part!
	PacketFtpDataReply *replyPkt = new PacketFtpDataReply(pkt->ftpID, pkt->fileIndex, pkt->packetNumber);
	Network.QueuePacket(replyPkt, pkt->sender);
}
void FtpManager::ReceiveFtpDataPartConfirmation(PacketFtpDataReply *pkt)
{
	// Get the transfer
	FtpTransfer *transfer = Ftp.GetFtpTransferFromFtpID(pkt->ftpID, pkt->sender);
	if(!transfer)
		return;

	// Get the index of the status
	int index = 0;
	for(int file = 0; file < pkt->fileIndex; ++file)
		index += transfer->request->files[file]->packets;

	// Flag the part of the packet as received!
	transfer->status[(index + pkt->packetNumber) * 2 + 1] = 1;
}
void FtpManager::FinishFtpRequest(PacketFtpFinished *pkt)
{
	print("FTP finished, ID: " << pkt->ftpID);

	FtpReceive *receive = Ftp.GetFtpReceiveFromFtpID(pkt->ftpID);
	Ftp.DeleteReceive(receive);
}

void FtpManager::SendFtp()
{
	/* This is nicer but seems to be way slower D: T_T ;___; *sadface
	// Get current time
	long long currentTime = Timer::GetCurrentTimeMs();

	// Check if we should send FTP data
	if(currentTime - ftpTime < FTP_DELAY_TIME)
		return;
	ftpTime = currentTime;

	// Check all transfers
	for(int i = 0; i < MAX_FTP_TRANSFERS; ++i)
	{
		if(ftpTransfers[i] == NULL)
			continue;

		// To check
		bool transferComplete = true;

		FtpTransfer *transfer = GetFtpTransfer(i);
		// An increment to temporary increase the index of the file/part we're sending
		int tmpIncFile = 0,tmpIncPart = 0;
		FtpFile *ftpFile = transfer->request->files[transfer->currentFile];
		while(ftpFile)
		{
			// Check if we received confirmation upon target receiving data
			if(transfer->status[transfer->statusIndex*2 + 1] == 1)
			{
				transfer->statusIndex++;
				tmpIncPart = 0;
				tmpIncFile = 0;
				// If target received last part of a file, go to next file
				if(++transfer->currentPart == ftpFile->packets){
					ftpFile = transfer->request->files[++transfer->currentFile];
					transfer->currentPart = 0;
					++transfer->statusIndex;
					continue;
				}
			}
			// Check if it is not yet time to resend it
			else if((currentTime - transfer->timeSent[(transfer->statusIndex + tmpIncFile + tmpIncPart)]) < FTP_RESEND_TIME)
			{
				transferComplete = false;
				tmpIncPart++;
				if(tmpIncPart == transfer->request->files[transfer->currentFile + tmpIncFile]->packets){
					tmpIncFile++;
					tmpIncPart = transfer->currentPart * -1;
				}
				continue;
			}

			// Data to send
			char data[MAX_FTP_DATA];

			// Get size to read
			unsigned long sizeToRead = ftpFile->filesize - (transfer->currentPart+tmpIncPart)*MAX_FTP_DATA;
			if(sizeToRead > MAX_FTP_DATA)
				sizeToRead = MAX_FTP_DATA;

			// Read and send that part of the data
			std::ifstream datafile;
			datafile.open(ftpFile->filename, std::ios::in | std::ios::ate | std::ios::binary);
			if(datafile.is_open())
			{
				datafile.seekg((transfer->currentPart + tmpIncPart) * MAX_FTP_DATA, std::ios_base::beg);
				datafile.read(data, sizeToRead);
				datafile.close();
			}

			// Change the sent status
			if((transfer->statusIndex + tmpIncFile + tmpIncPart) >= transfer->statusSize - 1)
				int b = 0;
			transfer->status[(transfer->statusIndex + tmpIncFile + tmpIncPart)*2 + 0] = 1;
			// Set the time sent
			transfer->timeSent[transfer->statusIndex + tmpIncFile + tmpIncPart] = currentTime;

			// Create data packet to send
			print("Sending part " << (transfer->currentPart + tmpIncPart + 1) << " / " << ftpFile->packets);
			PacketFtpData *dataPkt = new PacketFtpData(transfer->request->id, (transfer->currentFile + tmpIncFile), (transfer->currentPart + tmpIncPart), data, sizeToRead);
			Network.QueuePacket(dataPkt, transfer->target);

			// Flag to go to next transfer
			transferComplete = false;
			break;
		}

		// Check if a transfer is complete
		if(transferComplete)
		{
			print("FTP transfer to " << Network.GetClient(transfer->target)->name << ", ID: " << transfer->request->id << " succeeded!");
			Network.QueuePacket(new PacketFtpFinished(transfer->request->id), transfer->target);
			DeleteTransfer(transfer);
		}
	}
	*/

	// Get current time
	long long currentTime = Timer::GetCurrentTimeMs();

	// Check if we're clearing all FTP data or just some clients
	if(clearData)
	{
		clearData = false;
		if(clearAll){
			clearAll = false;
			DoClearAll();
		}
		for(int i = 0; i < MAX_CLIENTS; ++i){
			if(clearClients[i]){
				clearClients[i] = false;
				DoClearClient(i);
			}
		}
	}

	if(currentTime - ftpTime < FTP_DELAY_TIME)
	return;
	ftpTime = currentTime;

	// Check all transfers
	for(int i = 0; i < MAX_FTP_TRANSFERS; ++i)
	{
		if(ftpTransfers[i] == NULL)
		continue;

		// To check
		bool goToNextTransfer = false;
		// Create index for knowing where to flag
		int index = 0;

		FtpTransfer *transfer = GetFtpTransfer(i);
		// Gp through all the transfer's files
		for(int file = 0; file < MAX_FTP_FILES; ++file)
		{
			// Don't check the following files if one is NULL
			FtpFile *ftpFile = transfer->request->files[file];
			if(ftpFile == NULL)
			break;

			// Check if all parts are sent
			for(int part = 0; part < ftpFile->packets; ++part)
			{
				// First, check if we're just adding indices
				if(goToNextTransfer){
				++index;
				continue;
				}
				// Make sure the target hasn't already got the data
				if(transfer->status[index*2 + 1] == 1){
				++index;
				continue;
				}
				// If not yet received, check if we should resend it yet.
				if((currentTime - transfer->timeSent[index]) < FTP_RESEND_TIME){
					// If we're the last part of a file in the transfer, we must not complete the transfer yet
					if(part == (ftpFile->packets-1))
					goToNextTransfer = true;
					++index;
					continue;
				}

				// Data to send
				char data[MAX_FTP_DATA];

				// Get size to read
				unsigned long sizeToRead = ftpFile->filesize - part*MAX_FTP_DATA;
				if(sizeToRead > MAX_FTP_DATA)
				sizeToRead = MAX_FTP_DATA;

				// Read and send that part of the data
				std::ifstream datafile;
				datafile.open(ftpFile->filename, std::ios::in | std::ios::ate | std::ios::binary);
				if(datafile.is_open())
				{
					datafile.seekg(part * MAX_FTP_DATA, std::ios_base::beg);
					datafile.read(data, sizeToRead);
					datafile.close();
				}

				// Change the sent status
				transfer->status[index*2 + 0] = 1;
				// Set the time sent
				transfer->timeSent[index] = currentTime;

				// Create data packet to send
				print("Sending part " << (index+1) << " / " << ftpFile->packets);
				PacketFtpData *dataPkt = new PacketFtpData(transfer->request->id, file, part, data, sizeToRead);
				Network.QueuePacket(dataPkt, transfer->target);

				// Increase index
				++index;
				// Flag to go to next transfer
				goToNextTransfer = true;
			}
		}

		// Check if a transfer is complete
		if(!goToNextTransfer)
		{
			print("FTP transfer to " << Network.GetClient(transfer->target)->name << ", ID: " << transfer->request->id << " succeeded!");
			Network.QueuePacket(new PacketFtpFinished(transfer->request->id), transfer->target);
			DeleteTransfer(transfer);
		}
	}
}

bool FtpManager::MakeFtpRequest(FtpRequest *request, int target)
{
	if(!request)
		return false;

	PacketMakeFtpRequest *ftpRequestPacket = new PacketMakeFtpRequest(request);
	Network.QueuePacket(ftpRequestPacket, target);
	return true;
}

FtpRequest::FtpRequest( int id )
{
	this->id = id;
	for(int i = 0; i < MAX_FTP_FILES; ++i)
	{
		this->files[i] = NULL;
	}
}

FtpTransfer::FtpTransfer( FtpRequest *request, int target )
{
	this->request = request;
	this->target = target;
	this->currentFile = 0;
	this->currentPart = 0;
	this->statusIndex = 0;
	int size = 0;
	for(int i = 0; i < MAX_FTP_FILES; ++i)
	{
		if(request->files[i] != NULL)
		{
			size += request->files[i]->packets * 2;
		}
	}
	this->statusSize = size;
	this->status = new int[size];
	for(int i = 0; i < size; ++i)
	{
		this->status[i] = 0;
	}
	this->timeSent = new long long[size/2];
	for(int i = 0; i < size/2; ++i)
		this->timeSent[i] = 0;
}

FtpReceive::FtpReceive( int slotID, int ftpID, int files, char filenames[MAX_FTP_FILES][MAX_FILENAME_LENGTH], unsigned int filesizes[MAX_FTP_FILES], int parts[MAX_FTP_FILES] )
{
	this->id = slotID;
	this->ftpID = ftpID;
	this->files = files;
	int size = 0;
	for(int i = 0; i < MAX_FTP_FILES; ++i)	{
		strncpy(this->filenames[i], filenames[i], MAX_FILENAME_LENGTH);
		this->filesizes[i] = filesizes[i];
		this->parts[i] = parts[i];
		size += parts[i] * 2;
	}
	this->status = new int[size];
	for(int i = 0; i < size; ++i)
		this->status[i] = 0;
}

#endif // USE_NETWORK & USE_FTP
