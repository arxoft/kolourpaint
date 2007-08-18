
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
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

#define DEBUG_KP_TOOL_TEXT 1


#include <kpToolText.h>
#include <kpToolTextPrivate.h>

#include <QList>

#include <KLocale>

#include <kpDocument.h>
#include <kpTextSelection.h>
#include <kpToolSelectionEnvironment.h>
#include <kpViewManager.h>


// protected virtual [kpAbstractSelectionTool]
QString kpToolText::haventBegunDrawUserMessageCreate () const
{
    return i18n ("Left drag to create text box.");
}


// protected virtual [base kpAbstractSelectionTool]
void kpToolText::setSelectionBorderForBeginDrawCreate ()
{
    viewManager ()->setQueueUpdates ();
    {
        kpAbstractSelectionTool::setSelectionBorderForBeginDrawCreate ();
        viewManager ()->setTextCursorEnabled (false);
    }
    viewManager ()->restoreQueueUpdates ();
}


// private
int kpToolText::calcClickCreateDimension (int mouseStart, int mouseEnd,
    int preferredMin, int smallestMin,
    int docSize)
{
    Q_ASSERT (preferredMin >= smallestMin);
    Q_ASSERT (docSize > 0);

    // Get reasonable width/height for a text box.
    int ret = preferredMin;

    // X or Y increasing?
    if (mouseEnd >= mouseStart)
    {
        // Text box extends past document width/height?
        if (mouseStart + ret - 1 >= docSize)
        {
            // Cap width/height to not extend past but not below smallest
            // possible selection width/height
            ret = qMax (smallestMin, docSize - mouseStart);
        }
    }
    // X or Y decreasing
    else
    {
        // Text box extends past document start?
        // TODO: I doubt this code can be invoked for a click.
        //       Maybe very tricky interplay with accidental drag detection?
        if (mouseStart - ret + 1 < 0)
        {
            // Cap width/height to not extend past but not below smallest
            // possible selection width/height.
            ret = qMax (smallestMin, mouseStart + 1);
        }
    }

    return ret;
}

// private
bool kpToolText::shouldCreate (bool dragHasBegun,
        const QPoint &accidentalDragAdjustedPoint,
        const kpTextStyle &textStyle,
        int *minimumWidthOut, int *minimumHeightOut,
        bool *newDragHasBegun)
{
    *newDragHasBegun = dragHasBegun;

    // Is the drag so short that we're essentially just clicking?
    // Basically, we're trying to prevent unintentional creation of 1-pixel
    // selections.
    if (!dragHasBegun && accidentalDragAdjustedPoint == startPoint ())
    {
        // We had an existing text box before the click?
        if (hadSelectionBeforeDraw ())
        {
        #if DEBUG_KP_TOOL_TEXT && 1
            kDebug () << "\ttext box deselect - NOP - return" << endl;
        #endif
            // We must be attempting to deselect the text box.
            // This deselection has already been done by kpAbstractSelectionTool::beginDraw().
            // Therefore, we are not doing a drag.
            return false;
        }
        // We must be creating a new box.
        else
        {
            // This drag is currently a click - not a drag.
            //
            // However, as a special case, allow user to create a text box using a single
            // click.  But don't set newDragHasBegun since it would be untrue.
            //
            // This makes sure that a single click creation of text box
            // works even if draw() is invoked more than once at the
            // same position (esp. with accidental drag suppression
            // (above)).

            //
            // User is possibly clicking to create a text box.
            //
            // Create a text box of reasonable ("preferred minimum") size.
            //
            // However, if it turns out that this is just the start of the drag,
            // we will be called again but the above code will execute instead,
            // ignoring this resonable size.
            //

        #if DEBUG_KP_TOOL_TEXT && 1
            kDebug () << "\tclick creating text box" << endl;
        #endif

            // (Click creating text box with RMB would not be obvious
            //  since RMB menu most likely hides text box immediately
            //  afterwards)
            // TODO: I suspect this logic is simply too late
            // TODO: We setUserShapePoints() on return but didn't before.
            if (mouseButton () == 1)
                return false/*do not create text box*/;


            // Calculate suggested width.
            *minimumWidthOut = calcClickCreateDimension (
                startPoint ().x (),
                    accidentalDragAdjustedPoint.x (),
                kpTextSelection::PreferredMinimumWidthForTextStyle (textStyle),
                    kpTextSelection::MinimumWidthForTextStyle (textStyle),
                document ()->width ());

            // Calculate suggested height.
            *minimumHeightOut = calcClickCreateDimension (
                startPoint ().y (),
                    accidentalDragAdjustedPoint.y (),
                kpTextSelection::PreferredMinimumHeightForTextStyle (textStyle),
                    kpTextSelection::MinimumHeightForTextStyle (textStyle),
                document ()->height ());


            return true/*do create text box*/;
        }
    }
    else
    {
    #if DEBUG_KP_TOOL_TEXT && 1
        kDebug () << "\tdrag creating text box" << endl;
    #endif
        *minimumWidthOut = kpTextSelection::MinimumWidthForTextStyle (textStyle);
        *minimumHeightOut = kpTextSelection::MinimumHeightForTextStyle (textStyle);
        *newDragHasBegun = true;
        return true/*do create text box*/;
    }

}

