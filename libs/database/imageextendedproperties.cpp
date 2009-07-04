/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-04
 * Description : Access to extended properties of an image in the database
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "imageextendedproperties.h"

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>

// Local includes

#include "albumdb.h"
#include "databaseaccess.h"
#include "imagescanner.h"

namespace Digikam
{

ImageExtendedProperties::ImageExtendedProperties(qlonglong imageid)
              : m_id(imageid)
{
}

ImageExtendedProperties::ImageExtendedProperties()
              : m_id(0)
{
}

QString ImageExtendedProperties::intellectualGenre()
{
    return readProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreIntellectualGenre));
}

void ImageExtendedProperties::setIntellectualGenre(const QString &intellectualGenre)
{
    setProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreIntellectualGenre), intellectualGenre);
}

QString ImageExtendedProperties::jobId()
{
    return readProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreJobID));
}

void ImageExtendedProperties::setJobId(const QString &jobId)
{
    setProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreJobID), jobId);
}

QStringList ImageExtendedProperties::scene()
{
    return readFakeListProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreScene));
}

void ImageExtendedProperties::setScene(const QStringList &scene)
{
    setFakeListProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreScene), scene);
}

QStringList ImageExtendedProperties::subjectCode()
{
    return readFakeListProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreSubjectCode));
}

void ImageExtendedProperties::setSubjectCode(const QStringList &subjectCode)
{
    setFakeListProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreSubjectCode), subjectCode);
}

IptcCoreLocationInfo ImageExtendedProperties::location()
{
    IptcCoreLocationInfo location;
    location.country = readProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCountry));
    location.countryCode = readProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCountryCode));
    location.city = readProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCity));
    location.location = readProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreLocation));
    location.provinceState = readProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreProvinceState));
    return location;
}

void ImageExtendedProperties::setLocation(const IptcCoreLocationInfo &location)
{
    setProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCountry), location.country);
    setProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCountryCode), location.countryCode);
    setProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCity), location.city);
    setProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreLocation), location.location);
    setProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreProvinceState), location.provinceState);
}

QString ImageExtendedProperties::readProperty(const QString& property)
{
    return DatabaseAccess().db()->getImageProperty(m_id, property);
}

void ImageExtendedProperties::setProperty(const QString& property, const QString& value)
{
    DatabaseAccess().db()->setImageProperty(m_id, property, value);
}

QStringList ImageExtendedProperties::readFakeListProperty(const QString& property)
{
    QString value = DatabaseAccess().db()->getImageProperty(m_id, property);
    return value.split(',', QString::SkipEmptyParts);
}

void ImageExtendedProperties::setFakeListProperty(const QString& property, const QStringList& value)
{
    DatabaseAccess().db()->setImageProperty(m_id, property, value.join(","));
}

void ImageExtendedProperties::removeProperty(const QString &property)
{
    DatabaseAccess().db()->removeImageProperty(m_id, property);
}


} // namespace Digikam
