#include "axpch.h"
#include "Utils/FileDialogs.h"

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

namespace Axiom {
    std::optional<std::filesystem::path> FileDialogs::openFolder(const std::string& title) {
        @autoreleasepool {
          NSOpenPanel* panel = [NSOpenPanel openPanel];

          [panel setCanChooseFiles:NO];
          [panel setCanChooseDirectories:YES];
          [panel setAllowsMultipleSelection:NO];
          [panel setMessage:[NSString stringWithUTF8String:title.c_str()]];

          if ([panel runModal] == NSModalResponseOK) {
            NSURL* url = [[panel URLs] objectAtIndex:0];
            const char* utf8Path = [[url path] UTF8String];
            return std::filesystem::path(utf8Path);
          }
        }
        return std::nullopt;
    }

    std::optional<std::filesystem::path> FileDialogs::openFile(const std::string& title, const std::string& filter) {
        @autoreleasepool {
            NSOpenPanel* panel = [NSOpenPanel openPanel];

            [panel setCanChooseFiles:YES];
            [panel setCanChooseDirectories:NO];
            [panel setAllowsMultipleSelection:NO];
            [panel setMessage:[NSString stringWithUTF8String:title.c_str()]];

            if (!filter.empty()) {
                NSString* nsFilter = [NSString stringWithUTF8String:filter.c_str()];
                [panel setAllowedFileTypes:@[nsFilter]];
            }

            if ([panel runModal] == NSModalResponseOK) {
                NSURL* url = [[panel URLs] objectAtIndex:0];
                const char* utf8Path = [[url path] UTF8String];
                return std::filesystem::path(utf8Path);
            }
        }
        return std::nullopt;
    }
}
