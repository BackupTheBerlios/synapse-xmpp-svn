set(DIR tools/mapcontrol)

include_directories(${DIR})


set(mapcontrol_SRCS
	${DIR}/curve.cpp
	${DIR}/geometry.cpp
	${DIR}/imagemanager.cpp
	${DIR}/layer.cpp
	${DIR}/layermanager.cpp
	${DIR}/linestring.cpp
	${DIR}/mapadapter.cpp
	${DIR}/mapcontrol.cpp
	${DIR}/mapnetwork.cpp
	${DIR}/point.cpp
	${DIR}/tilemapadapter.cpp
	${DIR}/imagepoint.cpp
	${DIR}/gps_position.cpp
	${DIR}/maplayer.cpp
	${DIR}/geometrylayer.cpp
	${DIR}/googlemapadapter.cpp
)

set(mapcontrol_H
	${DIR}/curve.h
	${DIR}/geometry.h
	${DIR}/imagemanager.h
	${DIR}/layer.h
	${DIR}/layermanager.h
	${DIR}/linestring.h
	${DIR}/mapadapter.h
	${DIR}/mapcontrol.h
	${DIR}/mapnetwork.h
	${DIR}/point.h
	${DIR}/tilemapadapter.h
	${DIR}/imagepoint.h
	${DIR}/gps_position.h
	${DIR}/maplayer.h
	${DIR}/geometrylayer.h
	${DIR}/googlemapadapter.h
)