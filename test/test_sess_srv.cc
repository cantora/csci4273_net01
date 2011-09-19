#define BOOST_TEST_MODULE "net01:test_sess_srv"
#include <boost/test/unit_test.hpp>

#include <cerrno>

#include "sess_srv.h"

using namespace net01;

BOOST_AUTO_TEST_CASE(construct) {
	sess_srv ss(34);

	BOOST_REQUIRE_EQUAL(1, ss.rfd_set_size() );

	ss.add_rfd(56);
	BOOST_REQUIRE_EQUAL(2, ss.rfd_set_size() );
}

