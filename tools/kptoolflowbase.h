
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


#ifndef KP_TOOL_FLOW_BASE_H
#define KP_TOOL_FLOW_BASE_H


#include <qlist.h>
#include <qpixmap.h>
#include <qrect.h>

#include <kpcommandhistory.h>
#include <kptemppixmap.h>
#include <kptool.h>


class QPoint;
class QString;

class kpColor;
class kpMainWindow;
class kpToolFlowCommand;
class kpToolWidgetBrush;
class kpToolWidgetEraserSize;
class kpViewManager;


class kpToolFlowBase : public kpTool
{
Q_OBJECT

public:
    kpToolFlowBase (const QString &text, const QString &description,
        int key,
        kpMainWindow *mainWindow, const QString &name);
    virtual ~kpToolFlowBase ();

private:
    void clearBrushCursorData ();
    
protected:
    virtual QString haventBegunDrawUserMessage () const = 0;
    
    virtual bool haveSquareBrushes () const { return false; }
    virtual bool haveDiverseBrushes () const { return false; }    
    bool haveAnyBrushes () const
    {
        return (haveSquareBrushes () || haveDiverseBrushes ());
    }
    
    virtual bool colorsAreSwapped () const { return false; }
    
public:
    virtual void begin ();
    virtual void end ();

    virtual void beginDraw ();
    virtual void hover (const QPoint &point);
    
    // TODO: should be moved to kpPainter as that encapsulates all document drawing ops
    QList <QPoint> interpolatePoints (const QPoint &thisPoint,
        const QPoint &lastPoint,
        double probability = 1.0) KDE_DEPRECATED;

    // TODO: should be removed as all drawing operations should use kpPainter,
    //       not manually play with QPixmap's and masks.
    void drawLineSetupPainterMask (QPixmap *pixmap,
        QBitmap *maskBitmap,
        QPainter *painter, QPainter *maskPainter) KDE_DEPRECATED;
    // TODO: should be removed as all drawing operations should use kpPainter,
    //       not manually play with QPixmap's and masks.
    void drawLineTearDownPainterMask (QPixmap *pixmap,
        const QBitmap *maskBitmap,
        QPainter *painter, QPainter *maskPainter,
        bool drawingHappened = true) KDE_DEPRECATED;

    // drawPoint() normally calls drawLine(point,point).  Override drawPoint()
    // if you think you can be more efficient.
    virtual QRect drawPoint (const QPoint &point);
    virtual QRect drawLine (const QPoint &thisPoint, const QPoint &lastPoint) = 0;

    virtual bool drawShouldProceed (const QPoint & /*thisPoint*/, const QPoint & /*lastPoint*/, const QRect & /*normalizedRect*/) { return true; }
    virtual void draw (const QPoint &thisPoint, const QPoint &lastPoint, const QRect &normalizedRect);
    virtual void cancelShape ();
    virtual void releasedAllButtons ();
    virtual void endDraw (const QPoint &, const QRect &);

protected:
    kpTempPixmap::UserFunctionType brushDrawFunction () const;
    void *brushDrawFunctionData () const;
    
    int brushWidth () const;
    int brushHeight () const;

    kpToolFlowCommand *currentCommand () const;

protected slots:
    void updateBrushAndCursor ();

    virtual void slotForegroundColorChanged (const kpColor &col);
    virtual void slotBackgroundColorChanged (const kpColor &col);

protected:
    virtual kpColor color (int which);

    QPoint hotPoint () const;
    QPoint hotPoint (int x, int y) const;
    QPoint hotPoint (const QPoint &point) const;
    QRect hotRect () const;
    QRect hotRect (int x, int y) const;
    QRect hotRect (const QPoint &point) const;

private:
    struct kpToolFlowBasePrivate *d;
};


#endif  // KP_TOOL_FLOW_BASE_H
