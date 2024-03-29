#define BOOST_TEST_MODULE "net01:test_sess_srv"
#include <boost/test/unit_test.hpp>

#include <cerrno>

#include "msg.h"
#include <cstdlib>
#include <ctime>

using namespace net01;
using namespace std;


BOOST_AUTO_TEST_CASE(construct) {
	msg m;
	char data[] = "qwerqwerqwerqwerqwerqwerqwer";
	string str(data);
	msg m2(data);

	BOOST_REQUIRE_EQUAL(m.size(), 0);	
	BOOST_REQUIRE_EQUAL(m2.size(), sizeof(data)-1);	

	msg m3(m2);

	BOOST_REQUIRE(m2 == m3);

	msg m4 = m3;

	BOOST_REQUIRE(m4 == m3);

	//m.set_data(str);
	//BOOST_REQUIRE(m == m2);	
}

BOOST_AUTO_TEST_CASE(istrm) {
	int i;
	char data[] = "Again as your previous post the comment gives you the answer. copy algorithm takes three parameters - the first two are beginning and one past the end iterators of the source range and the third parameter is the beginning iterator of the destination range.";
	char data2[sizeof(data)];

	const msg m(data);
	char c;
	auto_ptr<istream> istr;

	for(int i = 0; i < 5; i++) {
		string str;

		istr = m.istrm();
		while( istr->good() ) {
			c = istr->get();
			if(istr->good() ) {
				str.push_back(c);
			}
		} 
	
		//cout << str << endl;
	
		string dstr(data);
		BOOST_REQUIRE_EQUAL(str.size(), dstr.size());
		BOOST_REQUIRE_EQUAL(str, dstr);
	}
}


BOOST_AUTO_TEST_CASE(ostrm) {
	auto_ptr<ostream> ostr;

	char data[] = "Again as your previous post the comment gives you the answer. copy algorithm takes three parameters - the first two are beginning and one past the end iterators of the source range and the third parameter is the beginning iterator of the destination range.";
	char data2[sizeof(data)];

	msg m(data);
	char c;
	auto_ptr<istream> istr;

	ostr = m.ostrm();

	*ostr << "blah blah";
	*ostr << "bleargh!" << endl << 45 << endl;

  
	for(int i = 0; i < 20; i++) {
		istr = m.istrm();

		char c = istr->get();
  
		while(istr->good() ) {
			cout << c;
			c = istr->get();
    	}
		cout << endl;
		*ostr << "ack ";
	}

}
