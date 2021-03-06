/*
 Copyright 2015 OpenMarket Ltd
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#import "MXKRoomSettingsViewController.h"

#import "NSBundle+MatrixKit.h"

@interface MXKRoomSettingsViewController()
{    
    // the room events listener
    id roomListener;
    
    // Observe kMXSessionWillLeaveRoomNotification to be notified if the user leaves the current room.
    id leaveRoomNotificationObserver;
}
@end

@implementation MXKRoomSettingsViewController

#pragma mark - Class methods

+ (UINib *)nib
{
    return [UINib nibWithNibName:NSStringFromClass([MXKRoomSettingsViewController class])
                          bundle:[NSBundle bundleForClass:[MXKRoomSettingsViewController class]]];
}

+ (instancetype)roomSettingsViewController
{
    return [[[self class] alloc] initWithNibName:NSStringFromClass([MXKRoomSettingsViewController class])
                                          bundle:[NSBundle bundleForClass:[MXKRoomSettingsViewController class]]];
}

#pragma mark -

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    [self refreshRoomSettings];
}

#pragma mark - Override MXKTableViewController

- (void)destroy
{
    if (roomListener)
    {
        [mxRoom.liveTimeline removeListener:roomListener];
        roomListener = nil;
    }
    
    if (leaveRoomNotificationObserver)
    {
        [[NSNotificationCenter defaultCenter] removeObserver:leaveRoomNotificationObserver];
        leaveRoomNotificationObserver = nil;
    }
    
    mxRoom = nil;
    mxRoomState = nil;
    
    [super destroy];
}

- (void)onMatrixSessionStateDidChange:(NSNotification *)notif;
{
    // Check this is our Matrix session that has changed
    if (notif.object == self.mainSession)
    {
        // refresh when the session sync is done.
        if (MXSessionStateRunning == self.mainSession.state)
        {
            [self refreshRoomSettings];
        }
    }
}

#pragma mark - Public API

/**
 Set the dedicated session and the room Id
 */
- (void)initWithSession:(MXSession*)mxSession andRoomId:(NSString*)roomId
{
    // Update the matrix session
    if (self.mainSession)
    {
        [self removeMatrixSession:self.mainSession];
    }
    mxRoom = nil;
    
    // Sanity checks
    if (mxSession && roomId)
    {
        [self addMatrixSession:mxSession];
        
        // Report the room identifier
        _roomId = roomId;
        mxRoom = [mxSession roomWithRoomId:roomId];
    }
    
    if (mxRoom)
    {
        // Register a listener to handle messages related to room name, topic...
        roomListener = [mxRoom.liveTimeline listenToEventsOfTypes:@[kMXEventTypeStringRoomName, kMXEventTypeStringRoomTopic, kMXEventTypeStringRoomAliases, kMXEventTypeStringRoomAvatar, kMXEventTypeStringRoomPowerLevels, kMXEventTypeStringRoomCanonicalAlias, kMXEventTypeStringRoomJoinRules, kMXEventTypeStringRoomGuestAccess, kMXEventTypeStringRoomHistoryVisibility] onEvent:^(MXEvent *event, MXTimelineDirection direction, MXRoomState *roomState) {
            
            // Consider only live events
            if (direction == MXTimelineDirectionForwards)
            {
                [self updateRoomState:mxRoom.state];
            }
            
        }];
        
        // Observe kMXSessionWillLeaveRoomNotification to be notified if the user leaves the current room.
        leaveRoomNotificationObserver = [[NSNotificationCenter defaultCenter] addObserverForName:kMXSessionWillLeaveRoomNotification object:nil queue:[NSOperationQueue mainQueue] usingBlock:^(NSNotification *notif) {
            
            // Check whether the user will leave the room related to the displayed participants
            if (notif.object == self.mainSession)
            {
                NSString *roomId = notif.userInfo[kMXSessionNotificationRoomIdKey];
                if (roomId && [roomId isEqualToString:_roomId])
                {
                    // We remove the current view controller.
                    [self withdrawViewControllerAnimated:YES completion:nil];
                }
            }
            
        }];
        
        [self updateRoomState:mxRoom.state];
    }
    
    self.title = [NSBundle mxk_localizedStringForKey:@"room_details_title"];
}

- (void)refreshRoomSettings
{
    [self.tableView reloadData];
}

#pragma mark - private methods

- (void)updateRoomState:(MXRoomState*)newRoomState
{
    mxRoomState = newRoomState.copy;
    
    [self refreshRoomSettings];
}

#pragma mark - UITableViewDataSource

// empty by default

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return 0;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return nil;
}

@end
