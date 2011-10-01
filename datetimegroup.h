/***************************************************************************
 *   Copyright (C) 2007-2008 by Riccardo Iaconelli <riccardo@kde.org>      *
 *   Copyright (C) 2007-2008 by Sebastian Kuegler <sebas@kde.org>          *
 *                                                                         *
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

#ifndef DATETIMEGROUP_H
#define DATETIMEGROUP_H

#include <QtCore/QTime>
#include <Plasma/Applet>
#include <Plasma/DataEngine>
#include <Plasma/Dialog>

#include "ui_dtgConfig.h"
#include "ui_timezonesConfig.h"
namespace Plasma
{
    class Svg;
}

class DateTimeGroup : public Plasma::Applet
{
    Q_OBJECT
    public:
        DateTimeGroup(QObject *parent, const QVariantList &args);
        ~DateTimeGroup();
        bool isLocalTimezone() const;
	QString currentTimezone() const;
        void init();
        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect);

    public slots:
        void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
        void updateColors();

    protected slots:
        void configAccepted();
        void configChanged();
        void constraintsEvent(Plasma::Constraints constraints);
        void resetSize();
    protected:
	QTime lastTimeSeen() const;
        void resetLastTimeSeen();
        void createConfigurationInterface(KConfigDialog *parent);
        void changeEngineTimezone(const QString &newTimezone);

    private slots:
        void configDrawShadowToggled(bool value);
	void configSpaceForLong(bool value);
	void configSpaceForStan(bool value);
	void configSpaceForShort(bool value);
	void configSpecialZonesToggle();
	void uncheckTimeBoxes();
    private:
        void updateSize();
        void generatePixmap();
        QRect preparePainter(QPainter *p, const QRect &rect, const QFont &font, const QString &text, bool singleline = false);
        void prepareFont(QFont &font, QRect &rect, const QString &text, bool singleline);
        void expandFontToMax(QFont &font, const QString &text);
	Plasma::IntervalAlignment intervalAlignment() const;
	QString setDtg(const Plasma::DataEngine::Data &data);
	
        QFont m_plainDateTimeGroupFont;
        bool m_isDefaultFont;
        bool m_useCustomColor;
        QColor m_plainDateTimeGroupColor;
        bool m_useCustomShadowColor;
        QColor m_plainDateTimeGroupShadowColor;
        bool m_drawShadow;
        QRect dtgRect;
	int updateInterval() const;
        bool m_showSeconds;
	bool m_rounded;
        bool m_natoOperation;
	bool m_local;
	int m_format;
	QTime m_lastTimeSeen;
	bool m_sAfterDay;
	bool m_sAfterHour;
	bool m_sAfterMinute;
	bool m_sAfterSecond;
	bool m_sAfterTimeZone;
	bool m_sAfterMonth;

        QString dtg;


        QVBoxLayout *m_layout;
        /// Designer Config files
        Ui::dtgConfig ui;
	Ui::timezonesConfig timezonesUi;
        Plasma::Svg *m_svg;
        bool m_svgExistsInTheme;
        QPixmap m_pixmap;
	QString tzBefNatoOp;
	QString curtz;

	
};

K_EXPORT_PLASMA_APPLET(datetimegroup, DateTimeGroup)

#endif
