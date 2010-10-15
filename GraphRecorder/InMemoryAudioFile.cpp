//
//  InMemoryAudioFile.m
//  HelloWorld
//
//  Created by Aran Mulholland on 22/02/09.
//  Copyright 2009 Aran Mulholland. All rights reserved.
//

#include "InMemoryAudioFile.h"
#include <AudioUnit/AudioUnit.h>

//overide init method
InMemoryAudioFile::InMemoryAudioFile(){ 
	isPlaying = false;
    mSoundBuffer.leftData = NULL;
    mSoundBuffer.rightData = NULL;
	mSoundBuffer.numFrames = 0;
	mSoundBuffer.sampleNum = 0;
}

InMemoryAudioFile::~InMemoryAudioFile(){ 
	if (mSoundBuffer.leftData != NULL){
		free(mSoundBuffer.leftData);
	}
	if (mSoundBuffer.rightData != NULL){
		free(mSoundBuffer.rightData);
	}
}

void InMemoryAudioFile::play(){
	isPlaying = true;
	mSoundBuffer.sampleNum = 0;
}

void InMemoryAudioFile::stop(){
	isPlaying = false;
}

// load up audio data from the demo files into mSoundBuffer.data used in the render proc
void InMemoryAudioFile::loadFile(CFURLRef sourceURL)
{
	ExtAudioFileRef xafref = 0;
	
	// open one of the two source files
	OSStatus result = ExtAudioFileOpenURL(sourceURL, &xafref);
	if (result || !xafref) { printf("ExtAudioFileOpenURL result %d %08X %4.4s\n", result, result, (char*)&result); return; }
	
	// get the file data format, this represents the file's actual data format
	CAStreamBasicDescription clientFormat;
	UInt32 propSize = sizeof(clientFormat);
	
	result = ExtAudioFileGetProperty(xafref, kExtAudioFileProperty_FileDataFormat, &propSize, &clientFormat);
	if (result) { printf("ExtAudioFileGetProperty kExtAudioFileProperty_FileDataFormat result %d %08X %4.4s\n", result, result, (char*)&result); return; }
	
	// set the client format to be what we want back
	double rateRatio = 44100.0 / clientFormat.mSampleRate;
	clientFormat.mSampleRate = 44100.0;
	clientFormat.SetCanonical(2, false);
	
	propSize = sizeof(clientFormat);
	result = ExtAudioFileSetProperty(xafref, kExtAudioFileProperty_ClientDataFormat, propSize, &clientFormat);
	if (result) { printf("ExtAudioFileSetProperty kExtAudioFileProperty_ClientDataFormat %d %08X %4.4s\n", result, result, (char*)&result); return; }
	
	// get the file's length in sample frames
	UInt64 numFrames = 0;
	propSize = sizeof(numFrames);
	result = ExtAudioFileGetProperty(xafref, kExtAudioFileProperty_FileLengthFrames, &propSize, &numFrames);
	if (result) { printf("ExtAudioFileGetProperty kExtAudioFileProperty_FileLengthFrames result %d %08X %4.4s\n", result, result, (char*)&result); return; }
	
	numFrames = (UInt32)(numFrames * rateRatio); // account for any sample rate conversion
	
	// set up our buffer
	mSoundBuffer.numFrames = numFrames;
	mSoundBuffer.asbd = clientFormat;
	mSoundBuffer.asbd.mChannelsPerFrame = 1;
	
	UInt32 samples = numFrames * mSoundBuffer.asbd.mChannelsPerFrame;
	mSoundBuffer.leftData = (AudioSampleType *)calloc(samples, sizeof(AudioSampleType));
	mSoundBuffer.rightData = (AudioSampleType *)calloc(samples, sizeof(AudioSampleType));
	mSoundBuffer.sampleNum = 0;
	
	// set up a AudioBufferList to read data into
	AudioBufferList *bufList;
    bufList = (AudioBufferList*)calloc(1, sizeof(AudioBufferList) + 2 * sizeof(AudioBuffer));
    bufList->mNumberBuffers = 2;
    bufList->mBuffers[0].mNumberChannels = 1;
    bufList->mBuffers[0].mDataByteSize = samples * sizeof(AudioSampleType);
    bufList->mBuffers[0].mData = mSoundBuffer.leftData;
	bufList->mBuffers[1].mNumberChannels = 1;
    bufList->mBuffers[1].mDataByteSize = samples * sizeof(AudioSampleType);
    bufList->mBuffers[1].mData = mSoundBuffer.rightData;
	
	// perform a synchronous sequential read of the audio data out of the file into our allocated data buffer
	UInt32 numPackets = numFrames;
	result = ExtAudioFileRead(xafref, &numPackets, bufList);
	if (result) {
		printf("ExtAudioFileRead result %d %08X %4.4s\n", result, result, (char*)&result); 
		free(mSoundBuffer.leftData);
		free(mSoundBuffer.rightData);
		mSoundBuffer.leftData = 0;
		mSoundBuffer.rightData = 0;
		return;
	}
//	else{
//		for (int i = 0; i < numPackets; i++){
//			printf("%d\n", ((AudioSampleType *)mSoundBuffer.leftData)[i]);
//		}
//	}
	
	//get rid of the buffer list
	free(bufList); 
	
	// close the file and dispose the ExtAudioFileRef
	ExtAudioFileDispose(xafref);

}

OSStatus InMemoryAudioFile::FillBuffer(AudioBufferList * ioData, UInt32 inNumberFrames){

	if (isPlaying == false) { 
		//zero out any buffers
		for (int i = 0 ; i < ioData->mNumberBuffers; i++){
			AudioBuffer buffer = ioData->mBuffers[i];
			//fill the buffer with zeros
			memset((Byte *)buffer.mData, 0, buffer.mDataByteSize);
		}
	}
	else{	
		AudioSampleType *leftBuffer = (AudioSampleType *)ioData->mBuffers[0].mData;
		AudioSampleType *rightBuffer = (AudioSampleType *)ioData->mBuffers[1].mData;
		AudioSampleType *leftSoundBuffer = (AudioSampleType *)mSoundBuffer.leftData;
		AudioSampleType *rightSoundBuffer = (AudioSampleType *)mSoundBuffer.rightData;
		
		for (int i = 0; i < inNumberFrames; i++) {
			leftBuffer[i] = leftSoundBuffer[mSoundBuffer.sampleNum];
			rightBuffer[i] = rightSoundBuffer[mSoundBuffer.sampleNum];	
			//printf("%d\n", leftBuffer[i]);
			mSoundBuffer.sampleNum++;
			
			if (mSoundBuffer.sampleNum >= mSoundBuffer.numFrames){
				mSoundBuffer.sampleNum = 0;
			}
		}
	}
	return noErr;
}

void InMemoryAudioFile::reset(){
	mSoundBuffer.sampleNum = 0;
}
