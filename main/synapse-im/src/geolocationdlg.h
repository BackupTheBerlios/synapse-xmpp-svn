#include "ui_geolocation.h"
#include "geolocation.h"

#include "xmpp_jid.h"

class MapControl;
class PsiAccount;

class GeolocationDlg : public QDialog, public Ui::Geolocation {
	Q_OBJECT
public:
	GeolocationDlg(const XMPP::Jid &j, PsiAccount *pa, QWidget *parent = 0);	
	~GeolocationDlg();

	void update(const XMPP::Jid &j);

public slots:
	void publish();
	void retract();
	void updatedGeometry();

private:
	void setReadOnly();

	MapControl *map;
	GeoLocation geo;
	PsiAccount *pa;
	XMPP::Jid jid;
};