/*
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef DTGTZWIDGET_H
#define DTGTZWIDGET_H
#include <map>
#include <kdeui_export.h>
#include <QtGui/QTreeWidget>

class KTimeZone;
class KTimeZones;

/**
 * @brief A time zone selection widget.
 *
 * \b Detail:
 *
 * This class provides for selection of one or more time zones.
 *
 * \b Example:
 *
 * To use the class to implement a system timezone selection feature:
 * \code
 *
 *  // This adds a time zone widget to a dialog.
 *  m_timezones = new Dtgtzwidget(this);
 *  ...
 * \endcode
 *
 * To use the class to implement a multiple-choice custom time zone selector:
 * \code
 *
 *  m_timezones = new Dtgtzwidget( this, "Time zones", vcalendarTimezones );
 *  m_timezones->setSelectionMode( QTreeView::MultiSelection );
 *  ...
 * \endcode
 *
 * \image html Dtgtzwidget.png "KDE Time Zone Widget"
 *
 * @author S.R.Haque <srhaque@iee.org>
 */
class KDEUI_EXPORT Dtgtzwidget : public QTreeWidget
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemView::SelectionMode selectionMode READ selectionMode WRITE setSelectionMode)

  public:
    /**
     * Constructs a time zone selection widget.
     *
     * @param parent The parent widget.
     * @param timeZones The time zone database to use. If 0, the system time zone
     *                  database is used.
     */
    explicit Dtgtzwidget( QWidget *parent = 0, KTimeZones *timeZones = 0 );

    /**
     * Destroys the time zone selection widget.
     */
    virtual ~Dtgtzwidget();

    /**
     * Returns the currently selected time zones. See QTreeView::selectionChanged().
     *
     * @return a list of time zone names, in the format used by the database
     *         supplied to the {@link Dtgtzwidget() } constructor.
     */
    QString selection() const;

    /**
     * Select/deselect the named time zone.
     *
     * @param zone The time zone name to be selected. Ignored if not recognized!
     * @param selected The new selection state.
     */
    void setSelected( const QString &zone, bool selected );

    /**
     * Unselect all timezones.
     * This is the same as QTreeWidget::clearSelection, except in checkable items mode,
     * where items are all unchecked.
     * The overload is @since 4.4.
     */
    void clearSelection();

    /**
     * Format a time zone name in a standardised manner. The returned value is
     * transformed via an i18n lookup, so the caller should previously have
     * set the time zone catalog:
     * \code
     *   KGlobal::locale()->insertCatalog( "timezones4" );
     * \endcode
     *
     * @return formatted time zone name.
     */
    static QString displayName( const KTimeZone &zone );
    public slots:
	void toggleLetters(bool value);
  private:
	void fillTimeZoneMap(std::map<int,QString> &timezone);
	class Private;
	Private* const d;
};

#endif
