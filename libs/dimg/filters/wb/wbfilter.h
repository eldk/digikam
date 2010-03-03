/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-16-01
 * Description : white balance color correction.
 *
 * Copyright (C) 2007-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008 by Guillaume Castagnino <casta at xwing dot info>
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

#ifndef WBFILTER_H
#define WBFILTER_H

// Qt includes.

#include <QColor>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "globals.h"

using namespace Digikam;

namespace Digikam
{

class DImg;
class WBFilterPriv;

class DIGIKAM_EXPORT WBContainer
{

public:

    WBContainer()
    {
        // Neutral color temperature settings.
        black       = 0.0;
        exposition  = 0.0;
        temperature = 6500.0;
        green       = 1.0;
        dark        = 0.5;
        gamma       = 1.0;
        saturation  = 1.0;
    };

    ~WBContainer(){};

public:

    double black;
    double exposition;
    double temperature;
    double green;
    double dark;
    double gamma;
    double saturation;
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT WBFilter : public DImgThreadedFilter
{

public:

    explicit WBFilter(DImg* orgImage, QObject* parent=0, const WBContainer& settings=WBContainer());
    WBFilter(uchar* data, uint width, uint height, bool sixteenBit, const WBContainer& settings=WBContainer());
    virtual ~WBFilter();

    static void autoExposureAdjustement(const DImg* img, double& black, double& expo);
    static void autoWBAdjustementFromColor(const QColor& tc, double& temperature, double& green);

protected:

    virtual void filterImage();

  protected:

    WBContainer m_settings;

private:

    void setRGBmult();
    void setLUTv();
    void adjustWhiteBalance(uchar* data, int width, int height, bool sixteenBit);
    inline unsigned short pixelColor(int colorMult, int index, int value);

    static void setRGBmult(double& temperature, double& green, float& mr, float& mg, float& mb);

private:

    WBFilterPriv* const d;
};

}  // namespace Digikam

#endif /* WBFILTER_H */
