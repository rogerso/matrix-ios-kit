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

#import <UIKit/UIKit.h>

#import "MXKDataSource.h"
#import "MXKRoomBubbleCellDataStoring.h"
#import "MXKEventFormatter.h"


/**
 Define the threshold which triggers a bubbles count flush.
 */
#define MXKROOMDATASOURCE_CACHED_BUBBLES_COUNT_THRESHOLD 30

/**
 Define the number of messages to preload around the initial event.
 */
#define MXKROOMDATASOURCE_PAGINATION_LIMIT_AROUND_INITIAL_EVENT 30

/**
 List the supported pagination of the rendered room bubble cells
 */
typedef enum : NSUInteger
{
    /**
     No pagination
     */
    MXKRoomDataSourceBubblesPaginationNone,
    /**
     The rendered room bubble cells are paginated per day
     */
    MXKRoomDataSourceBubblesPaginationPerDay
    
} MXKRoomDataSourceBubblesPagination;


#pragma mark - Cells identifiers

/**
 String identifying the object used to store and prepare room bubble data.
 */
extern NSString *const kMXKRoomBubbleCellDataIdentifier;


#pragma mark - Notifications
/**
 Posted when an information about the room has changed.
 Tracked informations are: lastMessage, hasUnread, notificationCount, highlightCount.
 The notification object is the `MXKRoomDataSource` instance.
 */
extern NSString *const kMXKRoomDataSourceMetaDataChanged;

/**
 Posted when a server sync starts or ends (depend on 'serverSyncEventCount').
 The notification object is the `MXKRoomDataSource` instance.
 */
extern NSString *const kMXKRoomDataSourceSyncStatusChanged;

#pragma mark - MXKRoomDataSource
@protocol MXKRoomBubbleCellDataStoring;

/**
 The data source for `MXKRoomViewController`.
 */
@interface MXKRoomDataSource : MXKDataSource <UITableViewDataSource>
{
@protected

    /**
     The data for the cells served by `MXKRoomDataSource`.
     */
    NSMutableArray *bubbles;

    /**
     The queue of events that need to be processed in order to compute their display.
     */
    NSMutableArray *eventsToProcess;
}

/**
 The id of the room managed by the data source.
 */
@property (nonatomic, readonly) NSString *roomId;

/**
 The room the data comes from.
 The object is defined when the MXSession has data for the room
 */
@property (nonatomic, readonly) MXRoom *room;

/**
 The timeline being managed. It can be the live timeline of the room
 or a timeline from a past event, initialEventId.
 */
@property (nonatomic, readonly) MXEventTimeline *timeline;

/**
 Flag indicating if the data source manages, or will manage, a live timeline.
 */
@property (nonatomic, readonly) BOOL isLive;

/**
 Flag indicating if the data source is used to peek into a room, ie it gets data from
 a room the user has not joined yet.
 */
@property (nonatomic, readonly) BOOL isPeeking;

/**
 The last event in the room that matches the `eventsFilterForMessages` property.
 */
@property (nonatomic, readonly) MXEvent *lastMessage;

/**
 The list of the attachments with thumbnail in the current available bubbles (MXKAttachment instances).
 */
@property (nonatomic, readonly) NSArray *attachmentsWithThumbnail;

/**
 Tell whether the room has unread messages.
 */
@property (nonatomic, readonly) BOOL hasUnread;

/**
 The number of unread messages that match the push notification rules.
 */
@property (nonatomic, readonly) NSUInteger notificationCount;

/**
 The number of highlighted unread messages (subset of notifications).
 */
@property (nonatomic, readonly) NSUInteger highlightCount;

/**
 The events are processed asynchronously. This property counts the number of queued events
 during server sync for which the process is pending.
 */
@property (nonatomic, readonly) NSInteger serverSyncEventCount;

/**
 The current text message partially typed in text input (use nil to reset it).
 */
@property (nonatomic) NSString *partialTextMessage;


#pragma mark - Configuration
/**
 The type of events to display as messages.
 */
@property (nonatomic) NSArray *eventsFilterForMessages;

/**
 The events to display texts formatter.
 `MXKRoomBubbleCellDataStoring` instances can use it to format text.
 */
@property (nonatomic) MXKEventFormatter *eventFormatter;

/**
 Show the date time label in rendered room bubble cells. NO by default.
 */
@property (nonatomic) BOOL showBubblesDateTime;

