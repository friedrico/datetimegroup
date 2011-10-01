/***************************************************************************
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
***************************************************************************/

#include "datetimegroup.h"
#include "dtgtzwidget.h"
#include <math.h>
#include <KSystemTimeZones>

#include <QtGui/QPainter>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QSpinBox>
#include <QtCore/QTimeLine>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtCore/QDate>

#include <KDebug>
#include <KLocale>
#include <KSharedConfig>
#include <KColorScheme>
#include <KGlobalSettings>
#include <KConfigDialog>
#include <KCalendarSystem>
#include <KServiceTypeTrader>
#include <KRun>
#include <KRun>
#include <Plasma/Theme>
#include <Plasma/Dialog>
#include <Plasma/Svg>
#include <Plasma/PaintUtils>
#include <Plasma/ToolTipManager>
#include <unistd.h>


DateTimeGroup::DateTimeGroup(QObject *parent, const QVariantList &args)
	: Plasma::Applet(parent, args),
	m_plainDateTimeGroupFont(KGlobalSettings::generalFont()),
	m_useCustomColor(false),
	m_useCustomShadowColor(false),
	m_drawShadow(true),
	m_rounded(false),
	m_natoOperation(false),
	m_local(false),
	m_format(0),
	m_sAfterDay(false),
	m_sAfterHour(false),
	m_sAfterMinute(false),
	m_sAfterSecond(false),
	m_sAfterTimeZone(false),
	m_sAfterMonth(false),
	m_layout(0),
	m_svg(0),
	tzBefNatoOp("Local"),
	curtz("Local")
{
	setHasConfigurationInterface(true);
	resize(250, 50);
}

DateTimeGroup::~DateTimeGroup()
{
}
bool DateTimeGroup::isLocalTimezone() const{
	return curtz=="Local";
}
QString DateTimeGroup::currentTimezone() const{
	return curtz;
}
QTime DateTimeGroup::lastTimeSeen() const{
	return m_lastTimeSeen; 
}
void DateTimeGroup::resetLastTimeSeen(){
	m_lastTimeSeen = QTime();
}
int DateTimeGroup::updateInterval() const{
	return m_showSeconds ? 1000 : 60000;
}
void DateTimeGroup::init(){
	configChanged();
	dataEngine("datetimegroup")->connectSource(currentTimezone(), this, updateInterval(), intervalAlignment() );
	connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColors()));
	connect(KGlobalSettings::self(), SIGNAL(appearanceChanged()), SLOT(resetSize()));
}

void DateTimeGroup::constraintsEvent(Plasma::Constraints constraints)
{
	if (constraints & Plasma::SizeConstraint ||
		constraints & Plasma::FormFactorConstraint) {
		updateSize();
	}
}

void DateTimeGroup::resetSize()
{
	constraintsEvent(Plasma::SizeConstraint);
}

void DateTimeGroup::updateSize()
{
	Plasma::FormFactor f = formFactor();
	
	if (f != Plasma::Vertical && f != Plasma::Horizontal) {
		const QFontMetricsF metrics(KGlobalSettings::smallestReadableFont());
		setMinimumSize(metrics.size(Qt::TextSingleLine, dtg));
	}
	
	int aspect = 12;
	
	int w, h;
	
	w = (int)(contentsRect().height() * aspect);
	h = (int)(contentsRect().width() / aspect);
	
	if (f == Plasma::Horizontal) {
		setMinimumSize(QSize(w, 0));
	} else {
		setMinimumSize(QSize(0, h));
	}
	
	setPreferredSize(QSize(w, h));
	emit sizeHintChanged(Qt::PreferredSize);
	
	if (m_isDefaultFont) {
		expandFontToMax(m_plainDateTimeGroupFont, dtg);
	}
	
	generatePixmap();
}

