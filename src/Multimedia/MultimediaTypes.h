/// Emil Hedemalm
/// 2014-02-16
/** Enumeration of multimedia types that we identify and support. 
	Note that this type is corelated to the encoding and file-ending, more so than the contents.
*/

#ifndef MULTIMEDIA_TYPES_H
#define MULTIMEDIA_TYPES_H

namespace MultimediaType {
enum multimediaTypes {
	UNKNOWN,
	OGG, /// Container format. Can include both Vorbis, Theora, etc.
	
};
};

#endif