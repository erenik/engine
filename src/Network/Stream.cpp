/// Emil Hedemalm
/// 2013-11-28
/// Class for encapsulating an individual stream (from host) to one or more clients (uni-/multi-cast).

#include "Stream.h"
#include "Peer.h"

/// Clean the ones below here later, pallar inte just nu
#include <cassert>

//#define GET_PIPELINE_ELEMENT(pipe, name) pipe.dynamicCast<QGst::Bin>()->getElementByName(name)

Stream::Stream(){
	name = "Unnamed";
	isLocal = false;
	isUpStream = isDownStream = false;
}
bool Stream::IsUpStream(){
	return isUpStream;
}
bool Stream::IsDownStream(){
	return isDownStream;
}
bool Stream::IsLocal(){
	return isLocal;
}


// Resume/start playback of stream.
void Stream::Play()
{
/*	if (this->pipeline.isNull())
	{
		std::cout << "Stream::Play, null pipeline, aborting";
		return;
	}
	std::cout << "Stream::Play; setting state to playing.";
	this->pipeline->setState(QGst::StatePlaying);
*/
}
// Pausees the stream, setting it to the "Paused" state.
void Stream::Pause()
{
/*	if (this->pipeline.isNull())
		return;
	this->pipeline->setState(QGst::StatePaused);
*/

}

// Seeks in seconds. Usage should be limited to those streams which are streaming from file.
void Stream::Seek(int timeInSeconds)
{
	/*
	if (this->pipeline.isNull())
		return;
	// Create a QGst::SeekEvent and send it to the pipeline.
	QTime position;
	position = position.addSecs(timeInSeconds);
	std::cout <<"Streaming to second: "<<timeInSeconds;
	QGst::SeekEventPtr event = QGst::SeekEvent::create(1.0, QGst::FormatTime, QGst::SeekFlagFlush, QGst::SeekTypeSet, QGst::ClockTime::fromTime(position), QGst::SeekTypeNone, QGst::ClockTime::None);
	this->pipeline->sendEvent(event);
	*/
}

// Stop and seek to beginning of stream.
void Stream::Stop()
{
	/*
	if (this->pipeline.isNull())
		return;
	Seek(0);
	this->pipeline->setState(QGst::StateReady);
	*/
}

/// Queries if the stream is currently paused.
bool Stream::IsPaused()
{
	/*
	return playbackState == QGst::StatePaused;
	*/
	return false;
}

/// Returns true only if the stream is currently active.
bool Stream::IsPlaying()
{
	/*
	return playbackState == QGst::StatePlaying;
	*/
	return false;
}

void Stream::AddClient(Peer * client){
	/*
	if (this->pipeline.isNull())
        	return;
	const String& clientAddress = client->ipAddress;
	QGst::ElementPtr audio = this->pipeline.dynamicCast<QGst::Bin>()->getElementByName("audioUdpSink");
	QGst::ElementPtr video = this->pipeline.dynamicCast<QGst::Bin>()->getElementByName("videoUdpSink");

	QGlib::emit<void>(audio, "add", clientAddress, 5101);
	QGlib::emit<void>(video, "add", clientAddress, 5102);
	clients.Add(client);
    client->stream = this;
	*/
}

// Removes client from the list and from the multicast group.
void Stream::RemoveClient(Peer * client){
	/*
	if (this->pipeline.isNull())
        	return;
	const String& clientAddress = client->ipAddress;
	QGst::ElementPtr audio = this->pipeline.dynamicCast<QGst::Bin>()->getElementByName("audioUdpSink");
	QGst::ElementPtr video = this->pipeline.dynamicCast<QGst::Bin>()->getElementByName("videoUdpSink");

	QGlib::emit<void>(audio, "remove", clientAddress, 5101);
	QGlib::emit<void>(video, "remove", clientAddress, 5102);
	clients.removeOne(client);
	// Remove links from the client to the stream as well.
	if (client->stream == this){
		client->stream = NULL;
		client->isStreaming = false;
	}
	/// Stop if no clients left on this stream.	
	if (clients.Size() == 0){
		this->pipeline->setState(QGst::StateReady);
	}
	*/
}

// Disconnects/removes all connected clients.
void Stream::ClearClients(){
	/*
	std::cout<< "Stream::ClearClients";
	while (clients.Size()){
		Peer * c = clients.at(0);
		RemoveClient(c);
	}
	*/
}

