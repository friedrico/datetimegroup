/*
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
 

#ifndef DATETIMEGROUPLIB_H
#define DATETIMEGROUPLIB_H
#include <map>
#include <Plasma/DataEngine>
#include <QStringList>

/**
 * This engine provides the current date and time for a given
 * timezone.
 *
 * "Local" is a special source that is an alias for the current
 * timezone.
 */
class DateTimeGroupLib : public Plasma::DataEngine
{
	Q_OBJECT

	public:
		DateTimeGroupLib(QObject* parent, const QVariantList& args);

	protected:
		bool sourceRequestEvent(const QString& name);
		bool updateSourceEvent(const QString& source);
		virtual QStringList sources() const;
	private:
		std::map<int, QString> timezone;

};
 
#endif