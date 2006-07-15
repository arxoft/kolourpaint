
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


#define DEBUG_KP_FLOOD_FILL 1


#include <kpfloodfill.h>

#include <QApplication>
#include <QImage>
#include <QLinkedList>
#include <QList>
#include <QPainter>

#include <kdebug.h>

#include <kpcolor.h>
#include <kpdefs.h>
#include <kpimage.h>
#include <kppixmapfx.h>
#include <kptool.h>


struct kpFillLine
{
    kpFillLine (int y = -1, int x1 = -1, int x2 = -1)
        : m_y (y), m_x1 (x1), m_x2 (x2)
    {
    }

    static int size ()
    {
        return sizeof (kpFillLine);
    }

    int m_y, m_x1, m_x2;
};

static int FillLinesListSize (const QLinkedList <kpFillLine> &fillLines)
{
    return (fillLines.size () * kpFillLine::size ());
}
    

struct kpFloodFillPrivate
{
    //
    // Copy of whatever was passed to the constructor.
    //
    
    kpImage *imagePtr;
    int x, y;
    kpColor color;
    int processedColorSimilarity;


    //
    // Set by Step 1.
    //
    
    kpColor colorToChange;


    //
    // Set by Step 2.
    //
    
    QImage readableImage;
    
    QLinkedList <kpFillLine> fillLines;
    QList < QLinkedList <kpFillLine> > fillLinesCache;

    QRect boundingRect;
    
    bool prepared;
};

kpFloodFill::kpFloodFill (kpImage *image, int x, int y,
                         const kpColor &color, int processedColorSimilarity)
    : d (new kpFloodFillPrivate ())
{
    d->imagePtr = image;
    d->x = x, d->y = y;
    d->color = color, d->processedColorSimilarity = processedColorSimilarity;

    d->prepared = false;
}

kpFloodFill::~kpFloodFill ()
{
    delete d;
}


// public
int kpFloodFill::size () const
{
    int fillLinesCacheSize = 0;
    foreach (const QLinkedList <kpFillLine> linesList, d->fillLinesCache)
    {
        fillLinesCacheSize += ::FillLinesListSize (linesList);
    }
    
    return ::FillLinesListSize (d->fillLines) +
           kpPixmapFX::imageSize (d->readableImage) +
           fillLinesCacheSize;
}
    
// public
kpColor kpFloodFill::color () const
{
    return d->color;
}

// public
int kpFloodFill::processedColorSimilarity () const
{
    return d->processedColorSimilarity;
}


QRect kpFloodFill::boundingRect ()
{
    prepare ();
    
    return d->boundingRect;
}




struct DrawLinesPackage
{
    const QLinkedList <kpFillLine> *lines;
    kpColor color;
};

static void DrawLinesHelper (QPainter *p,
        bool drawingOnRGBLayer,
        void *data)
{
    const DrawLinesPackage *pack = static_cast <DrawLinesPackage *> (data);

#if DEBUG_KP_FLOOD_FILL
    kDebug () << "DrawLinesHelper() lines"
        << " color=" << (int *) pack->color.toQRgb ()
        << endl;
#endif

    p->setPen (kpPixmapFX::draw_ToQColor (pack->color, drawingOnRGBLayer));
            
    foreach (const kpFillLine l, *pack->lines)
    {
        const QPoint p1 (l.m_x1, l.m_y);
        const QPoint p2 (l.m_x2, l.m_y);

        p->drawLine (p1, p2);
    }
}

static void DrawLines (kpImage *image,
        const QLinkedList <kpFillLine> &lines,
        const kpColor &color)
{
    DrawLinesPackage pack;
    pack.lines = &lines;
    pack.color = color;

    kpPixmapFX::draw (image, &::DrawLinesHelper,
        color.isOpaque (), color.isTransparent (),
        &pack);
}

void kpFloodFill::fill ()
{
    prepare ();


    QApplication::setOverrideCursor (Qt::WaitCursor);

    ::DrawLines (d->imagePtr, d->fillLines, d->color);

    QApplication::restoreOverrideCursor ();
}

kpColor kpFloodFill::colorToChange ()
{
    prepareColorToChange ();
    
    return d->colorToChange;
}

void kpFloodFill::prepareColorToChange ()
{
#if DEBUG_KP_FLOOD_FILL && 1
    kDebug () << "kpFloodFill::prepareColorToChange" << endl;
#endif
    if (d->colorToChange.isValid ())
        return;

    d->colorToChange = kpPixmapFX::getColorAtPixel (*d->imagePtr, QPoint (d->x, d->y));

    if (d->colorToChange.isOpaque ())
    {
    #if DEBUG_KP_FLOOD_FILL && 1
        kDebug () << "\tcolorToChange: r=" << d->colorToChange.red ()
                   << ", b=" << d->colorToChange.blue ()
                   << ", g=" << d->colorToChange.green ()
                   << endl;
    #endif
    }
    else
    {
    #if DEBUG_KP_FLOOD_FILL && 1
        kDebug () << "\tcolorToChange: transparent" << endl;
    #endif
    }
}

// Derived from the zSprite2 Graphics Engine

