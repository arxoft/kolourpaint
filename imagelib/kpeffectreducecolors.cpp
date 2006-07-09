
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


#define DEBUG_KP_EFFECT_REDUCE_COLORS 0


#include <kpeffectreducecolors.h>

#include <qbitmap.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qimage.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qradiobutton.h>

#include <kdebug.h>
#include <klocale.h>

#include <kppixmapfx.h>


static QImage::Format DepthToFormat (int depth)
{
    switch (depth)
    {
    case 1:
        // (can be MSB instead, I suppose)
        return QImage::Format_MonoLSB;
    
    case 8:
        return QImage::Format_Indexed8;

    case 24:
        // TODO: return RGB32 if no alpha.
        return QImage::Format_ARGB32;

    default:
        kError () << "kpeffectreducecolors.cpp:DepthToFormat(depth="
                  << depth << ")" << endl;
        return QImage::Format_Invalid;
    }
}

// exported
QImage ConvertImageDepth (const QImage &image, int depth, bool dither)
{
#if DEBUG_KP_EFFECT_REDUCE_COLORS
    kDebug () << "kpeffectreducecolors.cpp:ConvertImageDepth() changing image (w=" << image.width ()
               << ",h=" << image.height ()
               << ") depth from " << image.depth ()
                << " to " << depth
                << " (dither=" << dither << ")"
                << endl;
#endif

    if (image.isNull ())
        return image;

    if (depth == image.depth ())
        return image;


#if DEBUG_KP_EFFECT_REDUCE_COLORS && 0
    for (int y = 0; y < image.height (); y++)
    {
        for (int x = 0; x < image.width (); x++)
        {
            fprintf (stderr, " %08X", image.pixel (x, y));
        }
        fprintf (stderr, "\n");
    }
#endif


    // Hack around Qt's braindead QImage::convertDepth(1, ...) (with
    // dithering off) which produces pathetic results with an image that
    // only has 2 colours - sometimes it just gives a completely black
    // result.  Instead, we simply preserve the 2 colours.  One use case
    // is resaving a "colour monochrome" image (<= 2 colours but not
    // necessarily black & white).
    // TODO: still true for Qt4? and QImage::convertToFormat()?
    if (depth == 1 && !dither)
    {
    #if DEBUG_KP_EFFECT_REDUCE_COLORS
        kDebug () << "\tinvoking convert-to-depth 1 hack" << endl;
    #endif
        QRgb color0 = 0, color1 = 0;
        bool color0Valid = false, color1Valid = false;

        bool moreThan2Colors = false;

        // COMPAT: test new ctor
        QImage monoImage (image.width (), image.height (), QImage::Format_MonoLSB);
        monoImage.setNumColors (2);
    #if DEBUG_KP_EFFECT_REDUCE_COLORS
        kDebug () << "\t\tinitialising output image w=" << monoImage.width ()
                   << ",h=" << monoImage.height ()
                   << ",d=" << monoImage.depth ()
                   << endl;
    #endif
        for (int y = 0; y < image.height (); y++)
        {
            for (int x = 0; x < image.width (); x++)
            {
                // (this can be transparent)
                QRgb imagePixel = image.pixel (x, y);

                if (color0Valid && imagePixel == color0)
                    monoImage.setPixel (x, y, 0);
                else if (color1Valid && imagePixel == color1)
                    monoImage.setPixel (x, y, 1);
                else if (!color0Valid)
                {
                    color0 = imagePixel;
                    color0Valid = true;
                    monoImage.setPixel (x, y, 0);
                #if DEBUG_KP_EFFECT_REDUCE_COLORS
                    kDebug () << "\t\t\tcolor0=" << (int *) color0
                               << " at x=" << x << ",y=" << y << endl;
                #endif
                }
                else if (!color1Valid)
                {
                    color1 = imagePixel;
                    color1Valid = true;
                    monoImage.setPixel (x, y, 1);
                #if DEBUG_KP_EFFECT_REDUCE_COLORS
                    kDebug () << "\t\t\tcolor1=" << (int *) color1
                               << " at x=" << x << ",y=" << y << endl;
                #endif
                }
                else
                {
                #if DEBUG_KP_EFFECT_REDUCE_COLORS
                    kDebug () << "\t\t\timagePixel=" << (int *) imagePixel
                               << " at x=" << x << ",y=" << y
                               << " moreThan2Colors - abort hack" << endl;
                #endif
                    moreThan2Colors = true;

                    // Dijkstra, this is clearer than double break'ing or
                    // a check in both loops
                    goto exit_loop;
                }
            }
        }
    exit_loop:

        if (!moreThan2Colors)
        {
            monoImage.setColor (0, color0Valid ? color0 : 0xFFFFFF);
            monoImage.setColor (1, color1Valid ? color1 : 0x000000);
            return monoImage;
        }
    }


    QImage retImage = image.convertToFormat (::DepthToFormat (depth),
        Qt::AutoColor |
        (dither ? Qt::DiffuseDither : Qt::ThresholdDither) |
        Qt::ThresholdAlphaDither |
        (dither ? Qt::PreferDither : Qt::AvoidDither));

#if DEBUG_KP_EFFECT_REDUCE_COLORS && 0
    kDebug () << "After colour reduction:" << endl;
    for (int y = 0; y < image.height (); y++)
    {
        for (int x = 0; x < image.width (); x++)
        {
            fprintf (stderr, " %08X", image.pixel (x, y));
        }
        fprintf (stderr, "\n");
    }
#endif

    return retImage;
}


