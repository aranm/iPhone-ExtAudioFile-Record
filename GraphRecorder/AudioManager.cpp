/*
 *  AudioManager.cpp
 *  GraphRecorder
 *
 *  Created by Aran Mulholland on 14/10/10.
 *  Copyright 2010 None. All rights reserved.
 *
 */

#include "AudioManager.h"

#pragma mark -
#pragma mark Callbacks

static OSStatus audioOutputCallback(void *inRefCon, 
									AudioUnitRenderActionFlags *ioActionFlags, 
									const AudioTimeStamp *inTimeStamp, 
									UInt32 inBusNumber, 
									UInt32 inNumberFrames, 
									AudioBufferList *ioData) {  
	
	
	//get a reference to the 'this'
	AudioManager *audioManager = (AudioManager *)inRefCon;
	
	if (audioManager->inMemoryAudiofile != NULL && audioManager->inMemoryAudiofile->IsPlaying() == true) {
		//fill the buffers with audio goodness
		audioManager->inMemoryAudiofile->FillBuffer(ioData, inNumberFrames);
	}
	else{
		//fill the buffers with silence
		for (int i = 0 ; i < ioData->mNumberBuffers; i++){
			AudioBuffer buffer = ioData->mBuffers[i];
			//fill the buffer with zeros
			memset((Byte *)buffer.mData, 0, buffer.mDataByteSize);
		}
	}

	//if we are recording write the data to a file
	if (audioManager->mIsRecording){
		// write packets to file asyncronously, this prevents the render thread being blocked
		// if the file operation takes too long (which it would)
		ExtAudioFileWriteAsync(audioManager->mRecordFile, inNumberFrames, ioData);
	}
	
	return 0;
}

#pragma mark -
#pragma mark Initialisation

//method sets up an audio session in a very poor fashion
//in production code you would need to setup your interruption and route change callbacks
void InitialiseAudioSession(){

    AudioSessionInitialize(NULL, NULL, NULL, NULL);
	
	UInt32 audioCategory = kAudioSessionCategory_LiveAudio;
	AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(audioCategory), &audioCategory);
	
	AudioSessionSetActive(YES);	
}

//method sets up a graph with one node in it
//once again this is demo code, i'm not checking the error results of my sets
void AudioManager::InitialiseAudioGraph() {
	
	AudioComponentDescription outputDescription;
	
	//the AUNode
	AUNode outputNode;
	
	//create the graph
	OSErr err = noErr;
	err = NewAUGraph(&graph);
	
	//describe the node, this is our output node it is of type remoteIO
	outputDescription.componentFlags = 0;
	outputDescription.componentFlagsMask = 0;
	outputDescription.componentType = kAudioUnitType_Output;
	outputDescription.componentSubType = kAudioUnitSubType_RemoteIO;
	outputDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
	
	//add the node to the graph.
	err = AUGraphAddNode(graph, &outputDescription, &outputNode);
	
	//there are three steps, we open the graph, initialise it and start it.
	//when we open it (from the doco) the audio units belonging to the graph are open but not initialized. Specifically, no resource allocation occurs.
	err = AUGraphOpen(graph);
	
	//now that the graph is open we can get the AudioUnits that are in the nodes (or node in this case)
	//get the output AudioUnit from the graph, we supply a node and a description and the graph creates the AudioUnit which
	//we then request back from the graph, so we can set properties on it, such as its audio format
	err = AUGraphNodeInfo(graph, outputNode, &outputDescription, &remoteIOUnit);
	
	// Set up the master fader callback
	AURenderCallbackStruct playbackCallbackStruct;
	playbackCallbackStruct.inputProc = audioOutputCallback;
	//set the reference to "this" this becomes *inRefCon in the playback callback
	//as the callback is just a straight C method this is how we can pass it an this class
	playbackCallbackStruct.inputProcRefCon = this;
	
	//now set the callback on the output node, this callback gets called whenever the outputnode has render called on it
	err = AUGraphSetNodeInputCallback(graph, outputNode, 0, &playbackCallbackStruct);
	
	//lets actually set the audio format
	AudioStreamBasicDescription audioFormat;
	
	// Describe format
	audioFormat.mSampleRate			= 44100.00;
	audioFormat.mFormatID			= kAudioFormatLinearPCM;
	audioFormat.mFormatFlags		= kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsNonInterleaved | kAudioFormatFlagIsPacked;
	audioFormat.mFramesPerPacket	= 1;
	audioFormat.mChannelsPerFrame	= 2;
	audioFormat.mBitsPerChannel		= 16;
	audioFormat.mBytesPerPacket		= 2;
	audioFormat.mBytesPerFrame		= 2;
	
	//set the remoteIOUnit input properties
	err = AudioUnitSetProperty(remoteIOUnit, 
							   kAudioUnitProperty_StreamFormat, 
							   kAudioUnitScope_Input, 
							   0, 
							   &audioFormat, 
							   sizeof(audioFormat));
	
	//we then initiailze the graph
	err = AUGraphInitialize(graph);
	
	//this prints out a description of the graph, showing the nodes and connections, really handy.
	//this shows in the console (Command-Shift-R to see it)
	CAShow(graph); 
	
	//the final step, as soon as this is run, the graph will start requesting samples. some people would put this on the play button
	//but ive found that sometimes i get a bit of a pause so i let the callback get called from the start and only start filling the buffer
	//with samples when the play button is hit.
	//the doco says :
	//this function starts rendering by starting the head node of an audio processing graph. The graph must be initialized before it can be started.
	err = AUGraphStart(graph);	
}

