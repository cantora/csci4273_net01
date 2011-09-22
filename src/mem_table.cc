#include "mem_table.h"

using namespace std;
using namespace net01;

mem_table::~mem_table() {
	vector<istringstream *>::iterator it;

	for(it = m_vec.begin(); it != m_vec.end(); it++) {
		delete *it;
	}
}

istream_iterator<char> mem_table::at(int index) {
	assert(index >= 0);
	assert(index < m_vec.size());

	(*m_vec[index]).seekg(0);

	std::istream_iterator<char> it(*m_vec[index]);

	return it;
}

void mem_table::at(int index, string *str) {
	istream_iterator<char> itr, eos;

	assert(index >= 0);
	assert(index < m_vec.size() );

	for(itr = at(index); itr != eos; itr++) {
		str->push_back(*itr);
	}

}

void mem_table::insert(const std::string &str) {  
	m_vec.push_back(new std::istringstream(str) );
}

