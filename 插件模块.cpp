#include "预编译头.h"
#include "BDS内容.hpp"

#include <string>
#include <regex>
#include <fstream>
#include <map>
using std::string;
#ifndef _LIB
#define _LIB
#pragma comment(lib,"./BDXMONEY.lib")

#endif // !_LIB
#include"bdxmoney.h"
template<int argumnum> auto checkargu(std::string patter, std::smatch& checker, std::string& cmd) {
	bool test = std::regex_match(cmd, checker, std::regex(patter));
	return test;
}
std::map <std::string, double> scoreslist;
auto checkandgetinlist(std::string& objname) {
	for (auto i = scoreslist.begin(); i != scoreslist.end(); ++i) {
		if (i->first == objname) return i->second;
	}
	return (double)(0);
}
namespace WTMETHOD {
	Scoreboard* my_scoreboard;
	VA p_spscqueue;
	std::string m_doubleQM = "\"";
	// 执行后端指令
	bool runcmd(std::string cmd) {
		if (p_spscqueue != 0)
			return SYMCALL(bool, MSSYM_MD5_b5c9e566146b3136e6fb37f0c080d91e, p_spscqueue, cmd);
		return false;
	}
	//thook2
	THook2(_MCMD_GETSPSCQUEUE, VA, MSSYM_MD5_3b8fb7204bf8294ee636ba7272eec000,
		VA _this) {
		p_spscqueue = original(_this);
		return p_spscqueue;
	}
	void transfertobdxmoney(Player* p, std::string& cmd) {
		std::smatch checker;
		if (checkargu<0>("([^\\s]+)\\s(\\d{0,9})", checker, cmd)) {
			auto objname = checker.str(1); auto num = atoll(checker.str(2).c_str());
			auto pname = *p->getNameTag();
			auto proportion = checkandgetinlist(objname);
			if(proportion==0) {runcmd("w \"" + pname + u8"\" 请检查在配置文件中是否设置了该项且对应汇率是否输入正确（不能小于等于0)");return;}
			auto moneyshouldtransfer = (int)(num * proportion);
			if(moneyshouldtransfer<=1) { runcmd("w \"" + pname + u8"\" 能够转换的数额太少！(转换后数额至少为2才能进行转换)"); return; }
			
			auto testobj = my_scoreboard->getobject(&objname);
			if (!testobj) {
				runcmd("w \"" + pname + u8"\" 没有找到对应计分板");
				return;
			}
			__int64 a2[2];
			auto scoreid = my_scoreboard->getScoreboardID(p);
			auto scores = testobj->getplayerscoreinfo((ScoreInfo*)a2, scoreid);
			if ((__int64)scores->getcount() < num) {
				runcmd("w \"" + pname + u8"\" 您好像没有足够的余额(拥有：" + std::to_string(scores->getcount()) + u8",需要：" + std::to_string(num) + ")");
				return;
			}
			if (num > 2147483648) {
				runcmd("w \"" + pname + u8"\" 金钱数额过大，无法进行转换");
				return;
			}
			auto scoreref = my_scoreboard->getScoreboardIdentityRef((ScoreboardId*)scoreid);
			if (!scoreref) {
				runcmd("scoreboard players add \"" + pname + "\" " + objname + " 0");
				runcmd("w \"" + pname + "\" 404 not found please try again");
				return;
			}
			auto mxuid = atoll(p->getXuid().c_str());
			Money::createTrans(0, mxuid, moneyshouldtransfer, u8"transferfromscores转换");
			bool a1;
			my_scoreboard->modifiedscores(&a1, (ScoreboardId*)scoreid, testobj, (int)num, 2);
			runcmd("w \"" + pname + u8"\" 成功转换为了money*"+std::to_string(moneyshouldtransfer));
		}
		else {
			runcmd("w \"" + *p->getNameTag() + "\" 格式错误，/trtomoney [计分板名称] [转换数量(小于10位)]");
		}
	}
	void transferfrombdxmoney(Player* p, std::string& cmd) {
		std::smatch checker;
		if (checkargu<0>("([^\\s]+)\\s(\\d{0,9})", checker, cmd)) {
			auto objname = checker.str(1); auto num = atoll(checker.str(2).c_str());
			auto pname = *p->getNameTag();
			auto proportion = checkandgetinlist(objname);
			if (proportion == 0) { runcmd("w \"" + pname + u8"\" 请检查在配置文件中是否设置了该项且对应汇率是否输入正确（不能小于等于0)"); return; }
			auto moneyshouldtransfer = (int)(num / proportion);
			if (moneyshouldtransfer <= 1) { runcmd("w \"" + pname + u8"\" 能够转换的数额太少！(转换后数额至少为2才能进行转换)"); return; }


			auto testobj = my_scoreboard->getobject(&objname);
			if (!testobj) {
				runcmd("w \"" + pname + u8"\" 没有找到对应计分板");
				return;
			}
			auto mxuid = atoll(p->getXuid().c_str());
			auto mybdxmoney = Money::getMoney(mxuid);
			if (mybdxmoney < num) {
				runcmd("w \"" + pname + u8"\" 您好像没有足够的余额(拥有："+std::to_string(mybdxmoney)+u8",需要："+std::to_string(num)+")");
				return;
			}
			auto scoreid = my_scoreboard->getScoreboardID(p);
			auto scoreref = my_scoreboard->getScoreboardIdentityRef((ScoreboardId*)scoreid);
			if (!scoreref) {
				runcmd("scoreboard players add \"" + pname + "\" " + objname + " 0");
				runcmd("w \"" + pname + "\" 404 not found please try again");
				return;
			}
			bool a3 = 0;
			my_scoreboard->modifiedscores(&a3, (ScoreboardId*)scoreid, testobj, moneyshouldtransfer, 1);
			Money::reduceMoney(mxuid, num);//
			runcmd("w \"" + pname + u8"\" 成功转换为了指定计分板项目*"+std::to_string(moneyshouldtransfer));
		}
		else {
			runcmd("w \"" + *p->getNameTag() + u8"\" 格式错误，/trfrommoney [计分板名称] [转换数量(小于10位)]");
		}
	}
	void autotransfrombdxmoney(Player* p, std::string& objname) {
		auto pname = *p->getNameTag();
		auto proportion = checkandgetinlist(objname);
		if (proportion == 0) { runcmd("w \"" + pname + u8"\" 请检查在配置文件中是否设置了该项且对应汇率是否输入正确（不能小于等于0)"); return; }
		auto testobj = my_scoreboard->getobject(&objname);
		if (!testobj) {
			runcmd("w \"" + pname + u8"\" 没有找到对应计分板");
			return;
		}
		auto mxuid = atoll(p->getXuid().c_str());
		auto mybdxmoney = Money::getMoney(mxuid);
		if (mybdxmoney > 2147483648) {
			runcmd("w \"" + pname + u8"\" 您拥有的金钱数额过大，无法进行转换");
			return;
		}
		auto maxcharge = static_cast<money_t>((double)mybdxmoney / proportion);
		auto moneyshouldtransfer = static_cast<money_t>((double)maxcharge * proportion);
		auto shouldpay = static_cast<int>(moneyshouldtransfer);
		Money::reduceMoney(mxuid, shouldpay);
		auto scoreid = my_scoreboard->getScoreboardID(p);
		auto scoreref = my_scoreboard->getScoreboardIdentityRef((ScoreboardId*)scoreid);
		if (!scoreref) {
			runcmd("scoreboard players add \"" + pname + "\" " + objname + " 0");
			runcmd("w \"" + pname + "\" 404 not found please try again");
			return;
		}
		bool a3 = 0;
		my_scoreboard->modifiedscores(&a3, (ScoreboardId*)scoreid, testobj, (int)((double)shouldpay / proportion), 1);
		runcmd("w \"" + pname + u8"\" 自动转换了"+std::to_string(shouldpay)+u8"*money，已为指定计分板添加*" + std::to_string((int)((double)shouldpay / proportion)) + u8"\n汇率:" + std::to_string(proportion));
	}
	void autotranstobdxmoney(Player* p, std::string& objname) {
		auto pname = *p->getNameTag();
		auto proportion = checkandgetinlist(objname);
		if (proportion == 0) { runcmd("w \"" + pname + u8"\" 请检查在配置文件中是否设置了该项且对应汇率是否输入正确（不能小于等于0)"); return; }
		auto testobj = my_scoreboard->getobject(&objname);
		if (!testobj) {
			runcmd("w \"" + pname + u8"\" 没有找到对应计分板");
			return;
		}
		auto mxuid = atoll(p->getXuid().c_str());
		__int64 a2[2];
		auto scoreid = my_scoreboard->getScoreboardID(p);
		testobj->getplayerscoreinfo((ScoreInfo*)a2, scoreid);
		auto scoreref = my_scoreboard->getScoreboardIdentityRef((ScoreboardId*)scoreid);
		if (!scoreref) {
			runcmd("scoreboard players add \"" + pname + "\" " + objname + " 0");
			runcmd("w \"" + pname + "\" 404 not found please try again");
			return;
		}
		auto scores = testobj->getplayerscoreinfo((ScoreInfo*)a2, scoreid)->getcount();
		auto maxcharge = static_cast<int>((double)scores *proportion);
		auto moneyshouldtransfer = static_cast<int>(maxcharge / proportion);
		bool a3 = 0;
		my_scoreboard->modifiedscores(&a3, (ScoreboardId*)scoreid,testobj, moneyshouldtransfer, 2);
		Money::createTrans(0, mxuid, (__int64)(moneyshouldtransfer*proportion), u8"transferfromscores转换");
		runcmd("w \"" + pname + u8"\" 自动转换了" + std::to_string(moneyshouldtransfer) + u8"*指定计分板，已添加money*" + std::to_string((__int64)(moneyshouldtransfer * proportion))+u8"\n汇率:"+std::to_string(proportion));
	}
	void showmoneymessage(Player* p) {
		std::string str;
		for (auto i = scoreslist.begin(); i != scoreslist.end(); ++i) {
			str += u8"可兑换计分板名:" + i->first + u8",兑换汇率(计分板对money):" + std::to_string(i->second)+"\n";
		}
		runcmd("w \"" + *p->getNameTag() + "\" " + str);
	}
}

