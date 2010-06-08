// ****************************************************************************
//  font_file_manager_macos.mm                                     Tao project
// ****************************************************************************
//
//   File Description:
//
//     MacOS X methods for the FontFileManager class.
//
//
//
//
//
//
//
//
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "font_file_manager.h"

#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFURL.h>

TAO_BEGIN

QString FontFileManager::FontToFile(const QFont &font)
// ----------------------------------------------------------------------------
//   Find the font file that defines the given font
// ----------------------------------------------------------------------------
{
    QString             result;
    CTFontDescriptorRef desc       = NULL;
    CFURLRef            url        = NULL;
    CFStringRef         str        = NULL;
    const int           buffersize = 1024;
    char                buffer[buffersize];

    // 4 string containers for the same value is probably record-breaking ;-)
    QString     qfamily = font.family();
    std::string sfamily = qfamily.toStdString();
    const char *cfamily = sfamily.c_str();
    CFStringRef ffamily = CFStringCreateWithCString(kCFAllocatorDefault, cfamily,
                                                    kCFStringEncodingMacRoman);
    if (!ffamily)
        goto out;

    desc = CTFontDescriptorCreateWithNameAndSize (ffamily, 0.0);
    if (!desc)
        goto out;

    url = (CFURLRef)CTFontDescriptorCopyAttribute(desc, kCTFontURLAttribute);
    if (!url)
        goto out;

    str = CFURLGetString(url);
    if (!str)
        goto out;

    memset(buffer, 0, buffersize);
    CFURLGetFileSystemRepresentation(url, true, (UInt8*)buffer, buffersize - 1);
    result = buffer;

out:
    if (ffamily)
        CFRelease(ffamily);
    if (desc)
        CFRelease(desc);

    return result;
}

TAO_END
