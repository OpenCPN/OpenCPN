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

void setAppLevelDarkMode(bool enabled)
{
    if (@available(macOS 10.14, *)) {
        NSAppearance *appearance = (enabled ? [NSAppearance appearanceNamed:NSAppearanceNameDarkAqua] : nil);
        [[NSApplication sharedApplication] setAppearance:appearance];
    }
}

bool isDarkMode()
{
    // Discussiopn here:
    // https://stackoverflow.com/questions/51672124/how-can-dark-mode-be-detected-on-macos-10-14

    NSAppearance *appearance = NSApp.mainWindow.effectiveAppearance;
    if (@available(macOS 10.14, *)) {
      NSAppearanceName basicAppearance = [appearance bestMatchFromAppearancesWithNames:@[
            NSAppearanceNameAqua,
            NSAppearanceNameDarkAqua
        ]];
      return [basicAppearance isEqualToString:NSAppearanceNameDarkAqua];
    }

    return false;
}

void setWindowLevelDarkMode(NSWindow *window, bool enabled)
{
    NSAppearance *appearance = (enabled ? [NSAppearance appearanceNamed:NSAppearanceNameVibrantDark] : nil);
    [window setAppearance: appearance];
}
