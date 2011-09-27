#include <iostream>

#include "client.h"

extern "C" {
#include <signal.h>
}

#include <ctime>

using namespace std;
using namespace net01;

int main(int argc, char *argv[]) {
	
	client cl;
	
	try {
		cl.loop();
	}
	catch(int e) {
		cout << "error in client: " << strerror(e) << endl;
	}

}