namespace Reader {
	void generater() {
		std::fstream Loader;
		std::smatch checker;
		char buff[255]; std::string str;
		Loader.open("transmoney.ini");
		if (Loader.fail()) {
			std::cout << "error:when open transmoney.ini, not find the file!" << std::endl; return;
		}
		while (!Loader.eof()) {
			Loader.getline(buff, 255);
			str = buff;
			if (checkargu<0>("([^\\:]+)\\:([0-9]+.?[0-9]*)", checker, str)) {
				std::cout << checker.str(1) << ",another:" << checker.str(2) << std::endl;
				scoreslist.emplace(checker.str(1), atof(checker.str(2).c_str()));
			}
			else {
				if (Loader.eof()) break;
				std::cout << "error:when load " << str <<"check your input!"<< std::endl;
			}
		}
		Loader.close();
	}
}
//scoreboard
THook(void*, MSSYM_B2QQE170ServerScoreboardB2AAA4QEAAB1AE24VCommandSoftEnumRegistryB2AAE16PEAVLevelStorageB3AAAA1Z, void* _this, void* a2, void* a3) {
	WTMETHOD::my_scoreboard = (Scoreboard*)original(_this, a2, a3);
	return WTMETHOD::my_scoreboard;
}

// 输入指令
THook(void,
	MSSYM_B1QA6handleB1AE20ServerNetworkHandlerB2AAE26UEAAXAEBVNetworkIdentifierB2AAE24AEBVCommandRequestPacketB3AAAA1Z,
	void* _this, void* id, void* crp) {
	Player* p = SYMCALL(Player*, MSSYM_B2QUE15getServerPlayerB1AE20ServerNetworkHandlerB2AAE20AEAAPEAVServerPlayerB2AAE21AEBVNetworkIdentifierB2AAA1EB1AA1Z,
		_this, id, *((char*)crp + 16));
	auto cmd = (std::string(*(std::string*)((VA)crp + 40))).substr(1);
	if (cmd.find("trtomoney ", 0, 10) == 0) { cmd = cmd.substr(10, cmd.length() - 10); WTMETHOD::transfertobdxmoney(p, cmd); return; }
	if (cmd.find("trfrommoney ", 0, 12) == 0) { cmd = cmd.substr(12, cmd.length() - 12); WTMETHOD::transferfrombdxmoney(p, cmd); return; }
	if (cmd.find("autotrfrommoney ", 0, 16) == 0) { cmd = cmd.substr(16, cmd.length() - 16); WTMETHOD::autotransfrombdxmoney(p, cmd); return; }
	if (cmd.find("autotrtomoney ", 0, 14) == 0) { cmd = cmd.substr(14, cmd.length() - 14); WTMETHOD::autotranstobdxmoney(p, cmd); return; }
	if (cmd.find("trquery ", 0, 7) == 0) { WTMETHOD::showmoneymessage(p); return; }
	original(_this, id, crp);
}

