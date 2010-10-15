//
//  GraphRecorderViewController.m
//  GraphRecorder
//
//  Created by Aran Mulholland on 14/10/10.
//  Copyright None 2010. All rights reserved.
//

#import "GraphRecorderViewController.h"

@implementation GraphRecorderViewController
@synthesize audioManager, recordingFilePathAndName;


-(IBAction)play{
	if (audioManager->inMemoryAudiofile->IsPlaying()){ }
	else{
		audioManager->inMemoryAudiofile->play();
	}
}

-(IBAction)stopPlayback{
	
	if (audioManager->inMemoryAudiofile->IsPlaying()){
		audioManager->inMemoryAudiofile->stop();
	}
}

static Boolean IsAACHardwareEncoderAvailable(void)
{
    Boolean isAvailable = false;
	
    // get an array of AudioClassDescriptions for all installed encoders for the given format 
    // the specifier is the format that we are interested in - this is 'aac ' in our case
    UInt32 encoderSpecifier = kAudioFormatMPEG4AAC;
    UInt32 size;
	
    OSStatus result = AudioFormatGetPropertyInfo(kAudioFormatProperty_Encoders, sizeof(encoderSpecifier), &encoderSpecifier, &size);
    if (result) { printf("AudioFormatGetPropertyInfo kAudioFormatProperty_Encoders result %lu %4.4s\n", result, (char*)&result); return false; }
	
    UInt32 numEncoders = size / sizeof(AudioClassDescription);
    AudioClassDescription encoderDescriptions[numEncoders];
    
    result = AudioFormatGetProperty(kAudioFormatProperty_Encoders, sizeof(encoderSpecifier), &encoderSpecifier, &size, encoderDescriptions);
    if (result) { printf("AudioFormatGetProperty kAudioFormatProperty_Encoders result %lu %4.4s\n", result, (char*)&result); return false; }
    
    for (UInt32 i=0; i < numEncoders; ++i) {
        if (encoderDescriptions[i].mSubType == kAudioFormatMPEG4AAC && encoderDescriptions[i].mManufacturer == kAppleHardwareAudioCodecManufacturer) isAvailable = true;
    }
	
    return isAvailable;
}

- (NSString *)applicationDocumentsDirectory {	
	return [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
}

-(IBAction)record{
	
	BOOL aacEncoding = IsAACHardwareEncoderAvailable();
	
	NSString *completeFileNameAndPath = [[self applicationDocumentsDirectory] stringByAppendingPathComponent:[[NSProcessInfo processInfo] globallyUniqueString]];
	
	//if we can encode to AAC
	if (IsAACHardwareEncoderAvailable()){
		completeFileNameAndPath = [completeFileNameAndPath stringByAppendingString:@".m4a"];
	}
	//we record as straight WAV
	else{
		completeFileNameAndPath = [completeFileNameAndPath stringByAppendingString:@".wav"];
	}
	
	//create the url that the recording object needs to reference the file
	CFURLRef audioFileURL = CFURLCreateFromFileSystemRepresentation (NULL, (const UInt8 *)[completeFileNameAndPath cStringUsingEncoding:[NSString defaultCStringEncoding]] , strlen([completeFileNameAndPath cStringUsingEncoding:[NSString defaultCStringEncoding]]), false);		
	
	//start the recording, this will stop the old recording if it is going on.
	if (audioManager->StartRecord(audioFileURL, aacEncoding) == noErr){
		[self setRecordingFilePathAndName:completeFileNameAndPath];
	}
	else{
		[self setRecordingFilePathAndName:nil];  
	}
}

-(IBAction)stopRecording{
	if (audioManager->GetIsRecording()){
		audioManager->StopRecord();
	}
}

-(IBAction)playbackRecording{
	
}
	

/*
// The designated initializer. Override to perform setup that is required before the view is loaded.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
        // Custom initialization
    }
    return self;
}
*/

/*
// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView {
}
*/


/*
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
}
*/


/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}


- (void)dealloc {
    [super dealloc];
}

@end
