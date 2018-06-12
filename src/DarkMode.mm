/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#import "DarkMode.h"

void updateDarkModeStateForTreeStartingAtView(__kindof NSView *rootView, bool vibrant = false)
{
    for (NSView *view in rootView.subviews) {
        if( vibrant ) {
            view.appearance = [NSAppearance appearanceNamed:NSAppearanceNameVibrantDark];
        }
        if ([view isKindOfClass:[NSVisualEffectView class]]) {
            [(NSVisualEffectView *)view setMaterial:NSVisualEffectMaterialDark];
        }
        
        if ([view isKindOfClass:[NSClipView class]] ||
            [view isKindOfClass:[NSScrollView class]] ||
            [view isKindOfClass:[NSMatrix class]] ||
            [view isKindOfClass:[NSTextView class]] ||
            [view isKindOfClass:NSClassFromString(@"TBrowserTableView")] ||
            [view isKindOfClass:NSClassFromString(@"TIconView")]) {
            [view performSelector:@selector(setBackgroundColor:) withObject:[NSColor colorWithCalibratedWhite:0.1 alpha:1.0]];
        }
        
        if (view.subviews.count > 0)
            updateDarkModeStateForTreeStartingAtView(view, vibrant);
    }
}

void applyDarkAppearanceToWindow (NSWindow *window, bool vibrant, bool flat, bool subviews)
{
    if( vibrant ) {
        [window setAppearance:[NSAppearance appearanceNamed:NSAppearanceNameVibrantDark]];
    }
    if( flat ) {
        window.titlebarAppearsTransparent = true; // gives it "flat" look
    }
    [window setBackgroundColor:[NSColor blackColor]];
    if( subviews ) {
        updateDarkModeStateForTreeStartingAtView(window.contentView, vibrant);
    }
}
