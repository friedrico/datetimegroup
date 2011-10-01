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

#include "dtgtzwidget.h"

#include <QtCore/QFile>
#include <QtGui/QPixmap>
#include <map>
#include <KDebug>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ksystemtimezone.h>
#include <ktimezone.h>
#include <math.h>
class Dtgtzwidget::Private
{
public:
    Private() : itemsCheckable(false), singleSelection(true) {}

    enum Columns
    {
        CityColumn = 0,
        RegionColumn,
        CommentColumn,
	NatoColumn,
	NatoRoundedColumn
    };

    enum Roles
    {
        ZoneRole = Qt::UserRole + 0xF3A3CB1
    };
    bool itemsCheckable;
    bool singleSelection;
};

#ifndef KDE_USE_FINAL
static bool localeLessThan (const QString &a, const QString &b)
{
    return QString::localeAwareCompare(a, b) < 0;
}
#endif

Dtgtzwidget::Dtgtzwidget( QWidget *parent, KTimeZones *db )
  : QTreeWidget( parent ),
    d(new Dtgtzwidget::Private)
{

  // If the user did not provide a timezone database, we'll use the system default.
  setRootIsDecorated(false);
  setHeaderLabels( QStringList() << i18nc("Define an area in the time zone, like a town area", "Area" ) << i18nc( "Time zone", "Region" ) << i18n( "Comment" ) << "NATO Time Zone");

  // Collect zones by localized city names, so that they can be sorted properly.
  QStringList cities;
  QHash<QString, KTimeZone> zonesByCity;
QStringList NatoColumn;
QStringList NatoRoundedColumn;
  if (!db) {
      db = KSystemTimeZones::timeZones();

      // add UTC to the defaults default
      KTimeZone utc = KTimeZone::utc();
      cities.append(utc.name());
      zonesByCity.insert(utc.name(), utc);
  }
  const KTimeZones::ZoneMap zones = db->zones();
  for ( KTimeZones::ZoneMap::ConstIterator it = zones.begin(); it != zones.end(); ++it ) {
    const KTimeZone zone = it.value();
    const QString continentCity = displayName( zone );
    const int separator = continentCity.lastIndexOf('/');
    // Make up the localized key that will be used for sorting.
    // Example: i18n(Asia/Tokyo) -> key = "i18n(Tokyo)|i18n(Asia)|Asia/Tokyo"
    // The zone name is appended to ensure unicity even with equal translations (#174918)
    const QString key = continentCity.mid(separator+1) + '|'
                   + continentCity.left(separator) + '|' + zone.name();
    cities.append( key );
    zonesByCity.insert( key, zone );
  }
  qSort( cities.begin(), cities.end(), localeLessThan );
  
std::map<int, QString> timezone;
fillTimeZoneMap(timezone);

  foreach ( const QString &key, cities ) {
    const KTimeZone zone = zonesByCity.value(key);
    const QString tzName = zone.name();
    QString comment = zone.comment();
	;
	
    if ( !comment.isEmpty() )
      comment = i18n( comment.toUtf8() );

    // Convert:
    //
    //  "Europe/London", "GB" -> "London", "Europe/GB".
    //  "UTC",           ""   -> "UTC",    "".
    QStringList continentCity = displayName( zone ).split( '/' );

    QTreeWidgetItem *listItem = new QTreeWidgetItem( this );
    listItem->setText( Private::CityColumn, continentCity[ continentCity.count() - 1 ] );
    QString countryName = KGlobal::locale()->countryCodeToName( zone.countryCode() );
    if ( countryName.isEmpty() ) {
	continentCity[ continentCity.count() - 1 ] = zone.countryCode();
    }else {
	continentCity[ continentCity.count() - 1 ] = countryName;
    }
	int roundedOffset=zone.currentOffset()/60-(zone.currentOffset()/60)%60;

	listItem->setText( Private::RegionColumn, continentCity.join( QChar('/') ) );
	listItem->setText( Private::CommentColumn, comment );
	listItem->setData( Private::CityColumn, Private::ZoneRole, tzName ); // store complete path in custom role
	listItem->setText( Private::NatoColumn, timezone[zone.currentOffset()/60]);
	listItem->setText( Private::NatoRoundedColumn, timezone[roundedOffset].left(1)); //because of chatham island's daylight saving time

    // Locate the flag from /l10n/%1/flag.png.
    QString flag = KStandardDirs::locate( "locale", QString( "l10n/%1/flag.png" ).arg( zone.countryCode().toLower() ) );
    if ( QFile::exists( flag ) )
      listItem->setIcon( Private::RegionColumn, QPixmap( flag ) );
  }


}

