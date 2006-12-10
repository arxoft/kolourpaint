
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef KP_EFFECT_INVERT_H
#define KP_EFFECT_INVERT_H


#include <kpEffectCommandBase.h>
#include <kpimage.h>


class QCheckBox;
class QImage;
class QPixmap;

class kpMainWindow;


class kpEffectInvert
{
public:
    enum Channel
    {
        None = 0,
        Red = 1, Green = 2, Blue = 4,
        RGB = Red | Green | Blue
    };

    //
    // Inverts the colours of each pixel in the given image.
    // These functions differ from QImage::invertPixels() in the following ways:
    //
    // 1. for 8-bit images, it inverts the colours of the Colour Table
    //    (this means that you would get visually similar results to inversion
    //     at higher bit depths - rather than a "random-looking" inversion
    //     depending on the contents of the Colour Table)
    // 2. never inverts the Alpha Buffer
    //

    static void applyEffect (QPixmap *destPixmapPtr, int channels = RGB);
    static QPixmap applyEffect (const QPixmap &pm, int channels = RGB);
    static void applyEffect (QImage *destImagePtr, int channels = RGB);
    static QImage applyEffect (const QImage &img, int channels = RGB);
};


class kpEffectInvertCommand : public kpEffectCommandBase
{
public:
    kpEffectInvertCommand (int channels,
                           bool actOnSelection,
                           kpMainWindow *mainWindow);
    kpEffectInvertCommand (bool actOnSelection,
                           kpMainWindow *mainWindow);
    virtual ~kpEffectInvertCommand ();


    //
    // kpEffectCommandBase interface
    //

public:
    virtual bool isInvertible () const { return true; }

protected:
    virtual kpImage applyEffect (const kpImage &image);

    int m_channels;
};


#endif  // KP_EFFECT_INVERT_H
