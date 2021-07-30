#include <iostream>
#include <vector>
#include "httplib.h"
#include "Player.h"
#include "Ground.h"

using namespace std;
using namespace httplib;

#define pb push_back

int n, sz, cur = 1, blackout = -1;
vector <Player> V;

int main(){
	Server svr;

	cout << "Enter number of players :" << endl;
	cin >> n;
	cout << "Enter size of the ground :" << endl;
	cin >> sz;

	Ground G(sz, n);

	svr.Post("/register", [&](const Request &req, Response &res){
		bool flag = req.has_file("Register");
		if(!flag || (int)V.size() == n){
			res.set_content("Invalid Request", "Error");
		}
		else{
			const auto& file = req.get_file_value("Register");
			Player tmp;
			tmp.name = file.content;
			tmp.number = (int)V.size() + 1;
			cout << "Player number " << tmp.number << " registered with the name of " << tmp.name << endl;

			V.pb(tmp);
			string tmpres = "";
			tmpres += (char)(tmp.number + 48);
			res.set_content(tmpres, "Success");
		}
	});

	svr.Post("/check", [&](const Request &req, Response &res){
		bool flag = req.has_file("Check");
		if(!flag){
			res.set_content("Invalid Request", "Error");
		}
		else{
			const auto& file = req.get_file_value("Check");
			int tmp = (int)(file.content[0]) - 48;
			if((int)V.size() != n || tmp != cur){
				res.set_content("NO", "Success");
			}
			else{
				if(blackout != -1){
					res.set_content("Shutdown", "Success");
					cur = (cur % n) + 1;
					if(cur == blackout) svr.stop();
				}
				else res.set_content(G.Get_Map(), "Success");
			}
		}
	});

	svr.Post("/move", [&](const Request &req, Response &res){
		bool flag = req.has_file("Move");
		flag |= req.has_file("Wall");
		if(!flag){
			res.set_content("Invalid Request", "Error");
		}
		else if(req.has_file("Move")){
			const auto& file = req.get_file_value("Move");
			int tmp = (int)(file.content[0]) - 48;
			if((int)V.size() != n || tmp != cur || G.Move(cur - 1, file.content_type[0]) == false){
				res.set_content("Invalid Request", "Error");
			}
			else{
				if(G.Check(cur - 1)){
					res.set_content("Finish", "Seccess");
					cout << "Player number " << cur << " wins !" << endl;
					blackout = cur;
				}
				else{
					res.set_content("Success", "Success");
					cur = (cur % n) + 1;
				}
			}
		}
		else{
			const auto& file = req.get_file_value("Wall");
			int i = (int)(file.content[0]) - 49;
			int x = (int)(file.content[1]) - 65;
			int y = (int)(file.filename[0]) - 65;
			if((int)V.size() != n || i + 1 != cur || G.Add_Wall(i, x, y, file.content_type[0]) == false){
				res.set_content("Invalid Request", "Error");
			}
			else{
				res.set_content("Success", "Success");
				cur = (cur % n) + 1;
			}
		}
	});

	svr.listen("localhost", 8080);
}
