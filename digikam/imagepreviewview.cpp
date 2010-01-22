/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : a embedded view to show the image preview widget.
 *
 * Copyright (C) 2006-2010 Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "imagepreviewview.moc"

// Qt includes

#include <QCursor>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QPainter>
#include <QPixmap>
#include <QString>

// KDE includes

#include <kaction.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmimetype.h>
#include <kmimetypetrader.h>

// LibKIPI includes

#include <libkipi/plugin.h>
#include <libkipi/pluginloader.h>

// Local includes

#include "albumdb.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "albumwidgetstack.h"
#include "contextmenuhelper.h"
#include "databaseaccess.h"
#include "digikamapp.h"
#include "dimg.h"
#include "dmetadata.h"
#include "dpopupmenu.h"
#include "imageinfo.h"
#include "loadingdescription.h"
#include "metadatahub.h"
#include "previewloadthread.h"
#include "ratingpopupmenu.h"
#include "tagspopupmenu.h"
#include "themeengine.h"

namespace Digikam
{

class ImagePreviewViewPriv
{
public:

    ImagePreviewViewPriv()
    {
        previewThread        = 0;
        previewPreloadThread = 0;
        stack                = 0;
        hasPrev              = false;
        hasNext              = false;
        loadFullImageSize    = false;
        previewSize          = 1024;
    }

    bool               hasPrev;
    bool               hasNext;
    bool               loadFullImageSize;

    int                previewSize;

    QString            path;
    QString            nextPath;
    QString            previousPath;

    DImg               preview;

    ImageInfo          imageInfo;

    PreviewLoadThread* previewThread;
    PreviewLoadThread* previewPreloadThread;

