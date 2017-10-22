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

	virtual ~RotaryphoneWeb() {}

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
			_cids.clear();
			for (auto &x : _cid_to_answer) {
				_cids.insert(x.first);
			}
			_cid_to_answer.clear();
			return true;
		}
		if (name == "answer") {
			if (cid == _leader) return false;

			_cid_to_answer[cid] = atoi(parameters[0].c_str());
		}
		return false;
	}

	virtual bool get_value(const ClientID& cid, int state,
			       const string& name,
			       const vector<string>& parameters,
			       const map<string, string>& arguments,
			       string* output) {
		if (name == "answer") {
			if (cid == _leader) {
				*output = "";
				return false;
			}
			if (!_cid_to_answer.count(cid)) {
				*output = "";
				return true;
			}
			*output = Logger::stringify("%", _cid_to_answer[cid]);
			return true;
		}
		if (name == "results") {
			if (cid != _leader) {
				*output = "";
				return false;
			}
			stringstream ss;
			ss << _cid_to_answer.size() << endl;
			ss << _cids.size() << endl;

			map<int, int> tally;
			for (const auto &x : _cid_to_answer) {
				tally[x.second]++;
			}

			for (int i = 0; i < 5; ++i) {
				ss << tally[i] << endl;
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
		_cids.insert(cid);
	}

	virtual void bye_client(const ClientID& cid) {
		_cids.erase(cid);
	}

protected:
	virtual bool set_leader(const ClientID& cid) {
		if (_no_leader == true) {
			_leader = cid;
			Logger::info("leader: %", _leader);
			_no_leader = false;
			_cids.erase(cid);
			return true;
		}
		return cid == _leader;
	}

	virtual void cleanup(const ClientID& cid) {
	}

	bool _no_leader;
	ClientID _leader;
	map<ClientID, int> _cid_to_answer;
	set<ClientID> _cids;

	unique_ptr<ScaffoldNode> _leader_node;
	unique_ptr<ScaffoldNode> _follower_node;
};

}  // namespace minibus

#endif  // __ROTARYPHONE__WEB__ROTARYPHONE_WEB__H__
