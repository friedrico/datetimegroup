#include "datetimegrouplib.h"
 
#include <QDate>
#include <QTime>
#include <QStringList>
#include <KSystemTimeZones>
#include <KDateTime>
#include <map>
#include <Plasma/DataContainer>
 
DateTimeGroupLib::DateTimeGroupLib(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent, args)
{
    // We ignore any arguments - data engines do not have much use for them
    Q_UNUSED(args)
 
    // This prevents applets from setting an unnecessarily high
    // update interval and using too much CPU.
    // In the case of a clock that only has second precision,
    // a third of a second should be more than enough.
    setMinimumPollingInterval(333);
}

bool DateTimeGroupLib::sourceRequestEvent(const QString &name)
{
	timezone[0]=QString("Z");
	timezone[60]=QString("A");
	timezone[120]=QString("B");
	timezone[180]=QString("C");
	timezone[210]=QString("C*");
	timezone[240]=QString("D");
	timezone[270]=QString("D*");
	timezone[300]=QString("E");
	timezone[330]=QString("E*");
	timezone[345]=QString("E%1").arg( QChar( 0x2020 )); //†
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
	timezone[840]=QString("M%1").arg( QChar( 0x2020 )); //†
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
	return updateSourceEvent(name);
}
 
bool DateTimeGroupLib::updateSourceEvent(const QString &name)
{
	KDateTime dt;
	if (name == I18N_NOOP("Local")) {
		dt = KDateTime::currentLocalDateTime();
	} 
	else {
		// First check the timezone is valid
		KTimeZone newTz = KSystemTimeZones::zone(name);
		if (!newTz.isValid()) {
			return false;
		}

		// Get the date and time
		dt = KDateTime::currentDateTime(newTz);

	}
	QString natoTimeZone=QString(timezone[dt.utcOffset()/60]);
	if(natoTimeZone.isEmpty())
		natoTimeZone=" ";

	QString natoTimeZone2=QString(timezone[dt.utcOffset()/60-(dt.utcOffset()/60)%60]).left(1); //because of chatham island's daylight saving time

	setData(name, I18N_NOOP("Time"), dt.time());
	setData(name, I18N_NOOP("Day"), dt.toString("%d"));
	setData(name, I18N_NOOP("Hour"), dt.toString("%H"));
	setData(name, I18N_NOOP("Min"), dt.toString("%M"));
	setData(name, I18N_NOOP("Seconds"), dt.toString("%S"));
	setData(name, I18N_NOOP("TimeZone"), natoTimeZone);
	setData(name, I18N_NOOP("TimeZoneRounded"), natoTimeZone2);
	setData(name, I18N_NOOP("Month"), dt.toString("%:b").toLower());
	setData(name, I18N_NOOP("YearS"), dt.toString("%y"));
	setData(name, I18N_NOOP("YearL"), dt.toString("%Y"));
	setData(name, I18N_NOOP("GMTOffset"), dt.utcOffset()/60);
	return true;
}
 QStringList DateTimeGroupLib::sources() const{
	KTimeZones *c=KSystemTimeZones::timeZones();
	return QStringList(c->zones().keys());
// 	return c;
}
// This does the magic that allows Plasma to load
// this plugin.  The first argument must match
// the X-Plasma-EngineName in the .desktop file.
// The second argument is the name of the class in
// your plugin that derives from Plasma::DataEngine
K_EXPORT_PLASMA_DATAENGINE(datetimegroup, DateTimeGroupLib)
 
// this is needed since DateTimeGroupLib is a QObject
#include "datetimegrouplib.moc"