
/* This file is part of the KolourPaint project
   Copyright (c) 2003 Clarence Dang <dang@kde.org>
   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the names of the copyright holders nor the names of
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define DEBUG_KP_COMMAND_HISTORY 1

#include <kdebug.h>

#include <kpcommandhistory.h>
#include <kpmainwindow.h>
#include <kptool.h>


kpCommandHistory::kpCommandHistory (kpMainWindow *mainWindow)
    : KCommandHistory (mainWindow->actionCollection (), true/*menus in toolbar*/),
      m_mainWindow (mainWindow)
{
}

kpCommandHistory::~kpCommandHistory ()
{
}


// public slot virtual [base KCommandHistory]
void kpCommandHistory::undo ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "kpCommandHistory::undo() CALLED!" << endl;
#endif
    if (m_mainWindow && m_mainWindow->toolHasBegunShape ())
    {
    #if DEBUG_KP_COMMAND_HISTORY
        kdDebug () << "\thas begun shape - cancel draw" << endl;
    #endif
        m_mainWindow->tool ()->cancelShapeInternal ();
    }
    else
        KCommandHistory::undo ();
}

// public slot virtual [base KCommandHistory]
void kpCommandHistory::redo ()
{
    if (m_mainWindow && m_mainWindow->toolHasBegunShape ())
    {
        // Not completely obvious but what else can we do?
        //
        // Ignoring the request would not be intuitive for tools like
        // Polygon & Polyline (where it's not always apparent to the user
        // that s/he's still drawing a shape even though the mouse isn't
        // down).
        m_mainWindow->tool ()->cancelShapeInternal ();
    }
    else
        KCommandHistory::redo ();
}

