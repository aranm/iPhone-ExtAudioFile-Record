//
//  GraphRecorderViewController.h
//  GraphRecorder
//
//  Created by Aran Mulholland on 14/10/10.
//  Copyright None 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "AudioManager.h"

@interface GraphRecorderViewController : UIViewController {

	AudioManager *audioManager;
	NSString *recordingFilePathAndName;
}

@property (nonatomic)AudioManager *audioManager;
@property (nonatomic, retain)NSString *recordingFilePathAndName;

-(IBAction)play;
-(IBAction)stopPlayback;

-(IBAction)record;
-(IBAction)stopRecording;
-(IBAction)playbackRecording;


@end

