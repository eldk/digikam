/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-05-19
 * Copyright 2005 by Renchi Raju
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
 * ============================================================ */

/** @file searchquickdialog.h */

#ifndef SEARCHQUICKDIALOG_H
#define SEARCHQUICKDIALOG_H

#include <kdialogbase.h>
#include <qcstring.h>

class KURL;
class QLineEdit;
class QTimer;
class SearchResultsView;

/** @class SearchQuickDialog
 * 
 * This is the dialog for the quick search
 * @author Renchi Raju
 * 
 */
class SearchQuickDialog : public KDialogBase
{
    Q_OBJECT

public:

    /**
     * Constructor
     * @param parent parent window
     * @param url holds the url for the search
     */
    SearchQuickDialog(QWidget* parent, KURL& url);
    /**
     * Destructor
     */
    ~SearchQuickDialog();

protected:

    void hideEvent(QHideEvent* e);
    
private:

    QString possibleDate(const QString& str, bool& exact) const;
    
private:

    QLineEdit*           m_searchEdit;
    QLineEdit*           m_nameEdit;
    SearchResultsView*   m_resultsView;
    QTimer*              m_timer;
    KURL&                m_url;
    QString              m_longMonths[12];
    QString              m_shortMonths[12];
    
private slots:

    void slotTimeOut();
    void slotSearchChanged(const QString&);
};


#endif /* SEARCHQUICKDIALOG_H */