void kpFloodFill::prepare ()
{
#if DEBUG_KP_FLOOD_FILL && 1
    kDebug () << "kpFloodFill::prepare()" << endl;
#endif
    if (d->prepared)
        return;
        
    prepareColorToChange ();
        
    d->boundingRect = QRect ();


#if DEBUG_KP_FLOOD_FILL && 1
    kDebug () << "\tperforming NOP check" << endl;
#endif

    // get the color we need to replace
    if (d->processedColorSimilarity == 0 && d->color == d->colorToChange)
    {
        // need to do absolutely nothing (this is a significant optimisation
        // for people who randomly click a lot over already-filled areas)
        d->prepared = true;  // sync with all "return true"'s
        return;
    }

#if DEBUG_KP_FLOOD_FILL && 1
    kDebug () << "\tconverting to image" << endl;
#endif

    // The only way to read pixels.  Sigh.
    d->readableImage = kpPixmapFX::convertToImage (*d->imagePtr);

#if DEBUG_KP_FLOOD_FILL && 1
    kDebug () << "\tcreating fillLinesCache" << endl;
#endif

    // ready cache
    for (int i = 0; i < d->imagePtr->height (); i++)
         d->fillLinesCache.append (QLinkedList <kpFillLine> ());

#if DEBUG_KP_FLOOD_FILL && 1
    kDebug () << "\tcreating fill lines" << endl;
#endif

    // draw initial line
    addLine (d->y, findMinX (d->y, d->x), findMaxX (d->y, d->x));

    for (QLinkedList <kpFillLine>::ConstIterator it = d->fillLines.begin ();
         it != d->fillLines.end ();
         it++)
    {
    #if DEBUG_KP_FLOOD_FILL && 0
        kDebug () << "Expanding from y=" << (*it).m_y
                   << " x1=" << (*it).m_x1
                   << " x2=" << (*it).m_x2
                   << endl;
    #endif

        //
        // Make more lines above and below current line.
        //
        // WARNING: Adds to end of "fillLines" (the linked list we are iterating
        //          through).  Therefore, "fillLines" must remain a linked list
        //          - you cannot change it into a vector.  Also, do not use
        //          "foreach" for this loop as that makes a copy of the linked
        //          list at the start and won't see new lines.
        //
        findAndAddLines (*it, -1);
        findAndAddLines (*it, +1);
    }

#if DEBUG_KP_FLOOD_FILL && 1
    kDebug () << "\tfinalising memory usage" << endl;
#endif

    // finalize memory usage
    d->readableImage = QImage ();
    d->fillLinesCache.clear ();

    d->prepared = true;  // sync with all "return true"'s
}

void kpFloodFill::addLine (int y, int x1, int x2)
{
#if DEBUG_KP_FLOOD_FILL && 0
    kDebug () << "kpFillCommand::fillAddLine (" << y << "," << x1 << "," << x2 << ")" << endl;
#endif

    d->fillLines.append (kpFillLine (y, x1, x2));
    d->fillLinesCache [y].append (kpFillLine (y /* OPT */, x1, x2));
    d->boundingRect = d->boundingRect.unite (QRect (QPoint (x1, y), QPoint (x2, y)));
}

kpColor kpFloodFill::pixelColor (int x, int y, bool *beenHere) const
{
    if (beenHere)
        *beenHere = false;

    if (y >= (int) d->fillLinesCache.count ())
    {
        kError () << "kpFloodFill::pixelColor("
                   << x << ","
                   << y << ") y out of range=" << d->imagePtr->height () << endl;
        return kpColor::invalid;
    }

    foreach (const kpFillLine line, d->fillLinesCache [y])
    {
        if (x >= line.m_x1 && x <= line.m_x2)
        {
            if (beenHere)
                *beenHere = true;
            return d->color;
        }
    }

    return kpPixmapFX::getColorAtPixel (d->readableImage, QPoint (x, y));
}

bool kpFloodFill::shouldGoTo (int x, int y) const
{
    bool beenThere;
    const kpColor col = pixelColor (x, y, &beenThere);

    return (!beenThere && col.isSimilarTo (d->colorToChange, d->processedColorSimilarity));
}

void kpFloodFill::findAndAddLines (const kpFillLine &fillLine, int dy)
{
    // out of bounds?
    if (fillLine.m_y + dy < 0 || fillLine.m_y + dy >= d->imagePtr->height ())
        return;

    for (int xnow = fillLine.m_x1; xnow <= fillLine.m_x2; xnow++)
    {
        // At current position, right colour?
        if (shouldGoTo (xnow, fillLine.m_y + dy))
        {
            // Find minimum and maximum x values
            int minxnow = findMinX (fillLine.m_y + dy, xnow);
            int maxxnow = findMaxX (fillLine.m_y + dy, xnow);

            // Draw line
            addLine (fillLine.m_y + dy, minxnow, maxxnow);

            // Move x pointer
            xnow = maxxnow;
        }
    }
}

// finds the minimum x value at a certain line to be filled
int kpFloodFill::findMinX (int y, int x) const
{
    for (;;)
    {
        if (x < 0)
            return 0;

        if (shouldGoTo (x, y))
            x--;
        else
            return x + 1;
    }
}

// finds the maximum x value at a certain line to be filled
int kpFloodFill::findMaxX (int y, int x) const
{
    for (;;)
    {
        if (x > d->imagePtr->width () - 1)
            return d->imagePtr->width () - 1;

        if (shouldGoTo (x, y))
            x++;
        else
            return x - 1;
    }
}