/*
void Stream::OnPadAdded(const QGst::PadPtr& pad)
{
    String name = pad->name();
    String type = pad->caps()->internalStructure(0)->name();
    std::cout << "Name:" << name << "Type:" << type;
    
    QGst::ObjectPtr objptr = pad->parentElement();
    
    std::cout << objptr->pathString();
    
    while (objptr->parent())
        objptr = objptr->parent();
    
    QGst::PipelinePtr pipeline = objptr.dynamicCast<QGst::Pipeline>();
    
    if (type.Contains("audio"))
    {
        QGst::ElementPtr audioConverter = pipeline.dynamicCast<QGst::Bin>()->getElementByName("audioConvert");
        Q_ASSERT(audioConverter);
        QGst::PadPtr sink = audioConverter->getStaticPad("sink");
        if (!sink)
        {
			std::cout << "OnPadAdded: Didn't find audioconvert sink";
			return;
		}
        
        pad->link(sink);
    }
    else if (type.Contains("video"))
    {
        QGst::ElementPtr videoConverter = pipeline.dynamicCast<QGst::Bin>()->getElementByName("videoConvert");
        Q_ASSERT(videoConverter);
        QGst::PadPtr sink = videoConverter->getStaticPad("sink");
        if (!sink){
		std::cout << "OnPadAdded: Didn't find audioconvert sink";
		return;
	}
        
        pad->link(sink);
    }
}

void Stream::OnBusMessage(const QGst::MessagePtr& message)
{
    std::cout << "Stream BusMessage:" << message->typeName();
    
    switch (message->type())
    {
    case QGst::MessageAsyncDone:
    {
        String capsStr = "";

        QGst::PadPtr pad = GET_PIPELINE_ELEMENT(this->pipeline, "audioUdpSink")->getStaticPad("sink");
        if (pad)
        {
		QGst::CapsPtr caps = pad->negotiatedCaps();
		std::cout << "Caps: "<<caps;
		if (caps){
			capsStr.Add(caps->toString());
		}
		if (!audioOnly){
			pad = GET_PIPELINE_ELEMENT(this->pipeline, "videoUdpSink")->getStaticPad("sink");
			capsStr.Add(";");
			capsStr.Add(pad->negotiatedCaps()->toString());
		}
		this->mediaCaps = capsStr;
		std::cout << "Caps: "<<capsStr;
	//	Q_EMIT MediaCapsNegotiated(capsStr);
		Q_EMIT MediaCapsNegotiated(this);
        }
        break;
    }
    case QGst::MessageWarning:
    {
        QGst::WarningMessagePtr msg = message.dynamicCast<QGst::WarningMessage>();
        
        std::cout << msg->error() << msg->debugMessage();
        break;
    }
    case QGst::MessageError:
    {
        const QGlib::Error& error = message.staticCast<QGst::ErrorMessage>()->error();
        qCritical() << message->source()->name() << "Error" << error.code() << "in" << error.domain().toString() << "-" << error.message();
        //this->Stop();
        break;
    }
    case QGst::MessageStateChanged: //The element in message->source() has changed state
    {
    	const QGst::StateChangedMessagePtr& stateMsg = message.staticCast<QGst::StateChangedMessage>();
        static const String stateStr[] = { "StateVoidPending", "StateNull", "StateReady", "StatePaused", "StatePlaying" };
        std::cout << "Source:" << stateMsg->source()->name() << "Old:" << stateStr[stateMsg->oldState()] << "New:" << stateStr[stateMsg->newState()] << "Pending:" << stateStr[stateMsg->pendingState()];
        // Set playback state so we know for later
		playbackState = stateMsg->newState();        
        break;
    }
    default:
        break;
    }
}
*/

/// Returns current playback-time in seconds.
int Stream::CurrentTime(){
/*	if (this->pipeline){
		//here we query the pipeline about its position
		//and we request that the result is returned in time format
		QGst::PositionQueryPtr query = QGst::PositionQuery::create(QGst::FormatTime);
		this->pipeline->query(query);
		QTime time = QGst::ClockTime(query->position()).toTime();
		int seconds = QTime().secsTo(time);
		return seconds;
	} 
	*/
	return -1;
}
/// Returns media duration in seconds.
int Stream::MaxTime(){
	/*
	if (!this->pipeline)
		return -1;
	QGst::DurationQueryPtr query = QGst::DurationQuery::create(QGst::FormatTime);
        this->pipeline->query(query);
	QTime time = QGst::ClockTime(query->duration()).toTime();
	int seconds = QTime().secsTo(time);
        return seconds;
		*/
	return 0;
}

