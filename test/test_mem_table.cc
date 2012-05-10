#define BOOST_TEST_MODULE "net01:test_sess_srv"
#include <boost/test/unit_test.hpp>

#include <cerrno>

#include "mem_table.h"
#include <cstdlib>
#include <ctime>

using namespace net01;
using namespace std;

void insert_something(mem_table &mtb) {
	string s;
	s += 'a';
	s += 'b';
	s += 'c';

	mtb.insert(s);
}

BOOST_AUTO_TEST_CASE(insertabunch) {
	mem_table mtb;
	string s;
	std::istream_iterator<char> itr, eos;

	char buf[512];
	int count = 0;


	srand(time(NULL));
	mtb.insert("blalkjas;ldfjasdf"); count++;;
	mtb.insert(string("qwerqwer") + string("yuioyuioy") ); count++;

	insert_something(mtb); count++;

	for(int i = 0; i < 40; i++) {
		for(int j = 0; j < 15; j++) {
			s += (char) ((rand() % 40) + 36);
		}
		
		mtb.insert(s); count++;
		s = "";
	}

	cout << "table: " << endl;

	for(int i = 0; i < mtb.size(); i++) {
		cout << i << ":\t\"";
		for(itr = mtb.at(i); itr != eos; itr++)
			*itr;
		
		for(itr = mtb.at(i); itr != eos; itr++) cout << *itr;
		
		

		//string msg;
		//mtb.at(i, &msg);

		//cout << endl;cout << msg;
		cout << buf << "\"" << endl;
	} 

	BOOST_REQUIRE_EQUAL(mtb.size(), count);

	
}