// protected virtual [kpAbstractSelectionTool]
bool kpToolText::drawCreateMoreSelectionAndUpdateStatusBar (
        bool dragHasBegun,
        const QPoint &accidentalDragAdjustedPoint,
        const QRect &normalizedRectIn)
{
    // (will mutate this)
    QRect normalizedRect = normalizedRectIn;

    const kpTextStyle textStyle = environ ()->textStyle ();


    //
    // Calculate Text Box Rectangle.
    //

    bool newDragHasBegun = dragHasBegun;

    // (will set both variables)
    int minimumWidth = 0, minimumHeight = 0;
    if (!shouldCreate (dragHasBegun, accidentalDragAdjustedPoint, textStyle,
            &minimumWidth, &minimumHeight, &newDragHasBegun))
    {
        setUserShapePoints (accidentalDragAdjustedPoint);
        return newDragHasBegun;
    }


    // Make sure the dragged out rectangle is of the minimum width we just
    // calculated.
    if (normalizedRect.width () < minimumWidth)
    {
        if (accidentalDragAdjustedPoint.x () >= startPoint ().x ())
            normalizedRect.setWidth (minimumWidth);
        else
            normalizedRect.setX (normalizedRect.right () - minimumWidth + 1);
    }

    // Make sure the dragged out rectangle is of the minimum height we just
    // calculated.
    if (normalizedRect.height () < minimumHeight)
    {
        if (accidentalDragAdjustedPoint.y () >= startPoint ().y ())
            normalizedRect.setHeight (minimumHeight);
        else
            normalizedRect.setY (normalizedRect.bottom () - minimumHeight + 1);
    }

#if DEBUG_KP_TOOL_TEXT && 1
    kDebug () << "\t\tnormalizedRect=" << normalizedRect
                << " kpTextSelection::preferredMinimumSize="
                    << QSize (minimumWidth, minimumHeight)
                << endl;
#endif


    //
    // Construct and Deploy Text Box.
    //

    // Create empty text box.
    QList <QString> textLines;
    kpTextSelection textSel (normalizedRect, textLines, textStyle);

    // Render.
    viewManager ()->setTextCursorPosition (0, 0);
    document ()->setSelection (textSel);


    //
    // Update Status Bar.
    //

    QPoint actualEndPoint = KP_INVALID_POINT;
    if (startPoint () == normalizedRect.topLeft ())
        actualEndPoint = normalizedRect.bottomRight ();
    else if (startPoint () == normalizedRect.bottomRight ())
        actualEndPoint = normalizedRect.topLeft ();
    else if (startPoint () == normalizedRect.topRight ())
        actualEndPoint = normalizedRect.bottomLeft ();
    else if (startPoint () == normalizedRect.bottomLeft ())
        actualEndPoint = normalizedRect.topRight ();

    setUserShapePoints (startPoint (), actualEndPoint);

    return newDragHasBegun;
}

