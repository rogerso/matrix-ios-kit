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

#import "MXKRoomInputToolbarViewWithSimpleTextView.h"

@implementation MXKRoomInputToolbarViewWithSimpleTextView

+ (UINib *)nib
{
    return [UINib nibWithNibName:NSStringFromClass([MXKRoomInputToolbarViewWithSimpleTextView class])
                          bundle:[NSBundle bundleForClass:[MXKRoomInputToolbarViewWithSimpleTextView class]]];
}

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    // Set default message composer background color
    self.messageComposerTextView.backgroundColor = [UIColor whiteColor];
    
    // Add an accessory view to the text view in order to retrieve keyboard view.
    inputAccessoryView = [[UIView alloc] initWithFrame:CGRectZero];
    self.messageComposerTextView.inputAccessoryView = self.inputAccessoryView;
}

- (NSString*)textMessage
{
    return _messageComposerTextView.text;
}

- (void)setTextMessage:(NSString *)textMessage
{
    _messageComposerTextView.text = textMessage;
    self.rightInputToolbarButton.enabled = textMessage.length;
    
    if (!textMessage.length && _messageComposerTextView.isFirstResponder)
    {
        // Trick: Toggle default keyboard from 123 mode to ABC mode when text input is reset
        [_messageComposerTextView resignFirstResponder];
        [_messageComposerTextView becomeFirstResponder];
    }
}

- (BOOL)becomeFirstResponder
{
    return [_messageComposerTextView becomeFirstResponder];
}

- (void)dismissKeyboard
{
    if (_messageComposerTextView)
    {
        [_messageComposerTextView resignFirstResponder];
    }
}

#pragma mark - UITextViewDelegate

- (void)textViewDidBeginEditing:(UITextView *)textView
{
    if ([self.delegate respondsToSelector:@selector(roomInputToolbarView:isTyping:)])
    {
        [self.delegate roomInputToolbarView:self isTyping:YES];
    }
}

- (void)textViewDidEndEditing:(UITextView *)textView
{
    if ([self.delegate respondsToSelector:@selector(roomInputToolbarView:isTyping:)])
    {
        [self.delegate roomInputToolbarView:self isTyping:NO];
    }
}

- (void)textViewDidChange:(UITextView *)textView
{
    NSString *msg = textView.text;
    
    if (msg.length)
    {
        if ([self.delegate respondsToSelector:@selector(roomInputToolbarView:isTyping:)])
        {
            [self.delegate roomInputToolbarView:self isTyping:YES];
        }
        self.rightInputToolbarButton.enabled = YES;
    }
    else
    {
        if ([self.delegate respondsToSelector:@selector(roomInputToolbarView:isTyping:)])
        {
            [self.delegate roomInputToolbarView:self isTyping:NO];
        }
        self.rightInputToolbarButton.enabled = NO;
    }
}

- (BOOL)textView:(UITextView *)textView shouldChangeTextInRange:(NSRange)range replacementText:(NSString *)text
{
    // Hanlde here `Done` key pressed
    if([text isEqualToString:@"\n"])
    {
        [textView resignFirstResponder];
        return NO;
    }
    
    return YES;
}

@end
