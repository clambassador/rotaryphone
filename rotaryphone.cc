#include "ib/logger.h"
#include "rotaryphone/rotaryphone_web.h"

#include "centipede/types.h"
#include "centipede/webserver.h"
#include "centipede/backend/i_webserver_backend.h"
#include "centipede/nodes/scaffold_node.h"

#include <map>
#include <string>
#include <vector>

using namespace rotaryphone;
using namespace std;

int main(int argc, char** argv) {
	Config::_()->load("rotaryphone.cfg");

	RotaryphoneWeb rw;
	WebServer webserver(&rw);
	webserver.start_server(Config::_()->get("http_port"));

	cout << Config::_()->get("http_port") << endl;
	cout << "Running.\nHit any key to stop.";

	getchar();
	webserver.stop_server();
}