    AlbumWidgetStack*  stack;
};

ImagePreviewView::ImagePreviewView(QWidget *parent, AlbumWidgetStack *stack)
                : PreviewWidget(parent), d(new ImagePreviewViewPriv)
{
    d->stack = stack;

    // get preview size from screen size, but limit from VGA to WQXGA
    d->previewSize = qMax(KApplication::desktop()->height(),
                          KApplication::desktop()->width());
    if (d->previewSize < 640)
        d->previewSize = 640;
    if (d->previewSize > 2560)
        d->previewSize = 2560;

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // ------------------------------------------------------------

    connect(this, SIGNAL(signalShowNextImage()),
            this, SIGNAL(signalNextItem()));

    connect(this, SIGNAL(signalShowPrevImage()),
            this, SIGNAL(signalPrevItem()));

    connect(this, SIGNAL(signalRightButtonClicked()),
            this, SLOT(slotContextMenu()));

    connect(this, SIGNAL(signalLeftButtonClicked()),
            this, SIGNAL(signalBack2Album()));

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    // ------------------------------------------------------------

    slotReset();
}

ImagePreviewView::~ImagePreviewView()
{
    delete d->previewThread;
    delete d->previewPreloadThread;
    delete d;
}

void ImagePreviewView::setLoadFullImageSize(bool b)
{
    d->loadFullImageSize = b;
    reload();
}

void ImagePreviewView::setImage(const DImg& image)
{
    d->preview = image;

    updateZoomAndSize(true);

    viewport()->setUpdatesEnabled(true);
    viewport()->update();
}

DImg& ImagePreviewView::getImage() const
{
    return d->preview;
}

void ImagePreviewView::reload()
{
    // cache is cleaned from AlbumIconView::refreshItems
    setImagePath(d->path);
}

void ImagePreviewView::setPreviousNextPaths(const QString& previous, const QString& next)
{
    d->nextPath     = next;
    d->previousPath = previous;
}

void ImagePreviewView::setImagePath(const QString& path)
{
    setCursor( Qt::WaitCursor );

    d->path         = path;
    d->nextPath.clear();
    d->previousPath.clear();

    if (d->path.isEmpty())
    {
        slotReset();
        unsetCursor();
        return;
    }

    if (!d->previewThread)
    {
        d->previewThread = new PreviewLoadThread();
        d->previewThread->setDisplayingWidget(this);
        connect(d->previewThread, SIGNAL(signalImageLoaded(const LoadingDescription &, const DImg &)),
                this, SLOT(slotGotImagePreview(const LoadingDescription &, const DImg&)));
    }
    if (!d->previewPreloadThread)
    {
        d->previewPreloadThread = new PreviewLoadThread();
        d->previewPreloadThread->setDisplayingWidget(this);
        connect(d->previewPreloadThread, SIGNAL(signalImageLoaded(const LoadingDescription &, const DImg &)),
                this, SLOT(slotNextPreload()));
    }

    if (d->loadFullImageSize)
        d->previewThread->loadHighQuality(path, AlbumSettings::instance()->getExifRotate());
    else
        d->previewThread->load(path, d->previewSize, AlbumSettings::instance()->getExifRotate());
}

void ImagePreviewView::slotGotImagePreview(const LoadingDescription& description, const DImg& preview)
{
    if (description.filePath != d->path || description.isThumbnail())
        return;

    if (preview.isNull())
    {
        d->stack->setPreviewMode(AlbumWidgetStack::PreviewImageMode);
        QPixmap pix(visibleWidth(), visibleHeight());
        pix.fill(ThemeEngine::instance()->baseColor());
        QPainter p(&pix);
        QFileInfo info(d->path);
        p.setPen(QPen(ThemeEngine::instance()->textRegColor()));
        p.drawText(0, 0, pix.width(), pix.height(),
                   Qt::AlignCenter|Qt::TextWordWrap,
                   i18n("Cannot display preview for\n\"%1\"",
                   info.fileName()));
        p.end();
        // three copies - but the image is small
        setImage(DImg(pix.toImage()));
        d->stack->previewLoaded();
        emit signalPreviewLoaded(false);
    }
    else
    {
        DImg img(preview);
        if (AlbumSettings::instance()->getExifRotate())
            d->previewThread->exifRotate(img, description.filePath);
        d->stack->setPreviewMode(AlbumWidgetStack::PreviewImageMode);
        setImage(img);
        d->stack->previewLoaded();
        emit signalPreviewLoaded(true);
    }

    unsetCursor();
    slotNextPreload();
}

void ImagePreviewView::slotNextPreload()
{
    QString loadPath;
    if (!d->nextPath.isNull())
    {
        loadPath    = d->nextPath;
        d->nextPath.clear();
    }
    else if (!d->previousPath.isNull())
    {
        loadPath        = d->previousPath;
        d->previousPath.clear();
    }
    else
        return;

    if (d->loadFullImageSize)
        d->previewThread->loadHighQuality(loadPath, AlbumSettings::instance()->getExifRotate());
    else
        d->previewPreloadThread->load(loadPath, d->previewSize, AlbumSettings::instance()->getExifRotate());
}

void ImagePreviewView::setImageInfo(const ImageInfo & info, const ImageInfo& previous, const ImageInfo& next)
{
    d->imageInfo = info;
    d->hasPrev   = previous.isNull() ? false : true;
    d->hasNext   = next.isNull()     ? false : true;

    if (!d->imageInfo.isNull())
        setImagePath(info.filePath());
    else
        setImagePath();

    setPreviousNextPaths(previous.filePath(), next.filePath());
}

ImageInfo ImagePreviewView::getImageInfo() const
{
    return d->imageInfo;
}

void ImagePreviewView::slotContextMenu()
{
    if (d->imageInfo.isNull())
        return;

    QList<qlonglong> idList;
    idList << d->imageInfo.id();
    KUrl::List selectedItems;
    selectedItems << d->imageInfo.fileUrl();

    // Temporary actions --------------------------------------

    QAction *back2AlbumAction, *prevAction, *nextAction = 0;

    back2AlbumAction = new QAction(SmallIcon("folder-image"), i18n("Back to Album"), this);
    prevAction       = new QAction(SmallIcon("go-previous"),  i18nc("go to previous image", "Back"), this);
    nextAction       = new QAction(SmallIcon("go-next"),      i18nc("go to next image", "Forward"), this);
    prevAction->setEnabled(d->hasPrev);
    nextAction->setEnabled(d->hasNext);

    // --------------------------------------------------------

    DPopupMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);

