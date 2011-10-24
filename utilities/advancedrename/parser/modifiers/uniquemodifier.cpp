/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-27
 * Description : A modifier for appending a suffix number to a renaming option.
 *               This guarantees a unique string for duplicate values.
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at googlemail dot com>
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

#include "uniquemodifier.moc"

// KDE includes

#include <kdebug.h>
#include <klocale.h>

namespace Digikam
{

UniqueModifier::UniqueModifier()
    : Modifier(i18nc("unique value for duplicate strings", "Unique"),
               i18n("Add a suffix number to have unique strings in duplicate values"), "button_more")
{
    addToken("{unique}", description());
    addToken("{unique:||n||}", i18n("Add a suffix number, ||n|| specifies the number of digits to use"));

    QRegExp reg("\\{unique(:(\\d+))?\\}");
    reg.setMinimal(true);
    setRegExp(reg);
}

QString UniqueModifier::parseOperation(ParseSettings& settings)
{
    ParseResults::ResultsKey key = settings.currentResultsKey;
    cache[key] << settings.str2Modify;

    const QRegExp& reg = regExp();

    if (cache[key].count(settings.str2Modify) > 1)
    {
        QString result = settings.str2Modify;
        int index      = cache[key].count(settings.str2Modify) - 1;

        bool ok     = true;
        int slength = reg.cap(2).toInt(&ok);
        slength     = (slength == 0 || !ok) ? 1 : slength;
        result     += QString("_%1").arg(index, slength, 10, QChar('0'));
        return result;
    }

    return settings.str2Modify;
}

void UniqueModifier::reset()
{
    cache.clear();
}

} // namespace Digikam
