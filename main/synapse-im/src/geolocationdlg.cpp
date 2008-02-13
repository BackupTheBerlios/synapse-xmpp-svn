#include "geolocationdlg.h"
#include "mapcontrol.h"
#include <QWidget>

#include "psiaccount.h"
#include "userlist.h"
#include "pepmanager.h"

GeolocationDlg::GeolocationDlg(const XMPP::Jid &j, PsiAccount *_pa, QWidget *parent)
:QDialog(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);
	pa = _pa;

	setupUi(this);
	le_alt->setText("0");
	QHBoxLayout *hbox = new QHBoxLayout(f_map);
	map = new MapControl(QSize(416,316), MapControl::Panning, f_map);
	hbox->addWidget(map);
	map->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	connect(map, SIGNAL(mouseEventCoordinate(const QMouseEvent*, const QPointF)), this, SLOT(updatedGeometry()));
	connect(pb_close, SIGNAL(clicked()), this, SLOT(close()));

	pa->dialogRegister(this, j);

	if(pa->jid().bare() != j.bare()) {
		setReadOnly();
		pb_retract->hide();
		pb_publish->hide();
	} else {
		connect(pb_publish, SIGNAL(clicked()), this, SLOT(publish()));
		connect(pb_retract, SIGNAL(clicked()), this, SLOT(retract()));
	}
	update(j);
}

GeolocationDlg::~GeolocationDlg()
{
	pa->dialogUnregister(this);
	delete map;
}

void GeolocationDlg::update(const XMPP::Jid &j)
{
	jid = j;

	UserListItem *u = pa->find(jid);

	if(u) {
		geo = u->geoLocation();

		sb_floor->setValue(geo.floor());
		le_street->setText(geo.street());
		le_locality->setText(geo.locality());
		le_country->setText(geo.country());
		le_postalcode->setText(geo.postalcode());

		if(geo.alt().hasValue())
			le_alt->setText(QString("%1").arg(geo.alt().value()));

		if(geo.lat().hasValue() && geo.lon().hasValue()) {
			map->setView(QPointF(geo.lon().value(), geo.lat().value()));
			le_lat->setText(QString("%1").arg(geo.lat().value()));
			le_lon->setText(QString("%1").arg(geo.lon().value()));
			map->setZoom(8);
		} else {
			map->setZoom(2);
			pb_retract->hide();
		}

		le_desc->setText(geo.description());
	}
}

void GeolocationDlg::retract()
{
	pa->pepManager()->retract("http://jabber.org/protocol/geoloc", "current");
	close();
}

void GeolocationDlg::publish()
{
	geo.setFloor(sb_floor->value());
	geo.setStreet(le_street->text());
	geo.setLocality(le_locality->text());
	geo.setCountry(le_country->text());
	geo.setPostalcode(le_postalcode->text());

	geo.setAlt(le_alt->text().toInt());

	QPointF coord = map->getCurrentCoordinate();
	geo.setLat(coord.y());
	geo.setLon(coord.x());

	geo.setDescription(le_desc->text());

	pa->pepManager()->publish("http://jabber.org/protocol/geoloc", PubSubItem("current",geo.toXml((*pa->client()->rootTask()->doc()))));
	close();
}

void GeolocationDlg::setReadOnly()
{
	map->setMouseMode(MapControl::None);
}

void GeolocationDlg::updatedGeometry()
{
	QPointF point = map->getCurrentCoordinate();
	le_lat->setText(QString("%1").arg(point.y()));
	le_lon->setText(QString("%1").arg(point.x()));
}