void DateTimeGroup::configChanged()
{
	
	KConfigGroup cg = config();

	if (isUserConfiguring()) {
		configAccepted();
	}
	m_natoOperation = cg.readEntry("natoOperation", m_natoOperation);
	m_local= cg.readEntry("localTime", m_local);

	QFont f = cg.readEntry("plainDateTimeGroupFont", m_plainDateTimeGroupFont);
	m_isDefaultFont = f == m_plainDateTimeGroupFont;
	m_plainDateTimeGroupFont = f;
	
	m_useCustomColor = cg.readEntry("useCustomColor", m_useCustomColor);
	m_plainDateTimeGroupColor = cg.readEntry("plainDateTimeGroupColor", m_plainDateTimeGroupColor);
	m_useCustomShadowColor = cg.readEntry("useCustomShadowColor", m_useCustomShadowColor);
	m_plainDateTimeGroupShadowColor = cg.readEntry("plainDateTimeGroupShadowColor", m_plainDateTimeGroupShadowColor);
	
	m_drawShadow = cg.readEntry("plainDateTimeGroupDrawShadow", m_drawShadow);
	m_rounded=cg.readEntry("rounded",m_rounded);
	m_format=cg.readEntry("format",m_format);
	
	m_showSeconds=m_format==2;
	if (m_showSeconds) {
		//We don't need to cache the applet if it update every seconds
		setCacheMode(QGraphicsItem::NoCache);
	} else {
		setCacheMode(QGraphicsItem::DeviceCoordinateCache);
	}
	
	m_sAfterDay =cg.readEntry("sAfterDay", m_sAfterDay);
	m_sAfterHour = cg.readEntry("sAfterHour", m_sAfterHour);
	m_sAfterMinute = cg.readEntry("sAfterMinute", m_sAfterMinute);
	
	m_sAfterSecond = cg.readEntry("sAfterSecond", m_sAfterSecond);
	m_sAfterTimeZone = cg.readEntry("sAfterTimeZone", m_sAfterTimeZone);
	m_sAfterMonth = cg.readEntry("sAfterMonth", m_sAfterMonth);
	
	updateColors();
	
	if (m_useCustomColor) {
		m_pixmap = QPixmap();
		delete m_svg;
		m_svg = 0;
	}
	
	const QFontMetricsF metrics(KGlobalSettings::smallestReadableFont());
	setMinimumSize(metrics.size(Qt::TextSingleLine, dtg));
	updateSize();
}