THook(void, MSSYM_B1QA5setupB1AE20ChangeSettingCommandB2AAE22SAXAEAVCommandRegistryB3AAAA1Z, CommandRegistry* _this) {
	_this->registerCommand("trtomoney", u8"将指定计分板转换到bdxmoney,/trtomoney [计分板名称] [转换数量]", 0, { 0 }, { 0x40 });
	_this->registerCommand("trfrommoney", u8"将bdxmoney转换到指定计分板,/trfrommoney [计分板名称] [转换数量]", 0, { 0 }, { 0x40 });
	_this->registerCommand("autotrtomoney", u8"将指定计分板最大地转换到bdxmoney,/autotrtomoney [计分板名称]", 0, { 0 }, { 0x40 });
	_this->registerCommand("autotrfrommoney", u8"将bdxmoney最大地转换到指定计分板,/autotrfrommoney [计分板名称]", 0, { 0 }, { 0x40 });
	_this->registerCommand("trquery", u8"查询可兑换的计分板项目及汇率", 0, { 0 }, { 0x40 });
	original(_this);
}


void init() {
	std::cout << u8"bdx经济转换计分板1.14.60.5 作者：starkc" << std::endl;
	Reader::generater();
	// 此处填写插件加载时的操作
}

void exit() {
	// 此处填写插件卸载时的操作
}