/**
 A Boolean value that determines whether the date time labels are customized (By default date time display is handled by MatrixKit). NO by default.
 */
@property (nonatomic) BOOL useCustomDateTimeLabel;

/**
 Show the receipts in rendered bubble cell. YES by default.
 */
@property (nonatomic) BOOL showBubbleReceipts;

/**
 A Boolean value that determines whether the read receipts are customized (By default read receipts display is handled by MatrixKit). NO by default.
 */
@property (nonatomic) BOOL useCustomReceipts;

/**
 A Boolean value that determines whether the unsent button is customized (By default an 'Unsent' button is displayed by MatrixKit in front of unsent events). NO by default.
 */
@property (nonatomic) BOOL useCustomUnsentButton;

/**
 Show the typing notifications of other room members in the chat history (YES by default).
 */
@property (nonatomic) BOOL showTypingNotifications;

/**
 The pagination applied on the rendered room bubble cells (MXKRoomDataSourceBubblesPaginationNone by default).
 */
@property (nonatomic) MXKRoomDataSourceBubblesPagination bubblesPagination;

/**
 Max nbr of cached bubbles when there is no delegate.
 The default value is 30.
 */
@property (nonatomic) unsigned long maxBackgroundCachedBubblesCount;

/**
 The number of messages to preload around the initial event.
 The default value is 30.
 */
@property (nonatomic) NSUInteger paginationLimitAroundInitialEvent;

#pragma mark - Life cycle
/**
 Initialise the data source to serve data corresponding to the passed room.
 
 @param roomId the id of the room to get data from.
 @param mxSession the Matrix session to get data from.
 @return the newly created instance.
 */
- (instancetype)initWithRoomId:(NSString*)roomId andMatrixSession:(MXSession*)mxSession;

/**
 Initialise the data source to serve data corresponding to an event in the
 past of a room.

 @param roomId the id of the room to get data from.
 @param initialEventId the id of the event where to start the timeline.
 @param mxSession the Matrix session to get data from.
 @return the newly created instance.
 */
- (instancetype)initWithRoomId:(NSString*)roomId initialEventId:(NSString*)initialEventId andMatrixSession:(MXSession*)mxSession;

/**
 Initialise the data source to peek into a room.
 
 The data source will close the `peekingRoom` instance on [self destroy].

 @param peekingRoom the room to peek.
 @param initialEventId the id of the event where to start the timeline. nil means the live
                       timeline.
 @return the newly created instance.
 */
- (instancetype)initWithPeekingRoom:(MXPeekingRoom*)peekingRoom andInitialEventId:(NSString*)initialEventId;

/**
 Mark all messages as read
 */
- (void)markAllAsRead;

/**
 Reduce memory usage by releasing room data if the number of bubbles is over the provided limit 'maxBubbleNb'.
 
 This operation is ignored if some local echoes are pending or if unread messages counter is not nil.
 
 @param maxBubbleNb The room bubble data are released only if the number of bubbles is over this limit.
 */
- (void)limitMemoryUsage:(NSInteger)maxBubbleNb;

/**
 Force data reload.
 */
- (void)reload;

#pragma mark - Public methods
/**
 Get the data for the cell at the given index.

 @param index the index of the cell in the array
 @return the cell data
 */
- (id<MXKRoomBubbleCellDataStoring>)cellDataAtIndex:(NSInteger)index;

/**
 Get the data for the cell which contains the event with the provided event id.

 @param eventId the event identifier
 @return the cell data
 */
- (id<MXKRoomBubbleCellDataStoring>)cellDataOfEventWithEventId:(NSString*)eventId;

/**
 Get the index of the cell which contains the event with the provided event id.

 @param eventId the event identifier
 @return the index of the concerned cell (NSNotFound if none).
 */
- (NSInteger)indexOfCellDataWithEventId:(NSString *)eventId;

/**
 Get height of the cell at the given index.

 @param index the index of the cell in the array.
 @param maxWidth the maximum available width.
 @return the cell height (0 if no data is available for this cell, or if the delegate is undefined).
 */
- (CGFloat)cellHeightAtIndex:(NSInteger)index withMaximumWidth:(CGFloat)maxWidth;

