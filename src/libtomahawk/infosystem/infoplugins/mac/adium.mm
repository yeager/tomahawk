/* === This file is part of Tomahawk Player - <http://tomahawk-player.org> ===
 *
 *   Copyright 2010-2011, Christian Muehlhaeuser <muesli@tomahawk-player.org>
 *
 *   Tomahawk is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Tomahawk is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Tomahawk. If not, see <http://www.gnu.org/licenses/>.
 */

#import <Foundation/Foundation.h>
#import <ScriptingBridge/ScriptingBridge.h>
#import "adium.h"

void
script( const char* status )
{
    NSString *stat = [NSString stringWithUTF8String:status];
    NSAppleScript *appleScript = [[NSAppleScript alloc] initWithSource:stat];
    NSDictionary *errorDictionary;
    NSAppleEventDescriptor *eventDescriptor = [appleScript executeAndReturnError:&errorDictionary];
    [appleScript release];
}