//
// kpEffectReduceColorsCommand
//

kpEffectReduceColorsCommand::kpEffectReduceColorsCommand (int depth, bool dither,
                                                          bool actOnSelection,
                                                          kpMainWindow *mainWindow)
    : kpColorEffectCommand (commandName (depth, dither), actOnSelection, mainWindow),
      m_depth (depth), m_dither (dither)
{
}

kpEffectReduceColorsCommand::~kpEffectReduceColorsCommand ()
{
}


// public
QString kpEffectReduceColorsCommand::commandName (int depth, int dither) const
{
    if (depth == 1)
    {
        if (dither)
            return i18n ("Reduce to Monochrome (Dithered)");
        else
            return i18n ("Reduce to Monochrome");
    }
    else if (depth == 8)
    {
        if (dither)
            return i18n ("Reduce to 256 Color (Dithered)");
        else
            return i18n ("Reduce to 256 Color");
    }
    else
    {
        return QString::null;
    }
}


// public static
void kpEffectReduceColorsCommand::apply (QPixmap *destPixmapPtr, int depth, bool dither)
{
    if (!destPixmapPtr)
        return;

    if (depth != 1 && depth != 8)
        return;


    QImage image = kpPixmapFX::convertToImage (*destPixmapPtr);


    image = ::ConvertImageDepth (image, depth, dither);

    if (image.isNull ())
        return;


    QPixmap pixmap = kpPixmapFX::convertToPixmap (image, false/*no dither*/);


    // HACK: The above "image.convertDepth()" erases the Alpha Channel
    //       (at least for monochrome).
    //       qpixmap.html says "alpha masks on monochrome images are ignored."
    //
    //       Put the mask back.
    //
    if (!destPixmapPtr->mask ().isNull())
        pixmap.setMask (destPixmapPtr->mask ());

    *destPixmapPtr = pixmap;
}

// public static
QPixmap kpEffectReduceColorsCommand::apply (const QPixmap &pm, int depth, bool dither)
{
    QPixmap ret = pm;
    apply (&ret, depth, dither);
    return ret;
}


//
// kpEffectReduceColorsCommand implements kpColorEffectCommand interface
//

// protected virtual [base kpColorEffectCommand]
QPixmap kpEffectReduceColorsCommand::applyColorEffect (const QPixmap &pixmap)
{
    return apply (pixmap, m_depth, m_dither);
}


//
// kpEffectReduceColorsWidget
//