#pragma mark -
#pragma mark Constructor / Destructor

AudioManager::AudioManager(){
	
	InitialiseAudioSession();
	
	inMemoryAudiofile = new InMemoryAudioFile();
	
	mIsRecording = false;
	
	InitialiseAudioGraph();
}

AudioManager::~AudioManager(){

	delete inMemoryAudiofile;
}

#pragma mark -
#pragma mark Recording

OSStatus AudioManager::StartRecord(CFURLRef url, bool aacRecording){
	
	//just in case
	StopRecord();
	
	//setup the format
	CAStreamBasicDescription dstFormat, clientFormat;
	
	memset(&dstFormat, 0, sizeof(CAStreamBasicDescription));	
	memset(&clientFormat, 0, sizeof(CAStreamBasicDescription));	
	
	AudioFileTypeID fileTypeId = kAudioFileWAVEType;
	
	UInt32 size = sizeof(dstFormat);
	//if we can record to AAC
	if (aacRecording){
		dstFormat.mFormatID = kAudioFormatMPEG4AAC;
		dstFormat.mSampleRate = 44100.0;
		dstFormat.mChannelsPerFrame = 2;
		
		// use AudioFormat API to fill out the rest of the description
		size = sizeof(dstFormat);
		AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &size, &dstFormat);
		
		//set the file type Id
		fileTypeId = kAudioFileM4AType;
	}
	//otherwise we set up the format for PCM
	else{
		dstFormat.mFormatID = kAudioFormatLinearPCM;
		
		// setup the output file format
		dstFormat.mSampleRate = 44100.0; // set sample rate
		
		// create a 16-bit 44100kHz Stereo format
		dstFormat.mChannelsPerFrame = 2;
		dstFormat.mBitsPerChannel = 16;
		dstFormat.mBytesPerPacket = dstFormat.mBytesPerFrame = 4;
		dstFormat.mFramesPerPacket = 1;
		dstFormat.mFormatFlags = kLinearPCMFormatFlagIsPacked | kLinearPCMFormatFlagIsSignedInteger; // little-endian		
	}
	
	//get the client format directly from 
	UInt32 asbdSize = sizeof (AudioStreamBasicDescription);
	AudioUnitGetProperty(remoteIOUnit,
						 kAudioUnitProperty_StreamFormat,
						 kAudioUnitScope_Input,
						 0, // input bus
						 &clientFormat,
						 &asbdSize);
	
	printf("Destination recording file format: "); dstFormat.Print();
	printf("Source file format: "); clientFormat.Print();
	
	//open the file for recording
	OSStatus err = ExtAudioFileCreateWithURL(url, fileTypeId, &dstFormat, NULL, kAudioFileFlags_EraseFile, &mRecordFile);
	
	if (err == noErr){
		printf("recording\n");
		ExtAudioFileSetProperty(mRecordFile, kExtAudioFileProperty_ClientDataFormat, size, &clientFormat);
		//call this once as this will alloc space on the first call
		ExtAudioFileWriteAsync(mRecordFile,	0, NULL);
		mIsRecording = true;
	}
	else{		
		printf("Not Recording, Error Code:%d,\n", err);
		//dispose of the file reference
		mRecordFile = NULL;
	}
	return err;
}

void AudioManager::StopRecord(){
	if (mIsRecording){
		// end recording
		mIsRecording = false;
		//TODO: close the file
		ExtAudioFileDispose(mRecordFile);
		//dispose of the file reference
		mRecordFile = NULL;
	}
}