#pragma mark - Pagination
/**
 Load more messages.
 This method fails (with nil error) if the data source is not ready (see `MXKDataSourceStateReady`).
 
 @param numItems the number of items to get.
 @param direction backwards or forwards.
 @param onlyFromStore if YES, return available events from the store, do not make a pagination request to the homeserver.
 @param success a block called when the operation succeeds. This block returns the number of added cells.
 (Note this count may be 0 if paginated messages have been concatenated to the current first cell).
 @param failure a block called when the operation fails.
 */
- (void)paginate:(NSUInteger)numItems direction:(MXTimelineDirection)direction onlyFromStore:(BOOL)onlyFromStore success:(void (^)(NSUInteger addedCellNumber))success failure:(void (^)(NSError *error))failure;

/**
 Load enough messages to fill the rect.
 This method fails (with nil error) if the data source is not ready (see `MXKDataSourceStateReady`),
 or if the delegate is undefined (this delegate is required to compute the actual size of the cells).
 
 @param rect the rect to fill.
 @param direction backwards or forwards.
 @param minRequestMessagesCount if messages are not available in the store, a request to the homeserver
        is required. minRequestMessagesCount indicates the minimum messages count to retrieve from the hs.
 @param success a block called when the operation succeeds.
 @param failure a block called when the operation fails.
 */
- (void)paginateToFillRect:(CGRect)rect  direction:(MXTimelineDirection)direction withMinRequestMessagesCount:(NSUInteger)minRequestMessagesCount success:(void (^)())success failure:(void (^)(NSError *error))failure;


#pragma mark - Sending
/**
 Send a text message to the room.
 
 While sending, a fake event will be echoed in the messages list.
 Once complete, this local echo will be replaced by the event saved by the homeserver.

 @param text the text to send.
 @param success A block object called when the operation succeeds. It returns
                the event id of the event generated on the home server
 @param failure A block object called when the operation fails.
 */
- (void)sendTextMessage:(NSString*)text
                success:(void (^)(NSString *eventId))success
                failure:(void (^)(NSError *error))failure;

/**
 Send an image to the room.

 While sending, a fake event will be echoed in the messages list.
 Once complete, this local echo will be replaced by the event saved by the homeserver.

 @param image the UIImage containing the image to send.
 @param success A block object called when the operation succeeds. It returns
                the event id of the event generated on the home server
 @param failure A block object called when the operation fails.
 */
- (void)sendImage:(UIImage*)image
          success:(void (^)(NSString *eventId))success
          failure:(void (^)(NSError *error))failure;

/**
 Send an image to the room.

 While sending, a fake event will be echoed in the messages list.
 Once complete, this local echo will be replaced by the event saved by the homeserver.

 @param image the UIImage containing the image to send.
 @param body A textual representation of the image.
 @param success A block object called when the operation succeeds. It returns
 the event id of the event generated on the home server
 @param failure A block object called when the operation fails.
 */
- (void)sendImage:(UIImage*)image
             body:(NSString *)body
          success:(void (^)(NSString *eventId))success
          failure:(void (^)(NSError *error))failure;

/**
 Send an image to the room.
 
 While sending, a fake event will be echoed in the messages list.
 Once complete, this local echo will be replaced by the event saved by the homeserver.
 
 @param imageLocalURL the local filesystem path of the image to send.
 @param mimeType the mime type of the image
 @param success A block object called when the operation succeeds. It returns
 the event id of the event generated on the home server
 @param failure A block object called when the operation fails.
 */
- (void)sendImage:(NSURL *)imageLocalURL mimeType:(NSString*)mimetype success:(void (^)(NSString *))success failure:(void (^)(NSError *))failure;

/**
 Send an image to the room.

 While sending, a fake event will be echoed in the messages list.
 Once complete, this local echo will be replaced by the event saved by the homeserver.

 @param imageLocalURL the local filesystem path of the image to send.
 @param mimeType the mime type of the image
 @param body A textual representation of the image.
 @param success A block object called when the operation succeeds. It returns
 the event id of the event generated on the home server
 @param failure A block object called when the operation fails.
 */
- (void)sendImage:(NSURL *)imageLocalURL mimeType:(NSString*)mimetype body:(NSString *)body success:(void (^)(NSString *))success failure:(void (^)(NSError *))failure;

/**
 Send an video to the room.

 While sending, a fake event will be echoed in the messages list.
 Once complete, this local echo will be replaced by the event saved by the homeserver.

 @param videoLocalURL the local filesystem path of the video to send.
 @param videoThumbnail the UIImage hosting a video thumbnail.
 @param success A block object called when the operation succeeds. It returns
                the event id of the event generated on the home server
 @param failure A block object called when the operation fails.
 */
