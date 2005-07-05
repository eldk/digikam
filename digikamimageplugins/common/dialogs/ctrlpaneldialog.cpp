/* ============================================================
 * File  : ctrlpaneldialog.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-07
 * Description : A control panel dialog for plugins
 *
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */
 
// Qt includes.

#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>
#include <qtimer.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <kdebug.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "ctrlpaneldialog.h"

namespace DigikamImagePlugins
{

CtrlPanelDialog::CtrlPanelDialog(QWidget* parent, QString title, QString name, 
                                 bool loadFileSettings)
               : KDialogBase(Plain, title,
                             Help|User1|User2|User3|Ok|Cancel, Ok,
                             parent, 0, true, true,
                             i18n("&Reset Values"),
                             i18n("&Load..."),
                             i18n("&Save As...")),
                     m_parent(parent), m_name(name)
{
    m_currentRenderingMode = NoneRendering;
    m_timer                = 0L;
    m_threadedFilter       = 0L;
    QString whatsThis;
        
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis ( User2, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis ( User3, i18n("<p>Save all filter parameters to settings text file.") );  
    showButton(User2, loadFileSettings);
    showButton(User3, loadFileSettings);
        
    resize(configDialogSize(name + QString::QString(" Tool Dialog")));  
        
    // -------------------------------------------------------------

    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QFrame *headerFrame = new QFrame( plainPage() );
    headerFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* layout = new QHBoxLayout( headerFrame );
    layout->setMargin( 2 ); // to make sure the frame gets displayed
    layout->setSpacing( 0 );
    QLabel *pixmapLabelLeft = new QLabel( headerFrame );
    pixmapLabelLeft->setScaledContents( false );
    layout->addWidget( pixmapLabelLeft );
    QLabel *labelTitle = new QLabel( title, headerFrame );
    layout->addWidget( labelTitle );
    layout->setStretchFactor( labelTitle, 1 );
    topLayout->addWidget(headerFrame);
    
    QString directory;
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", 
                                     KGlobal::dirs()->kde_default("data") +
                                     "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left",
                                                 "digikamimageplugins_banner_left.png");
    
    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );
    
    // -------------------------------------------------------------

    QHBoxLayout *hlay1 = new QHBoxLayout(topLayout);
    
    m_imagePreviewWidget = new Digikam::ImagePannelWidget(240, 160, plainPage(), true);
    hlay1->addWidget(m_imagePreviewWidget);
    
    // -------------------------------------------------------------
    
    QTimer::singleShot(0, this, SLOT(slotInit())); 
        
    // -------------------------------------------------------------
    
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotFocusChanged()));
}

CtrlPanelDialog::~CtrlPanelDialog()
{
    saveDialogSize(m_name + QString::QString(" Tool Dialog"));   

    if (m_timer)
       delete m_timer;
       
    if (m_threadedFilter)
       delete m_threadedFilter;    
       
}

void CtrlPanelDialog::slotInit()
{
    // Abort current computation.
    slotUser1();
    // Waiting filter thread finished.
    kapp->processEvents();
    // Reset values to defaults.
    QTimer::singleShot(0, this, SLOT(slotUser1())); 
}

void CtrlPanelDialog::setAboutData(KAboutData *about)
{
    QPushButton *helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Plugin Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    helpButton->setPopup( helpMenu->menu() );
}

void CtrlPanelDialog::abortPreview()
{
    m_currentRenderingMode = NoneRendering;
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_imagePreviewWidget->setEnable(true);    
    enableButton(Ok, true);  
    enableButton(User2, true);
    enableButton(User3, true);  
    setButtonText(User1, i18n("&Reset Values"));
    setButtonWhatsThis( User1, i18n("<p>Reset all filter parameters to their default values.") );
    renderingFinished();
}

void CtrlPanelDialog::slotUser1()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_threadedFilter->stopComputation();
       }
    else
       {
       resetValues();
       slotEffect();    
       }
} 

void CtrlPanelDialog::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_threadedFilter->stopComputation();
       kapp->restoreOverrideCursor();
       }
       
    done(Cancel);
}

void CtrlPanelDialog::closeEvent(QCloseEvent *e)
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_threadedFilter->stopComputation();
       kapp->restoreOverrideCursor();
       }
       
    e->accept();    
}

void CtrlPanelDialog::slotFocusChanged(void)
{
    if (m_currentRenderingMode == FinalRendering)
       {
       m_imagePreviewWidget->update();
       return;
       }
    else if (m_currentRenderingMode == PreviewRendering)
       {
       m_threadedFilter->stopComputation();
       }
       
    QTimer::singleShot(0, this, SLOT(slotEffect()));        
}

void CtrlPanelDialog::slotHelp()
{
    KApplication::kApplication()->invokeHelp(m_name, "digikamimageplugins");
}

void CtrlPanelDialog::slotTimer()
{
    if (m_timer)
       {
       m_timer->stop();
       delete m_timer;
       }
    
    m_timer = new QTimer( this );
    connect( m_timer, SIGNAL(timeout()),
             this, SLOT(slotEffect()) );
    m_timer->start(500, true);
}

void CtrlPanelDialog::slotEffect()
{
    // Computation already in process.
    if (m_currentRenderingMode == PreviewRendering) return;     
    
    m_currentRenderingMode = PreviewRendering;

    m_imagePreviewWidget->setEnable(false);
    setButtonText(User1, i18n("&Abort"));
    setButtonWhatsThis( User1, i18n("<p>Abort the current image rendering.") );
    enableButton(Ok,    false);
    enableButton(User2, false);
    enableButton(User3, false);    
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    m_imagePreviewWidget->setProgress(0);
    
    if (m_threadedFilter)
       delete m_threadedFilter;
       
    prepareEffect();
}

void CtrlPanelDialog::slotOk()
{
    m_currentRenderingMode = FinalRendering;

    m_imagePreviewWidget->setEnable(false);
    enableButton(Ok,    false);
    enableButton(User1, false);
    enableButton(User2, false);
    enableButton(User3, false);
    kapp->setOverrideCursor( KCursor::waitCursor() );
    m_imagePreviewWidget->setProgress(0);
    
    if (m_threadedFilter)
       delete m_threadedFilter;
    
    prepareFinal();
}

void CtrlPanelDialog::customEvent(QCustomEvent *event)
{
    if (!event) return;

    Digikam::ThreadedFilter::EventData *d = (Digikam::ThreadedFilter::EventData*) event->data();

    if (!d) return;
    
    if (d->starting)           // Computation in progress !
        {
        m_imagePreviewWidget->setProgress(d->progress);
        }  
    else 
        {
        if (d->success)        // Computation Completed !
            {
            switch (m_currentRenderingMode)
              {
              case PreviewRendering:
                 {
                 kdDebug() << "Preview " << m_name << " completed..." << endl;
                 putPreviewData();
                 abortPreview();
                 break;
                 }
              
              case FinalRendering:
                 {
                 kdDebug() << "Final" << m_name << " completed..." << endl;
                 putFinalData();
                 kapp->restoreOverrideCursor();
                 accept();
                 break;
                 }
              }
            }
        else                   // Computation Failed !
            {
            switch (m_currentRenderingMode)
                {
                case PreviewRendering:
                    {
                    kdDebug() << "Preview " << m_name << " failed..." << endl;
                    // abortPreview() must be call here for set progress bar to 0 properly.
                    abortPreview();
                    break;
                    }
                
                case FinalRendering:
                    break;
                }
            }
        }
    
    delete d;        
}

}  // NameSpace DigikamImagePlugins

#include "ctrlpaneldialog.moc"