kpEffectReduceColorsWidget::kpEffectReduceColorsWidget (bool actOnSelection,
                                                        kpMainWindow *mainWindow,
                                                        QWidget *parent )
    : kpColorEffectWidget (actOnSelection, mainWindow, parent)
{
    QVBoxLayout *lay = new QVBoxLayout (this);
    lay->setSpacing(spacingHint ());
    lay->setMargin(marginHint ());


    m_blackAndWhiteRadioButton =
        new QRadioButton (i18n ("&Monochrome"), this);

    m_blackAndWhiteDitheredRadioButton =
        new QRadioButton (i18n ("Mo&nochrome (dithered)"), this);

    m_8BitRadioButton = new QRadioButton (i18n ("256 co&lor"), this);

    m_8BitDitheredRadioButton = new QRadioButton (i18n ("256 colo&r (dithered)"), this);

    m_24BitRadioButton = new QRadioButton (i18n ("24-&bit color"), this);


    // COMPAT: don't think this is needed
    QButtonGroup *buttonGroup = new QButtonGroup (this);
    buttonGroup->addButton (m_blackAndWhiteRadioButton);
    buttonGroup->addButton (m_blackAndWhiteDitheredRadioButton);
    buttonGroup->addButton (m_8BitRadioButton);
    buttonGroup->addButton (m_8BitDitheredRadioButton);
    buttonGroup->addButton (m_24BitRadioButton);


    const int screenDepth = QPixmap::defaultDepth ();
#if DEBUG_KP_EFFECT_REDUCE_COLORS
    kDebug () << "kpEffectReduceColorsWidget::<ctor> screenDepth="
               << screenDepth
               << endl;
#endif

    // Note that everything is disabled for a 1-bit screen since there
    // would be no effect.  I won't support 2-bit or 4-bit screens either :)
    m_blackAndWhiteRadioButton->setEnabled (screenDepth >= 8);
    m_blackAndWhiteDitheredRadioButton->setEnabled (screenDepth >= 8);
    m_8BitRadioButton->setEnabled (screenDepth >= 8);
    // (not enabled if screenDepth==8 as m_8BitRadioButton already serves
    //  as NOP default)
    m_8BitDitheredRadioButton->setEnabled (screenDepth > 8);
    // (not "screenDepth >= 24" as we need a NOP default for 15/16-bit
    //  screens)
    m_24BitRadioButton->setEnabled (screenDepth > 8);


    m_defaultRadioButton = 0;

    if (m_24BitRadioButton->isEnabled ())
    {
    #if DEBUG_KP_EFFECT_REDUCE_COLORS
        kDebug () << "\tdefault is 24-bit button" << endl;
    #endif
        m_defaultRadioButton = m_24BitRadioButton;
    }
    else if (m_8BitRadioButton->isEnabled ())
    {
    #if DEBUG_KP_EFFECT_REDUCE_COLORS
        kDebug () << "\tdefault is 8-bit button" << endl;
    #endif
        m_defaultRadioButton = m_8BitRadioButton;
    }
    else
    {
    #if DEBUG_KP_EFFECT_REDUCE_COLORS
        kDebug () << "\tuser must have a 1-bit screen - no default" << endl;
    #endif
    }


    if (m_defaultRadioButton)
        m_defaultRadioButton->setChecked (true);


    lay->addWidget (m_blackAndWhiteRadioButton);
    lay->addWidget (m_blackAndWhiteDitheredRadioButton);
    lay->addWidget (m_8BitRadioButton);
    lay->addWidget (m_8BitDitheredRadioButton);
    lay->addWidget (m_24BitRadioButton);


    connect (m_blackAndWhiteRadioButton, SIGNAL (toggled (bool)),
             this, SIGNAL (settingsChanged ()));
    connect (m_blackAndWhiteDitheredRadioButton, SIGNAL (toggled (bool)),
             this, SIGNAL (settingsChanged ()));
    connect (m_8BitRadioButton, SIGNAL (toggled (bool)),
             this, SIGNAL (settingsChanged ()));
    connect (m_8BitDitheredRadioButton, SIGNAL (toggled (bool)),
             this, SIGNAL (settingsChanged ()));
    connect (m_24BitRadioButton, SIGNAL (toggled (bool)),
             this, SIGNAL (settingsChanged ()));
}

kpEffectReduceColorsWidget::~kpEffectReduceColorsWidget ()
{
}


// public
int kpEffectReduceColorsWidget::depth () const
{
    if (m_blackAndWhiteRadioButton->isChecked () ||
        m_blackAndWhiteDitheredRadioButton->isChecked ())
    {
        return 1;
    }
    else if (m_8BitRadioButton->isChecked () ||
             m_8BitDitheredRadioButton->isChecked ())
    {
        return 8;
    }
    else if (m_24BitRadioButton->isChecked ())
    {
        return 24;
    }
    else
    {
        return 0;
    }
}

// public
bool kpEffectReduceColorsWidget::dither () const
{
    return (m_blackAndWhiteDitheredRadioButton->isChecked () ||
            m_8BitDitheredRadioButton->isChecked ());
}


//
// kpEffectReduceColorsWidget implements kpColorEffectWidget interface
//

// public virtual [base kpColorEffectWidget]
QString kpEffectReduceColorsWidget::caption () const
{
    return i18n ("Reduce To");
}


// public virtual [base kpColorEffectWidget]
bool kpEffectReduceColorsWidget::isNoOp () const
{
    return (!m_defaultRadioButton || m_defaultRadioButton->isChecked ());
}

// public virtual [base kpColorEffectWidget]
QPixmap kpEffectReduceColorsWidget::applyColorEffect (const QPixmap &pixmap)
{
    return kpEffectReduceColorsCommand::apply (pixmap, depth (), dither ());
}


// public virtual [base kpColorEffectWidget]
kpColorEffectCommand *kpEffectReduceColorsWidget::createCommand () const
{
    return new kpEffectReduceColorsCommand (depth (), dither (),
                                            m_actOnSelection,
                                            m_mainWindow);
}


#include <kpeffectreducecolors.moc>