- (void)sendVideo:(NSURL*)videoLocalURL
    withThumbnail:(UIImage*)videoThumbnail
          success:(void (^)(NSString *eventId))success
          failure:(void (^)(NSError *error))failure;

/**
 Send an video to the room.

 While sending, a fake event will be echoed in the messages list.
 Once complete, this local echo will be replaced by the event saved by the homeserver.

 @param videoLocalURL the local filesystem path of the video to send.
 @param videoThumbnail the UIImage hosting a video thumbnail.
 @param body A textual representation of the image.
 @param success A block object called when the operation succeeds. It returns
 the event id of the event generated on the home server
 @param failure A block object called when the operation fails.
 */
- (void)sendVideo:(NSURL*)videoLocalURL
    withThumbnail:(UIImage*)videoThumbnail
             body:(NSString *)body
          success:(void (^)(NSString *eventId))success
          failure:(void (^)(NSError *error))failure;

/**
 Send a file to the room.
 
 While sending, a fake event will be echoed in the messages list.
 Once complete, this local echo will be replaced by the event saved by the homeserver.
 
 @param fileLocalURL the local filesystem path of the file to send.
 @param mimeType the mime type of the file.
 @param success A block object called when the operation succeeds. It returns
 the event id of the event generated on the home server
 @param failure A block object called when the operation fails.
 */
- (void)sendFile:(NSURL*)fileLocalURL
        mimeType:(NSString*)mimeType
          success:(void (^)(NSString *eventId))success
          failure:(void (^)(NSError *error))failure;

/**
 Send a file to the room.

 While sending, a fake event will be echoed in the messages list.
 Once complete, this local echo will be replaced by the event saved by the homeserver.

 @param fileLocalURL the local filesystem path of the file to send.
 @param mimeType the mime type of the file.
 @param body A textual representation of the image.
 @param success A block object called when the operation succeeds. It returns
 the event id of the event generated on the home server
 @param failure A block object called when the operation fails.
 */
- (void)sendFile:(NSURL*)fileLocalURL
        mimeType:(NSString*)mimeType
            body:(NSString *)body
         success:(void (^)(NSString *eventId))success
         failure:(void (^)(NSError *error))failure;

/**
 Send a room message to a room.
 
 While sending, a fake event will be echoed in the messages list.
 Once complete, this local echo will be replaced by the event saved by the homeserver.

 @param msgType the type of the message. @see MXMessageType.
 @param content the message content that will be sent to the server as a JSON object.
 @param success A block object called when the operation succeeds. It returns
                the event id of the event generated on the home server
 @param failure A block object called when the operation fails.
 */
- (void)sendMessageOfType:(MXMessageType)msgType
                  content:(NSDictionary*)content
                  success:(void (^)(NSString *eventId))success
                  failure:(void (^)(NSError *error))failure;

/**
 Resend a room message event.
 
 The echo message corresponding to the event will be removed and a new echo message
 will be added at the end of the room history.

 @param the id of the event to resend.
 @param success A block object called when the operation succeeds. It returns
                the event id of the event generated on the home server
 @param failure A block object called when the operation fails.
 */
- (void)resendEventWithEventId:(NSString*)eventId
                  success:(void (^)(NSString *eventId))success
                  failure:(void (^)(NSError *error))failure;


#pragma mark - Events management
/**
 Get an event loaded in this room datasource.

 @param the id of the event to retrieve.
 @return the MXEvent object or nil if not found.
 */
- (MXEvent *)eventWithEventId:(NSString *)eventId;

/**
 Remove an event from the events loaded by room datasource.

 @param the id of the event to remove.
 */
- (void)removeEventWithEventId:(NSString *)eventId;

/**
 This method is called to handle each read receipt event which is received in forward mode.
 
 You should not call this method directly.
 You may override it in inherited 'MXKRoomDataSource' class.
 
 @param receiptEvent an event with 'm.receipt' type.
 @param roomState the room state right before the event
 */
- (void)didReceiveReceiptEvent:(MXEvent *)receiptEvent roomState:(MXRoomState *)roomState;

/**
 Overridable method to customise the way how unsent messages are managed.
 By default, they are added to the end of the timeline.
 */
- (void)handleUnsentMessages;

@end
