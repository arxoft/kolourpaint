
/*
   Copyright (c) 2003-2004 Clarence Dang <dang@kde.org>
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


#include <kptoolwidgetlinewidth.h>

#include <qbitmap.h>
#include <qpainter.h>

#include <klocale.h>


static int lineWidths [] = {1, 2, 3, 5, 8};

kpToolWidgetLineWidth::kpToolWidgetLineWidth (QWidget *parent)
    : kpToolWidgetBase (parent)
{
    setInvertSelectedPixmap ();

    int numLineWidths = sizeof (lineWidths) / sizeof (lineWidths [0]);

    int w = (width () - 2/*margin*/) * 3 / 4;
    int h = (height () - 2/*margin*/ - (numLineWidths - 1)/*spacing*/) * 3 / (numLineWidths * 4);

    for (int i = 0; i < numLineWidths; i++)
    {
        QPixmap pixmap ((w <= 0 ? width () : w),
                        (h <= 0 ? height () : h));
        pixmap.fill (Qt::white);

        QBitmap maskBitmap (pixmap.width (), pixmap.height ());
        maskBitmap.fill (Qt::color0/*transparent*/);
        
        
        QPainter painter (&pixmap), maskPainter (&maskBitmap);
        painter.setPen (Qt::black), maskPainter.setPen (Qt::color1/*opaque*/);
        painter.setBrush (Qt::black), maskPainter.setBrush (Qt::color1/*opaque*/);

        QRect rect = QRect (0, (pixmap.height () - lineWidths [i]) / 2,
                            pixmap.width (), lineWidths [i]);
        painter.drawRect (rect), maskPainter.drawRect (rect);

        painter.end (), maskPainter.end ();
        
        
        pixmap.setMask (maskBitmap);

        addOption (pixmap, QString::number(lineWidths [i]));
        startNewOptionRow ();
    }

    relayoutOptions ();
    setSelected (0, 0);
}

kpToolWidgetLineWidth::~kpToolWidgetLineWidth ()
{
}

int kpToolWidgetLineWidth::lineWidth () const
{
    return lineWidths [selectedRow ()];
}

// virtual protected slot
void kpToolWidgetLineWidth::setSelected (int row, int col)
{
    kpToolWidgetBase::setSelected (row, col);
    emit lineWidthChanged (lineWidth ());
};

#include <kptoolwidgetlinewidth.moc>
