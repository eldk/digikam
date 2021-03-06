/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-11-15
 * @brief  Exiv2 library interface
 *
 * @author Copyright (C) 2009-2016 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2009-2012 by Marcel Wiesweg
 *         <a href="mailto:marcel dot wiesweg at gmx dot de">marcel dot wiesweg at gmx dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Local includes

#include "metaengine_data.h"
#include "metaengine.h"
#include "metaengine_p.h"

namespace Digikam
{

MetaEngineData::MetaEngineData()
    : d(0)
{
}

MetaEngineData::MetaEngineData(const MetaEngineData& other)
{
    d = other.d;
}

MetaEngineData::~MetaEngineData()
{
}

MetaEngineData& MetaEngineData::operator=(const MetaEngineData& other)
{
    d = other.d;
    return *this;
}

}  // namespace Digikam
