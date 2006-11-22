
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


#define DEBUG_KP_TOOL_COLOR_ERASER 0


#include <kptoolcoloreraser.h>

#include <qbitmap.h>
#include <qimage.h>
#include <qpainter.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>

#include <kpbug.h>
#include <kpcolor.h>
#include <kpdocument.h>
#include <kppainter.h>
#include <kppixmapfx.h>
#include <kptoolflowcommand.h>


kpToolColorEraser::kpToolColorEraser (kpMainWindow *mainWindow)
    : kpToolFlowBase (i18n ("Color Eraser"),
        i18n ("Replaces pixels of the foreground color with the background color"),
        Qt::Key_O,
        mainWindow,
        "tool_color_eraser")
{
}

kpToolColorEraser::~kpToolColorEraser ()
{
}


// public virtual [base kpTool]
void kpToolColorEraser::globalDraw ()
{
#if DEBUG_KP_TOOL_COLOR_ERASER
    kDebug () << "kpToolColorEraser::globalDraw()" << endl;
#endif
    if (!drawShouldProceed (QPoint ()/*unused*/, QPoint ()/*unused*/, QRect ()/*unused*/))
        return;

    QApplication::setOverrideCursor (Qt::WaitCursor);

    kpToolFlowCommand *cmd = new kpToolFlowCommand (
        i18n ("Color Eraser"), mainWindow ());

    const QRect dirtyRect = kpPainter::washRect (document ()->pixmap (),
        0, 0, document ()->width (), document ()->height (),
        backgroundColor ()/*color to draw in*/,
        foregroundColor ()/*color to replace*/,
        processedColorSimilarity ());

    if (!dirtyRect.isEmpty ())
    {
        document ()->slotContentsChanged (dirtyRect);


        cmd->updateBoundingRect (dirtyRect);
        cmd->finalize ();

        commandHistory ()->addCommand (cmd, false /* don't exec */);

        // don't delete - it's up to the commandHistory
        cmd = 0;
    }
    else
    {
    #if DEBUG_KP_TOOL_COLOR_ERASER
        kDebug () << "\tisNOP" << endl;
    #endif
        delete cmd;
        cmd = 0;
    }

    QApplication::restoreOverrideCursor ();
}

QString kpToolColorEraser::haventBegunDrawUserMessage () const
{
    return i18n ("Click or drag to erase pixels of the foreground color.");
}


bool kpToolColorEraser::drawShouldProceed (const QPoint & /*thisPoint*/,
    const QPoint & /*lastPoint*/,
    const QRect & /*normalizedRect*/)
{
    if (foregroundColor () == backgroundColor () &&
        processedColorSimilarity () == 0)
    {
        return false;
    }
    
    return true;
}


QRect kpToolColorEraser::drawLine (const QPoint &thisPoint, const QPoint &lastPoint)
{
#if DEBUG_KP_TOOL_COLOR_ERASER
    kDebug () << "kpToolColorEraser::drawLine(thisPoint=" << thisPoint
        << ",lastPoint=" << lastPoint << ")" << endl;
#endif

    const QRect dirtyRect = kpPainter::washLine (document ()->pixmap (),
        lastPoint.x (), lastPoint.y (),
        thisPoint.x (), thisPoint.y (),
        color (mouseButton ())/*color to draw in*/,
        brushWidth (), brushHeight (),
        color (1 - mouseButton ())/*color to replace*/,
        processedColorSimilarity ());

#if DEBUG_KP_TOOL_COLOR_ERASER
    kDebug () << "\tdirtyRect=" << dirtyRect << endl;
#endif

    if (!dirtyRect.isEmpty ())
    {
        document ()->slotContentsChanged (dirtyRect);
        return dirtyRect;
    }
    
    return QRect ();
}

#include <kptoolcoloreraser.moc>
