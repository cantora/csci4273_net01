#ifndef SEQ_TABLE_H
#define SEQ_TABLE_H

#include <cerrno>

#include <iostream>
#include <vector>
#include <string>
#include <iterator>

/*
 * a table of character buffers
 * accessed by index
 */
namespace net01 {

class seq_table {
	public:
		
		virtual size_t size() const = 0;
		
		virtual void insert(const std::string &str) = 0;
		
		virtual std::istream_iterator<char> at(int index) = 0;
		virtual void at(int index, std::string *str) = 0;

	protected:

}; /* seq_table */

}; /* net01 */

/*
		typedef std::istream * table_type;
		//typedef std::vector<table_type>::const_iterator const_iterator;


//const_iterator begin() { return m_vec.begin();}
		//const_iterator end() { return m_vec.end();}
		
		//unsigned int index(const_iterator it) const {return it - m_vec.begin(); }
*/
#endif /* SEQ_TABLE_H */