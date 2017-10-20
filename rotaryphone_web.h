#ifndef __ROTARYPHONE__ROTARYPHONE_WEB__H__
#define __ROTARYPHONE__ROTARYPHONE_WEB__H__

#include <cstdlib>
#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "ib/config.h"
#include "ib/formatting.h"

#include "centipede/types.h"
#include "centipede/webserver.h"
#include "centipede/backend/i_webserver_backend.h"
#include "centipede/nodes/scaffold_node.h"

#include "labelgossip/format.h"
#include "labelgossip/manager.h"

using namespace centipede;
using namespace ib;
using namespace std;

namespace rotaryphone {

class RotaryphoneWeb : public IWebserverBackend {
public:
	RotaryphoneWeb() {
		_leader_node.reset(new ScaffoldNode(
				Config::_()->gets("leader_html_file")));
		_follower_node.reset(new ScaffoldNode(
				Config::_()->gets("follower_html_file")));
		_no_leader = true;
	}

	virtual void init(const Manager& mgr) {
	}

	virtual ~LabelgossipWeb() {}

	virtual void get_page(const ClientID& cid, int state,
	                      string* output) {
		BasePropertyPage bpp;
		bpp.set("cid", cid);
		if (set_leader(cid)) {
			*output = _leader_node.get()->display(&bpp);
		} else {
			*output = _follower_node.get()->display(&bpp);
		}

	}

        virtual void get_resource(const ClientID& cid,
                                  const ResourceID& rid,
                                  const string& ject,
                                  string* output) {
        }

	virtual bool set_value(const ClientID& cid, int state,
			       const string& name,
			       const vector<string>& parameters,
			       const map<string, string>& arguments) {
		if (name == "reset") {
			if (cid != _leader) return false;

			unique_lock<mutex> ul(_m);
			_cid_to_answer.clear();
			return true;
		}
		if (name == "answer") {
			if (cid == _leader) return false;

			unique_lock<mutex> ul(_m);
			_cid_to_answer[cid] = atoi(parameters[0]);
		}
		return false;
	}

	virtual bool get_value(const ClientID& cid, int state,
			       const string& name,
			       const vector<string>& parameters,
			       const map<string, string>& arguments,
			       string* output) {
		if (name == "results") {
			if (cid != _leader) {
				*output = "";
				return false;
			}
			stringstream ss;
			map<int, int> tally;
			for (const auto &x : _cid_to_answer) {
				tally[x.second]++;
			}

			*output = ss.str();
			return true;
		}
		Logger::error("unknown value %", name);
		*output = "";
		return false;
	}

	virtual int run_command(const ClientID& cid, int state,
	                        const string& command,
				const vector<string>& parameters,
				const map<string, string>& arguments) {
		return 0;
	}

        virtual void run_node_command(const ClientID&, int state,
                                      const string& node,
                                      const string& command,
                                      const vector<string>& parameters,
                                      const map<string, string>& arguments) {
	}

        virtual int recv_post(const ClientID&, const string& command,
                              const string& key, const string& filename,
                              const string& content_type, const string& encoding,
                              const string& data, uint64_t offset,
                              size_t size, string* output) {
		return 0;
	}

	virtual void new_client(const ClientID& cid) {
	}

	virtual void bye_client(const ClientID& cid) {
	}

protected:
	virtual void is_leader(const ClientID& cid) {
		unique_lock<mutex> _ul(_m);

		if (_no_leader == true) {
			_leader = cid;
			_no_leader = false;
			return true;
		}
		return false;
	}

	virtual void cleanup(const ClientID& cid) {
	}

	map<ClientID, Format*> _cid_to_format;
	map<ClientID, vector<string>> _cid_to_format_list;
	map<ClientID, unique_ptr<Range>> _cid_to_narrow;
	map<ClientID, unique_ptr<Range>> _cid_to_range;
	map<string, Format*> _fmts;
	vector<pair<string, Format*>> _sorted_formats;

	unique_ptr<ScaffoldNode> _node;
	mutex _m;

	bool _no_leader;
	ClientID _leader;
};

}  // namespace minibus

#endif  // __ROTARYPHONE__WEB__ROTARYPHONE_WEB__H__
