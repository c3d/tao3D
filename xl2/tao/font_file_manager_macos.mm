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

static void AddFontFileFromCTFontDescriptor(const void *value, void *context)
// ----------------------------------------------------------------------------
//   Callback used in FilesForFontFamily. Adds a single font file to a list.
// ----------------------------------------------------------------------------
{
    CTFontDescriptorRef desc       = (CTFontDescriptorRef)value;
    QStringList *       list       = (QStringList*)context;
    CFURLRef            url        = NULL;
    CFStringRef         str        = NULL;
    const int           buffersize = 1024;
    char                buffer[buffersize];

    url = (CFURLRef)CTFontDescriptorCopyAttribute(desc, kCTFontURLAttribute);
    if (!url)
        return;

    str = CFURLGetString(url);
    if (!str)
        return;

    memset(buffer, 0, buffersize);
    CFURLGetFileSystemRepresentation(url, true, (UInt8*)buffer, buffersize-1);

    *list << (QString)buffer;
}

QStringList FontFileManager::FilesForFontFamily(const QString &family)
// ----------------------------------------------------------------------------
//   Find the font files that define the given font family (Windows version)
// ----------------------------------------------------------------------------
{
    QStringList            result;
    CFArrayRef             matchings  = NULL;
    CFMutableDictionaryRef attr       = NULL;
    CTFontDescriptorRef    desc       = NULL;
    CFRange                range;

    // 4 string containers for the same value is probably record-breaking ;-)
    std::string sfamily = family.toStdString();
    const char *cfamily = sfamily.c_str();
    CFStringRef ffamily = CFStringCreateWithCString(kCFAllocatorDefault,
                                                    cfamily,
                                                    kCFStringEncodingMacRoman);
    if (!ffamily)
        goto out;

    attr =  CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks);

    if (!attr)
        goto out;

    CFDictionaryAddValue(attr, kCTFontFamilyNameAttribute, ffamily);

    desc = CTFontDescriptorCreateWithAttributes(attr);
    if (!desc)
        goto out;

    matchings = CTFontDescriptorCreateMatchingFontDescriptors(desc, NULL);
    if (!matchings)
        goto out;

    range.location = 0;
    range.length = CFArrayGetCount(matchings);
    CFArrayApplyFunction(matchings, range, AddFontFileFromCTFontDescriptor,
                         &result);

out:
    if (ffamily)
        CFRelease(ffamily);
    if (attr)
        CFRelease(attr);
    if (desc)
        CFRelease(desc);
    if (matchings)
        CFRelease(matchings);

    return result;
}

TAO_END