    cmhelper.addAction(prevAction, true);
    cmhelper.addAction(nextAction, true);
    cmhelper.addAction(back2AlbumAction);
    cmhelper.addGotoMenu(idList);
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("image_edit");
    cmhelper.addServicesMenu(selectedItems);
    cmhelper.addKipiActions(idList);
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("image_find_similar");
    cmhelper.addActionLightTable();
    cmhelper.addQueueManagerMenu();
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addActionItemDelete(this, SLOT(slotDeleteItem()));
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAssignTagsMenu(idList);
    cmhelper.addRemoveTagsMenu(idList);
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addRatingMenu();

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalAssignTag(int)),
            this, SLOT(slotAssignTag(int)));

    connect(&cmhelper, SIGNAL(signalRemoveTag(int)),
            this, SLOT(slotRemoveTag(int)));

    connect(&cmhelper, SIGNAL(signalAssignRating(int)),
            this, SLOT(slotAssignRating(int)));

    connect(&cmhelper, SIGNAL(signalAddToExistingQueue(int)),
            this, SIGNAL(signalAddToExistingQueue(int)));

    connect(&cmhelper, SIGNAL(signalGotoTag(int)),
            this, SLOT(slotGotoTag(int)));

    connect(&cmhelper, SIGNAL(signalGotoAlbum(const ImageInfo&)),
            this, SIGNAL(signalGotoAlbumAndItem(const ImageInfo&)));

    connect(&cmhelper, SIGNAL(signalGotoDate(const ImageInfo&)),
            this, SIGNAL(signalGotoDateAndItem(const ImageInfo&)));

    // handle temporary actions

    QAction* choice = cmhelper.exec(QCursor::pos());
    if (choice)
    {
        if (choice == prevAction)            emit signalPrevItem();
        else if (choice == nextAction)       emit signalNextItem();
        else if (choice == back2AlbumAction) emit signalBack2Album();
    }
}

void ImagePreviewView::slotAssignTag(int tagID)
{
    if (!d->imageInfo.isNull())
    {
        MetadataHub hub;
        hub.load(d->imageInfo);
        hub.setTag(tagID, true);
        hub.write(d->imageInfo, MetadataHub::PartialWrite);
        hub.write(d->imageInfo.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void ImagePreviewView::slotRemoveTag(int tagID)
{
    if (!d->imageInfo.isNull())
    {
        MetadataHub hub;
        hub.load(d->imageInfo);
        hub.setTag(tagID, false);
        hub.write(d->imageInfo, MetadataHub::PartialWrite);
        hub.write(d->imageInfo.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void ImagePreviewView::slotAssignRating(int rating)
{
    rating = qMin(5, qMax(0, rating));
    if (!d->imageInfo.isNull())
    {
        MetadataHub hub;
        hub.load(d->imageInfo);
        hub.setRating(rating);
        hub.write(d->imageInfo, MetadataHub::PartialWrite);
        hub.write(d->imageInfo.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void ImagePreviewView::slotThemeChanged()
{
    QPalette plt(palette());
    plt.setColor(backgroundRole(), ThemeEngine::instance()->baseColor());
    setPalette(plt);
}

void ImagePreviewView::slotDeleteItem()
{
    emit signalDeleteItem();
}

void ImagePreviewView::resizeEvent(QResizeEvent* e)
{
    if (!e) return;

    Q3ScrollView::resizeEvent(e);

    updateZoomAndSize(false);
}

int ImagePreviewView::previewWidth()
{
    return d->preview.width();
}

int ImagePreviewView::previewHeight()
{
    return d->preview.height();
}

bool ImagePreviewView::previewIsNull()
{
    return d->preview.isNull();
}

void ImagePreviewView::resetPreview()
{
    d->preview   = DImg();
    d->path.clear();
    d->imageInfo = ImageInfo();

    updateZoomAndSize(true);
    emit signalPreviewLoaded(false);
}

void ImagePreviewView::paintPreview(QPixmap *pix, int sx, int sy, int sw, int sh)
{
    DImg img     = d->preview.smoothScaleSection(sx, sy, sw, sh, tileSize(), tileSize());
    QPixmap pix2 = img.convertToPixmap();
    QPainter p(pix);
    p.drawPixmap(0, 0, pix2);
    p.end();
}

void ImagePreviewView::viewportPaintExtraData()
{
    if (!m_movingInProgress && !previewIsNull())
    {
        QPainter p(viewport());
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setBackgroundMode(Qt::TransparentMode);
        QFontMetrics fontMt = p.fontMetrics();

        QString text;
        QRect textRect, fontRect;
        QRect region = contentsRect();
        p.translate(region.topLeft());

        // Drawing separate view.

        if (!d->loadFullImageSize)
        {
            text     = i18n("Reduced Size Preview");
            fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text);
            textRect.setTopLeft(QPoint(region.topLeft().x()+20, region.topLeft().y()+20));
            textRect.setSize( QSize(fontRect.width()+2, fontRect.height()+2) );
            drawText(&p, textRect, text);
        }

        p.end();
    }
}

void ImagePreviewView::slotGotoTag(int tagID)
{
    emit signalGotoTagAndItem(tagID);
}

QImage ImagePreviewView::previewToQImage() const
{
    return d->preview.copyQImage();
}

}  // namespace Digikam
