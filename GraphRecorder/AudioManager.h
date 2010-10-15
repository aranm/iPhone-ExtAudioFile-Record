/*
 *  AudioManager.h
 *  GraphRecorder
 *
 *  Created by Aran Mulholland on 14/10/10.
 *  Copyright 2010 None. All rights reserved.
 *
 */


#include <AudioUnit/AudioUnit.h>
#import "AudioToolbox/AudioToolbox.h"
#include "InMemoryAudioFile.h"

class AudioManager{
	
private:
	
	//the graph of audio connections
	AUGraph graph;
	
	//the recording description
	AudioStreamBasicDescription recordingAudioFormat;
	
	//the AUNode
	AUNode outputMixerNode;
	
	void InitialiseAudioGraph();
	
public:
	
	AudioManager();
	~AudioManager();
	
	InMemoryAudioFile *inMemoryAudiofile;
	
	//all of these variables have to be marked as public because we
	//refer to them from the render callback
	AudioUnit remoteIOUnit;
	BOOL mIsRecording;
	ExtAudioFileRef	mRecordFile;
	
	//recording methods
	OSStatus StartRecord(CFURLRef url, bool aacRecording);
	void StopRecord();
	inline BOOL GetIsRecording() { return mIsRecording; }	
	

};