Dtgtzwidget::~Dtgtzwidget()
{
    delete d;
}
void Dtgtzwidget::toggleLetters(bool value){
// 	tis.
// 	QTreeWidgetItemIterator it(this);
// 	int e=0;
// 	int q=0;
// 	while (*it) {
// 		++e;
// 		if((*it)->text(0)=="Eucla"){
// 			++q;
// 			kDebug()<<(*it)->text(0)<<(*it)->text(1)<<(*it)->text(2)<<(*it)->text(3)<<(*it)->text(4)<<(*it)->text(5)<<value;
// 		}
// 		(*it)->setText(Private::CurrentColumn, (*it)->text(value?5:4));
// 		++it;
// 	}
// 	kDebug()<<value <<isColumnHidden ( int column );
setColumnHidden(3,value);
setColumnHidden(4,!value);
}
void Dtgtzwidget::fillTimeZoneMap(std::map<int,QString> &timezone){
	timezone[0]=QString("Z");
	timezone[60]=QString("A");
	timezone[120]=QString("B");
	timezone[180]=QString("C");
	timezone[210]=QString("C*");
	timezone[240]=QString("D");
	timezone[270]=QString("D*");
	timezone[300]=QString("E");
	timezone[330]=QString("E*");
	timezone[345]=QString("E%1").arg( QChar( 0x2020 ));;
	timezone[360]=QString("F");
	timezone[390]=QString("F*");
	timezone[420]=QString("G");
	timezone[480]=QString("H");
	timezone[540]=QString("I");
	timezone[570]=QString("I*");
	timezone[600]=QString("K");
	timezone[630]=QString("K*");
	timezone[660]=QString("L");
	timezone[690]=QString("L*");
	timezone[720]=QString("M");
	timezone[765]=QString("M%1").arg( QChar( 0x25C6 )); //♦\u2598
	timezone[780]=QString("M*");
	timezone[840]=QString("M%1").arg( QChar( 0x2020 ));;
	timezone[-60]=QString("N");
	timezone[-120]=QString("O");
	timezone[-180]=QString("P");
	timezone[-210]=QString("P*");
	timezone[-240]=QString("Q");
	timezone[-270]=QString("Q*");
	timezone[-300]=QString("R");
	timezone[-360]=QString("S");
	timezone[-420]=QString("T");
	timezone[-480]=QString("U");
	timezone[-540]=QString("V");
	timezone[-570]=QString("V*");
	timezone[-600]=QString("W");
	timezone[-660]=QString("X");
	timezone[-720]=QString("Y");
	
}
QString Dtgtzwidget::displayName( const KTimeZone &zone )
{
    return i18n( zone.name().toUtf8() ).replace( '_', ' ' );
}

QString Dtgtzwidget::selection() const
{
	return 	selectedItems().length()==0?"Local":selectedItems().first()->data( Private::CityColumn, Private::ZoneRole ).toString();
}

void Dtgtzwidget::setSelected( const QString &zone, bool selected )
{
    bool found = false;
    // Loop through all entries.
    const int rowCount = model()->rowCount(QModelIndex());
    for (int row = 0; row < rowCount; ++row) {
        const QModelIndex index = model()->index(row, Private::CityColumn);
        const QString tzName = index.data(Private::ZoneRole).toString();
        if (tzName == zone) {

            if (d->singleSelection && selected) {
                clearSelection();
            }
		selectionModel()->select(index, selected ? (QItemSelectionModel::Select | QItemSelectionModel::Rows) : (QItemSelectionModel::Deselect | QItemSelectionModel::Rows));

            // Ensure the selected item is visible as appropriate.
            scrollTo( index );

            found = true;

            if (selected) {
                break;
            }
        }
    }

    if ( !found )
        kDebug() << "No such zone: " << zone;
}

void Dtgtzwidget::clearSelection()
{
        QTreeWidget::clearSelection();
}


#include "dtgtzwidget.moc"