QString DateTimeGroup::setDtg(const Plasma::DataEngine::Data &data){
	switch(m_format){
		case 0:dtg = 
			data["Day"].toString()+
			(m_sAfterDay?" ":"")
			+data["Hour"].toString()+
			(m_sAfterHour?" ":"")
			+data["Min"].toString()+
			(m_sAfterMinute?" ":"")
			+(m_local?"J":data[(m_rounded?"TimeZoneRounded":"TimeZone")].toString())+
			(m_sAfterTimeZone?" ":"")
			+data["Month"].toString()+
			(m_sAfterMonth?" ":"")
			+data["YearS"].toString(); 
			break;
		case 1:dtg = data["Hour"].toString()+
			(m_sAfterHour?" ":"")
			+data["Min"].toString()+
			(m_sAfterMinute?" ":"")
			+(m_local?"J":data[(m_rounded?"TimeZoneRounded":"TimeZone")].toString());
			break;
		case 2:dtg = data["Day"].toString()+
			(m_sAfterDay?" ":"")
			+data["Hour"].toString()+
			(m_sAfterHour?" ":"")
			+data["Min"].toString()+
			(m_sAfterMinute?" ":"")
			+data["Seconds"].toString()+
			(m_sAfterSecond?" ":"")
			+(m_local?"J":data[(m_rounded?"TimeZoneRounded":"TimeZone")].toString())+
			(m_sAfterTimeZone?" ":"")
			+data["Month"].toString()+
			(m_sAfterMonth?" ":"")
			+data["YearL"].toString();
	}
	
	return dtg;
}
void DateTimeGroup::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
	Q_UNUSED(source);
	QTime m_time = data["Time"].toTime();
	KConfigGroup cg= config();
	kDebug()<<cg.entryMap();

	if ((m_showSeconds && m_time.second() != lastTimeSeen().second()) ||
		m_time.minute() != lastTimeSeen().minute()) {
		setDtg(data);
		m_lastTimeSeen = data["Time"].toTime();
		generatePixmap();
		update();
	}
}
void DateTimeGroup::createConfigurationInterface(KConfigDialog *parent)
{
	QWidget *widget = new QWidget();
	ui.setupUi(widget);
	parent->addPage(widget, i18n("Appearance"), "view-media-visualization");

	ui.localTime->setChecked(m_local);
	ui.natoOperation->setChecked(m_natoOperation);
	
	ui.plainDateTimeGroupFontBold->setChecked(m_plainDateTimeGroupFont.bold());
	ui.plainDateTimeGroupFontItalic->setChecked(m_plainDateTimeGroupFont.italic());
	ui.plainDateTimeGroupFont->setCurrentFont(m_plainDateTimeGroupFont);
	ui.useCustomColor->setChecked(m_useCustomColor);
	
	ui.plainDateTimeGroupColor->setColor(m_plainDateTimeGroupColor);
	ui.drawShadow->setChecked(m_drawShadow);
	ui.useCustomShadowColor->setChecked(m_useCustomShadowColor);
	ui.plainDateTimeGroupShadowColor->setColor(m_plainDateTimeGroupShadowColor);
	ui.drawShadow->setChecked(m_drawShadow);
	ui.sAfterDay->setChecked(m_sAfterDay);
	
	ui.sAfterHour->setChecked(m_sAfterHour);
	ui.sAfterMinute->setChecked(m_sAfterMinute);
	ui.sAfterSecond->setChecked(m_sAfterSecond);
	ui.sAfterMonth->setChecked(m_sAfterMonth);
	ui.sAfterTimeZone->setChecked(m_sAfterTimeZone);
	
	switch(m_format){
		case 1:ui.shortFormat->setChecked(true); 
			configSpaceForShort(true);
			break;
		case 2:ui.longFormat->setChecked(true); 
			configSpaceForLong(true);
			break;
		default:ui.stanFormat->setChecked(true);
			configSpaceForStan(true);
	}
	
	QWidget *widgettz = new QWidget();
	timezonesUi.setupUi(widgettz);
	timezonesUi.searchLine->addTreeWidget(timezonesUi.timeZones);
	timezonesUi.fullLetters->setChecked(m_rounded);
	emit timezonesUi.timeZones->hideColumn(4);
	emit timezonesUi.timeZones->toggleLetters(m_rounded);
	parent->addPage(widgettz, i18n("Time Zones"), "preferences-desktop-locale");
	
	connect(ui.drawShadow, SIGNAL(toggled(bool)), this, SLOT(configDrawShadowToggled(bool)));
	connect(ui.natoOperation, SIGNAL(toggled(bool)), this, SLOT(configSpecialZonesToggle()));
	connect(timezonesUi.fullLetters, SIGNAL(toggled(bool)), timezonesUi.timeZones, SLOT(toggleLetters(bool)));
	connect(ui.localTime, SIGNAL(toggled(bool)), this, SLOT(configSpecialZonesToggle()));
	connect(ui.longFormat, SIGNAL(toggled(bool)), this, SLOT(configSpaceForLong(bool)));
	connect(ui.shortFormat, SIGNAL(toggled(bool)), this, SLOT(configSpaceForShort(bool)));
	connect(ui.stanFormat, SIGNAL(toggled(bool)), this, SLOT(configSpaceForStan(bool)));
	connect(timezonesUi.timeZones, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(uncheckTimeBoxes()));

	configDrawShadowToggled(m_drawShadow);
	configSpecialZonesToggle();
	
	connect(timezonesUi.timeZones, SIGNAL(itemClicked(QTreeWidgetItem*,int)), parent, SLOT(settingsModified()));
	connect(ui.plainDateTimeGroupFont, SIGNAL(currentFontChanged(QFont)),parent, SLOT(settingsModified()));
	connect(ui.plainDateTimeGroupFontBold, SIGNAL(stateChanged(int)),parent, SLOT(settingsModified()));
	connect(ui.plainDateTimeGroupFontItalic, SIGNAL(stateChanged(int)),parent, SLOT(settingsModified()));
	connect(ui.useCustomColor, SIGNAL(stateChanged(int)),parent, SLOT(settingsModified()));
	
	connect(ui.plainDateTimeGroupColor, SIGNAL(changed(QColor)),parent, SLOT(settingsModified()));
	connect(ui.drawShadow, SIGNAL(stateChanged(int)),parent, SLOT(settingsModified()));
	connect(ui.useCustomShadowColor, SIGNAL(stateChanged(int)),parent, SLOT(settingsModified()));
	connect(ui.plainDateTimeGroupShadowColor, SIGNAL(changed(QColor)), parent, SLOT(settingsModified()));
	connect(ui.natoOperation, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
	
	connect(ui.localTime, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
	connect(ui.shortFormat, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
	connect(ui.stanFormat, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
	connect(ui.longFormat, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
	connect(ui.sAfterDay, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
	
	connect(ui.sAfterHour, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
	connect(ui.sAfterMinute, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
	connect(ui.sAfterSecond, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
	connect(ui.sAfterTimeZone, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
	connect(ui.sAfterMonth, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
	connect(timezonesUi.fullLetters, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
}
void DateTimeGroup::configSpaceForLong(bool value){
	ui.sAfterSecond->setEnabled(value);	
}
void DateTimeGroup::uncheckTimeBoxes(){
	ui.natoOperation->setChecked(false);
	ui.localTime->setChecked(false);
}
void DateTimeGroup::configSpaceForShort(bool value){
	ui.sAfterSecond->setEnabled(!value);
	ui.sAfterTimeZone->setEnabled(!value);
	ui.sAfterMonth->setEnabled(!value);
	ui.sAfterDay->setEnabled(!value);
}
void DateTimeGroup::configSpaceForStan(bool value){
	ui.sAfterSecond->setEnabled(!value);
}
void DateTimeGroup::configDrawShadowToggled(bool value){
	ui.useCustomShadowColor->setEnabled(value);
	ui.customShadowColorLabel->setEnabled(value);
	ui.plainDateTimeGroupShadowColor->setEnabled(value && ui.useCustomShadowColor->isChecked());
}

void DateTimeGroup::configSpecialZonesToggle()
{
// 	kDebug()<<"nato:"<<ui.natoOperation->isChecked()<<"local"<<ui.localTime->isChecked();
	ui.localTime->setEnabled(!ui.natoOperation->isChecked());
	ui.natoOperation->setEnabled(!ui.localTime->isChecked());
}
Plasma::IntervalAlignment DateTimeGroup::intervalAlignment() const
{
	return m_showSeconds ? Plasma::NoAlignment : Plasma::AlignToMinute;
}
void DateTimeGroup::configAccepted()
{
	bool m_formatChanged;
	KConfigGroup cg = config();
	
	if (m_isDefaultFont && ui.plainDateTimeGroupFont->currentFont() != m_plainDateTimeGroupFont) {
		m_isDefaultFont = false;
	}
	
	m_plainDateTimeGroupFont = ui.plainDateTimeGroupFont->currentFont();
	
	if(m_sAfterDay!=ui.sAfterDay->isChecked()){
		m_sAfterDay = !m_sAfterDay;
		cg.writeEntry("sAfterDay", m_sAfterDay);
		m_formatChanged=true;
	}
	if(m_sAfterHour!=ui.sAfterHour->isChecked()){
		m_sAfterHour = !m_sAfterHour;
		cg.writeEntry("sAfterHour", m_sAfterHour);
		m_formatChanged=true;
	}
	
	if(m_sAfterMinute!=ui.sAfterMinute->isChecked()){
		m_sAfterMinute =!m_sAfterMinute;
		cg.writeEntry("sAfterMinute", m_sAfterMinute);	
		m_formatChanged=true;
	}
	
	if(m_sAfterSecond!=ui.sAfterSecond->isChecked()){
		m_sAfterSecond =!m_sAfterSecond;
		cg.writeEntry("sAfterSecond", m_sAfterSecond);
		m_formatChanged=true;
	}
	
	if(m_sAfterTimeZone!=ui.sAfterTimeZone->isChecked()){
		m_sAfterTimeZone = !m_sAfterTimeZone;
		cg.writeEntry("sAfterTimeZone", m_sAfterTimeZone);
		m_formatChanged=true;
	}
	
	if(m_sAfterMonth!=ui.sAfterMonth->isChecked()){
		m_sAfterMonth = !m_sAfterMonth;
		cg.writeEntry("sAfterMonth", m_sAfterMonth);
		m_formatChanged=true;
	}
	if(m_rounded!=timezonesUi.fullLetters->isChecked()){
		m_rounded = !m_rounded;
		cg.writeEntry("rounded", m_rounded);
		m_formatChanged=true;
	}
	
	if(m_natoOperation!= ui.natoOperation->isChecked()){
		m_natoOperation=!m_natoOperation;
		cg.writeEntry("natoOperation", m_natoOperation);
		tzBefNatoOp=cg.readEntry("currentTimezone");
		changeEngineTimezone( (m_natoOperation?"UTC":tzBefNatoOp)); 
	}

	if(m_local!= ui.localTime->isChecked()){
		m_local=!m_local;
		cg.writeEntry("localTime", m_local);
		tzBefNatoOp=m_natoOperation?"UTC":cg.readEntry("currentTimezone");
		changeEngineTimezone( (m_local?"Local":tzBefNatoOp)); 
	}
	if (timezonesUi.timeZones->selection()!=curtz && !m_local && !m_natoOperation) {
		changeEngineTimezone(timezonesUi.timeZones->selection());
		cg.writeEntry("currentTimezone", timezonesUi.timeZones->selection());
	}
	if(ui.shortFormat->isChecked()&&m_format!=1){
		int oldm_format=m_format;
		m_format=1;
		m_formatChanged=false;
		if(oldm_format==0)
			setDtg(dataEngine("datetimegroup")->query(currentTimezone()));
	}
	else if(ui.longFormat->isChecked()&&m_format!=2){
		m_format=2;
	}
	else if(ui.stanFormat->isChecked()&&m_format!=0){
		int oldm_format=m_format;
		m_format=0;
		m_formatChanged=false;
		if(oldm_format==1)
			setDtg(dataEngine("datetimegroup")->query(currentTimezone()));
	}
	
	cg.writeEntry("format",m_format);
	if (m_showSeconds != (m_format==2)) {
		m_showSeconds = !m_showSeconds;
		
		if (m_showSeconds) {
			//We don't need to cache the applet if it update every second
			setCacheMode(QGraphicsItem::NoCache);
		} else {
			setCacheMode(QGraphicsItem::DeviceCoordinateCache);
		}
		
		changeEngineTimezone(currentTimezone());
	}
	if(m_formatChanged)
		setDtg(dataEngine("datetimegroup")->query(currentTimezone()));
		
	m_useCustomColor = ui.useCustomColor->isChecked();
	cg.writeEntry("useCustomColor", m_useCustomColor);
	if (m_useCustomColor) {
		m_plainDateTimeGroupColor = ui.plainDateTimeGroupColor->color();
		cg.writeEntry("plainDateTimeGroupColor", m_plainDateTimeGroupColor);
		m_pixmap = QPixmap();
		delete m_svg;
		m_svg = 0;
	} else {
		m_plainDateTimeGroupColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
	}
	
	m_useCustomShadowColor = ui.useCustomShadowColor->isChecked();
	cg.writeEntry("useCustomShadowColor", m_useCustomShadowColor);
	if (m_useCustomShadowColor) {
		m_plainDateTimeGroupShadowColor = ui.plainDateTimeGroupShadowColor->color();
		cg.writeEntry("plainDateTimeGroupShadowColor", m_plainDateTimeGroupShadowColor);
	} else {
		m_plainDateTimeGroupShadowColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
	}
	m_drawShadow = ui.drawShadow->isChecked();
	cg.writeEntry("plainDateTimeGroupDrawShadow", m_drawShadow);
	
	m_plainDateTimeGroupFont.setBold(ui.plainDateTimeGroupFontBold->checkState() == Qt::Checked);
	m_plainDateTimeGroupFont.setItalic(ui.plainDateTimeGroupFontItalic->checkState() == Qt::Checked);
	cg.writeEntry("plainDateTimeGroupFont", m_plainDateTimeGroupFont);
	
	constraintsEvent(Plasma::SizeConstraint);
	generatePixmap();
	update();
	
	emit sizeHintChanged(Qt::PreferredSize);
	emit configNeedsSaving();
}

void DateTimeGroup::changeEngineTimezone(const QString &newTimezone)
{
	resetLastTimeSeen();
	dataEngine("datetimegroup")->disconnectSource(currentTimezone(), this);  
	dataEngine("datetimegroup")->connectSource(newTimezone, this, updateInterval(), intervalAlignment());
	curtz=newTimezone;
}



void DateTimeGroup::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
	Q_UNUSED(option);
	if (dtg.isEmpty()) {
		return;
	}
	
	p->setPen(QPen(m_plainDateTimeGroupColor));
	p->setRenderHint(QPainter::SmoothPixmapTransform);
	p->setRenderHint(QPainter::Antialiasing);
	
	QFont smallFont = KGlobalSettings::smallestReadableFont();
	
	dtgRect = contentsRect;
	
	m_plainDateTimeGroupFont.setPointSizeF(qMax(dtgRect.height(), KGlobalSettings::smallestReadableFont().pointSize()));
	preparePainter(p, dtgRect, m_plainDateTimeGroupFont, dtg, true);
	
	if (m_useCustomColor || !m_svgExistsInTheme) {
		QFontMetrics fm(p->font());
		
		QPointF timeTextOrigin(QPointF(qMax(0, (dtgRect.center().x() - fm.width(dtg) / 2)),
						(dtgRect.center().y() + fm.height() / 3)));
		p->translate(-0.5, -0.5);
		
		if (m_drawShadow) {
			QPen tmpPen = p->pen();
			
			qreal shadowOffset = 1.0;
			QPen shadowPen;
			QColor shadowColor = m_plainDateTimeGroupShadowColor;
			shadowColor.setAlphaF(.4);
			shadowPen.setColor(shadowColor);
			p->setPen(shadowPen);
			QPointF shadowTimeTextOrigin = QPointF(timeTextOrigin.x() + shadowOffset,
								timeTextOrigin.y() + shadowOffset);
			p->drawText(shadowTimeTextOrigin, dtg);
			
			p->setPen(tmpPen);
			
			QLinearGradient gradient = QLinearGradient(QPointF(0, 0), QPointF(0, fm.height()));
			
			QColor startColor = m_plainDateTimeGroupColor;
			startColor.setAlphaF(.95);
			QColor stopColor = m_plainDateTimeGroupColor;
			stopColor.setAlphaF(.7);
			
			gradient.setColorAt(0.0, startColor);
			gradient.setColorAt(0.5, stopColor);
			gradient.setColorAt(1.0, startColor);
			QBrush gradientBrush(gradient);
			
			QPen gradientPen(gradientBrush, tmpPen.width());
			p->setPen(gradientPen);
		}
		p->drawText(timeTextOrigin, dtg);
	} else {
		QRect adjustedTimeRect = m_pixmap.rect();
		adjustedTimeRect.moveCenter(dtgRect.center());
		p->drawPixmap(adjustedTimeRect, m_pixmap);
	}
}

void DateTimeGroup::generatePixmap()
{
	if (m_useCustomColor || !m_svgExistsInTheme) {
		return;
	}
	
	if (!m_svg) {
		m_svg = new Plasma::Svg(this);
		m_svg->setImagePath("widgets/labeltexture");
		m_svg->setContainsMultipleImages(true);
	}
	
	QRect rect(contentsRect().toRect());
	QFont font(m_plainDateTimeGroupFont);
	prepareFont(font, rect, dtg, true);
	m_pixmap = Plasma::PaintUtils::texturedText(dtg, font, m_svg);
}

void DateTimeGroup::expandFontToMax(QFont &font, const QString &text)
{
	bool first = true;
	const QRect rect = contentsRect().toRect();
	
	do {
		if (first) {
			first = false;
		} else  {
			font.setPointSize(font.pointSize() + 1);
		}
		
		const QFontMetrics fm(font);
		QRect fr = fm.boundingRect(rect, Qt::TextSingleLine, text);
		if (fr.width() >= rect.width() || fr.height() >= rect.height()) {
			break;
		}
	} while (true);
}

void DateTimeGroup::prepareFont(QFont &font, QRect &rect, const QString &text, bool singleline)
{
	QRect tmpRect;
	bool first = true;
	const int smallest = KGlobalSettings::smallestReadableFont().pointSize();
	
	do {
		if (first) {
			first = false;
		} else  {
			font.setPointSize(qMax(smallest, font.pointSize() - 1));
		}
		
		const QFontMetrics fm(font);
		int flags = (singleline || ((formFactor() == Plasma::Horizontal) && (contentsRect().height() < font.pointSize()*6))) ?
		Qt::TextSingleLine : Qt::TextWordWrap;
		
		tmpRect = fm.boundingRect(rect, flags, text);
	} while (font.pointSize() > smallest &&
	(tmpRect.width() > rect.width() || tmpRect.height() > rect.height()));
	
	rect = tmpRect;
}

QRect DateTimeGroup::preparePainter(QPainter *p, const QRect &rect, const QFont &font, const QString &text, bool singleline)
{
	QRect tmpRect = rect;
	QFont tmpFont = font;
	
	prepareFont(tmpFont, tmpRect, text, singleline);
	
	p->setFont(tmpFont);
	
	return tmpRect;
}


void DateTimeGroup::updateColors()
{
	if (!m_useCustomColor) {
		m_plainDateTimeGroupColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
	}
	
	if (!m_useCustomShadowColor) {
		m_plainDateTimeGroupShadowColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
	}
	
	if (!m_useCustomColor || !m_useCustomShadowColor) {
		update();
	}
}


#include "datetimegroup.moc"
