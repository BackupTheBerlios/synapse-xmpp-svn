include_directories(
	../cutestuff/util
	../cutestuff/network
	../third-party/qca/include/QtCrypto
)

set(cutestuff_SRCS
	../cutestuff/util/bytestream.cpp
	../cutestuff/network/bsocket.cpp
	../cutestuff/network/httpconnect.cpp
	../cutestuff/network/httppoll.cpp
	../cutestuff/network/socks.cpp
)

set(cutestuff_H
	../cutestuff/util/bytestream.h
	../cutestuff/network/bsocket.h
	../cutestuff/network/httpconnect.h
	../cutestuff/network/httppoll.h
	../cutestuff/network/socks.h
)

#if (!irisnet)

include_directories(
	../cutestuff/legacy
)

set(cutestuff_SRCS ${cutestuff_SRCS}
	../cutestuff/legacy/safedelete.cpp
	../cutestuff/legacy/ndns.cpp
	../cutestuff/legacy/srvresolver.cpp
	../cutestuff/legacy/servsock.cpp
)

set(cutestuff_H
	${cutestuff_H}
	../cutestuff/legacy/safedelete.h
	../cutestuff/legacy/ndns.h
	../cutestuff/legacy/srvresolver.h
	../cutestuff/legacy/servsock.h
)

#endif (!irisnet)