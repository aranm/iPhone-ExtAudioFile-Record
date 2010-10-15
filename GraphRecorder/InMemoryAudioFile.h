//
//  InMemoryAudioFile.h
//  HelloWorld
//
//  Created by Aran Mulholland on 22/02/09.
//  Copyright 2009 Aran Mulholland. All rights reserved.
//


#include <AudioUnit/AudioUnit.h>
#import "AudioToolbox/AudioToolbox.h"
#include "CAStreamBasicDescription.h"

typedef struct {
    AudioStreamBasicDescription asbd;
    void *leftData;
    void *rightData;
	UInt32 numFrames;
	UInt32 sampleNum;
} SoundBuffer, *SoundBufferPtr;

class InMemoryAudioFile {
	
private:
//	AudioStreamBasicDescription		mDataFormat;                    
//    AudioFileID						mAudioFile;                     
//    UInt32							bufferByteSize;                 
//    SInt64							mCurrentPacket;                 
//    UInt32							mNumPacketsToRead;              
//    AudioStreamPacketDescription	*mPacketDescs;                  
//	SInt64							packetCount;
//	UInt32							*audioData;
//	SInt64							packetIndex;
//	SInt64							leftPacketIndex;
//	SInt64							rightPacketIndex;
	
	Boolean		isPlaying;
	
	SoundBuffer mSoundBuffer;
	
public:
	
	InMemoryAudioFile();
    ~InMemoryAudioFile();
	
	void loadFile(CFURLRef sourceURL);
	
	OSStatus FillBuffer(AudioBufferList * ioData, UInt32 inNumberFrames);
	
	//reset the index to the start of the file
	void reset();
	void play();
	void stop();
	
	//void SetIsPlaying(Boolean value) { isPlaying = value; }
	Boolean IsPlaying() { return isPlaying; }
};
