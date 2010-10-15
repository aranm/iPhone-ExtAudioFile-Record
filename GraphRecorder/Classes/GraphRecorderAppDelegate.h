//
//  GraphRecorderAppDelegate.h
//  GraphRecorder
//
//  Created by Aran Mulholland on 14/10/10.
//  Copyright None 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#include "AudioManager.h"

@class GraphRecorderViewController;

@interface GraphRecorderAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    GraphRecorderViewController *viewController;
	AudioManager *audioManager;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet GraphRecorderViewController *viewController;

@end

