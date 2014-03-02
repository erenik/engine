/// Emil Hedemalm
/// 2013-11-28
/// Class for encapsulating an individual stream (from host) to one or more clients (uni-/multi-cast).

#ifndef STREAM_H
#define STREAM_H

#include "SIP/SIPPacket.h"

class Peer;

class Stream {
public:
	Stream();
	
	// Regular playback functions, woo!
	/// Resume/start playback of stream.
	void Play();
	/// Pausees the stream, setting it to the "Paused" state.
	void Pause();
	/// Seeks in seconds. Usage should be limited to those streams which are streaming from file.
	void Seek(int timeInSeconds);
	/// Stop and seek to beginning of stream.
	void Stop();
	/// Queries if the stream is currently paused.
	bool IsPaused();
	/// Returns true only if the stream is currently active.
	bool IsPlaying();

	/// To add a client to the stream!
	void AddClient(Peer * client);
	/// Removes client from the list and from the multicast group.
	void RemoveClient(Peer * client);
	/// Disconnects/removes all connected clients.
	void ClearClients();
	/// Handler for hwne the sources are ready to stream!
//	void OnPadAdded(const QGst::PadPtr& pad);
	/// Message handler for stream notifications
//	void OnBusMessage(const QGst::MessagePtr& message);

	/// Set when managing stream. TODO: Make private?
	bool audioOnly;

	/** Packet copy of the original SIP request as the caps are sent with a SIP OK as a reply to this request. 
		In a more complex application all these "requests" should probably be given their own data-strucatures,
		a manager etc., but for now we will try and keep it simple...
	*/
	SIPPacket requestPacket;
	/// For storing the caps related to the media for new connecting clients? TODO: Make private?
	String mediaCaps;
	
	/// Name of the stream.
	String name;
	/// Media currently streaming.
	String media;

	/// List of all clients listening to this stream.
	List<Peer*> clients;
	/// If this is an upstream (meaning we are the host). The stream can only be 1 of up, down or local at any one time!
	bool IsUpStream();
	/// If this is a downstream (meaning we are the client). The stream can only be 1 of up, down or local at any one time!
	bool IsDownStream();
	/// If streaming to self, then the connectedClients will be empty, etc. The stream can only be 1 of up, down or local at any one time!
	bool IsLocal();

	/// Returns current playback-time in seconds.
	int CurrentTime();
	/// Returns media duration in seconds.
	int MaxTime();

//signals:
//	void MediaCapsNegotiated(const String& str);
	/// Emitted once a stream has successfully established media caps that can then be sent to all requesting peers.
//	void MediaCapsNegotiated(Stream * forStream);
private:
	/// QtGStreamer Pipeline that handles the media-stream.
//	QGst::PipelinePtr pipeline;
	bool isUpStream;
	bool isDownStream;
	/// If flagged, the stream is used only by the host to play up media for itself (special case)
	bool isLocal;
	/// Used to keep track if it's currently playing/paused/stopped.
	int playbackState;
};
    

#endif
