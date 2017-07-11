#include "tinyxml/tinyxml.h"
#include "Md5.h"
#include "HttpModule.h"
#include "log4z.h"
#include <boost/algorithm/string.hpp>  
#include "VerifyApp.h"
#include "ConfigDefine.h"
#include "json/json.h"
#include "ISeverApp.h"
#include "AsyncRequestQuene.h"
#include "TaskPoolModule.h"
#include "VerifyApp.h"
#include "AnyLoginTask.h"
#include <fstream>
void CHttpModule::init(IServerApp* svrApp)
{
	IGlobalModule::init(svrApp);

	std::string strNotifyUrl = Wechat_notifyUrl;
	// parse port ;
	std::size_t nPosDot = strNotifyUrl.find_last_of(':');
	std::size_t nPosSlash = strNotifyUrl.find_last_of('/');
	uint16_t nPort = 80;
	if (nPosDot != std::string::npos && std::string::npos != nPosSlash)
	{
		auto strPort = strNotifyUrl.substr(nPosDot + 1 , nPosSlash - nPosDot - 1 );
		nPort = atoi(strPort.c_str());
		if (0 == nPort)
		{
			nPort = 80;
		}
	}
	//
	mHttpServer = boost::make_shared<http::server::server>(nPort);
	mHttpServer->run();

	// parse uri 
	std::size_t nPos = strNotifyUrl.find_last_of('/');
	std::string strUri = strNotifyUrl.substr(nPos,strNotifyUrl.size() - nPos );
	registerHttpHandle( strUri, boost::bind(&CHttpModule::onHandleVXPayResult, this, boost::placeholders::_1));

	registerHttpHandle("/playerInfo.yh", boost::bind(&CHttpModule::handleGetPlayerInfo, this, boost::placeholders::_1));
	registerHttpHandle("/addRoomCard.yh", boost::bind(&CHttpModule::handleAddRoomCard, this, boost::placeholders::_1));
	registerHttpHandle("/AnyLogin.yh", boost::bind(&CHttpModule::handleAnySdkLogin, this, boost::placeholders::_1));

	// halde config file 
	registerHttpHandle("/NJMJa.php", boost::bind(&CHttpModule::onHandleConfigN, this, boost::placeholders::_1));
	registerHttpHandle("/configNA.php", boost::bind(&CHttpModule::onHandleConfigNA, this, boost::placeholders::_1));
	registerHttpHandle("/configGolden.php", boost::bind(&CHttpModule::onHandleConfigGolden, this, boost::placeholders::_1));
	registerHttpHandle("/configGoldenA.php", boost::bind(&CHttpModule::onHandleConfigGoldenA, this, boost::placeholders::_1));
	registerHttpHandle("/configRefresh.php", boost::bind(&CHttpModule::onHandleConfigRefresh, this, boost::placeholders::_1));
}

void CHttpModule::update(float fDeta)
{
	IGlobalModule::update(fDeta);
	std::set<http::server::connection_ptr> vOut;
	if (!mHttpServer->getRequestConnects(vOut))
	{
		return;
	}

	for (auto& ref : vOut)
	{
		auto req = ref->getReqPtr();
		auto pReply = ref->getReplyPtr();
		auto iter = vHttphandles.find( req->uri );
		if (iter == vHttphandles.end())
		{
			LOGFMTE("no handle for uri = %s",req->uri.c_str());
			*pReply = http::server::reply::stock_reply(http::server::reply::bad_request);
			ref->doReply();
			continue;
		}
		
		auto pfunc = iter->second;
		if ( !pfunc(ref) )
		{
			*pReply = http::server::reply::stock_reply(http::server::reply::bad_request);
			ref->doReply();
		}
	}
	vOut.clear();
}

bool CHttpModule::registerHttpHandle(std::string strURI, httpHandle pHandle)
{
	auto iter = vHttphandles.find(strURI);
	if (iter != vHttphandles.end())
	{
		LOGFMTE("already register handle for uri = %s",strURI.c_str());
		return false;
	}
	vHttphandles[strURI] = pHandle;
	return true;
}


 std::string getXmlNodeValue(const char* pnodeName, TiXmlNode* pRoot)
{
	std::string str = "";
	TiXmlElement* pValueParent = (TiXmlElement*)pRoot->FirstChild(pnodeName);
	if (pValueParent)
	{
		TiXmlNode* pValue = pValueParent->FirstChild();
		if (pValue)
		{
			str = pValue->Value();
			return str;
		}
	}
	LOGFMTE("xml node = %s value is null", pnodeName);
	return str;
}

// process vx pay result 
bool CHttpModule::onHandleVXPayResult(http::server::connection_ptr ptr)
{
	auto req = ptr->getReqPtr();
	auto res = ptr->getReplyPtr();

	if (req->contentSize == 0)
	{
		return false;
	}

	// parse xml 
	TiXmlDocument t;
	t.Parse(req->reqContent.c_str(), 0, TIXML_ENCODING_UTF8);
	if (t.Error())
	{
		LOGFMTE("vx pay parse xml error : %s",t.ErrorDesc());
		return false;
	}
#ifndef Node_Value_F  
#define node_Value( x ) getXmlNodeValue((x),pRoot)
#define Node_Value_F ;
#endif 
	TiXmlNode* pRoot = t.RootElement();
	uint8_t nRet = 0;
	do
	{
		if (!pRoot)
		{
			nRet = 1;
			break;
		}

		auto retCode = node_Value("return_code");
		if (retCode != "SUCCESS")
		{
			nRet = 3;
			auto strEmsg = node_Value("return_msg");
			LOGFMTE("ret msg : %s", strEmsg.c_str());
			break;
		}
 
		// do parse 
		auto fee = node_Value("total_fee");
		auto strTradeNo = node_Value("out_trade_no");
		auto payResult = node_Value("result_code");
		auto payTime = node_Value("time_end");
		if ("FAIL" == payResult)
		{
			return true ;
		}
		
		std::vector<std::string> vOut;
		boost::split(vOut,strTradeNo,boost::is_any_of("E"));
		if (vOut.size() < 2)
		{
			LOGFMTE("trade out error = %s",strTradeNo.c_str() );
			nRet = 4;
			break;
		}
		auto shopItem = vOut[0];
		auto userUID = vOut[1];
		LOGFMTD( "GO TO DB Verify trade = %s , fee = %s payTime = %s",strTradeNo.c_str(),fee.c_str(),payTime.c_str() );

		// do DB verify ;
		auto pVeirfyModule = ((CVerifyApp*)getSvrApp())->getTaskPoolModule();
		pVeirfyModule->doDBVerify(atoi(userUID.c_str()), atoi(shopItem.c_str()), ePay_WeChat,strTradeNo,atoi(fee.c_str()));
	} while (0);

	std::string str = "";
	if (nRet != 0)
	{
		str = "<xml><return_code><![CDATA[FAIL]]></return_code> <return_msg><![CDATA[unknown]]></return_msg> </xml> ";
	}
	else
	{
		str = "<xml><return_code><![CDATA[SUCCESS]]></return_code> <return_msg><![CDATA[OK]]></return_msg> </xml> ";
	}

	res->setContent(str,"text/xml");
	ptr->doReply();
	t.SaveFile("reT.xml");
	return true;
}

bool CHttpModule::handleGetPlayerInfo(http::server::connection_ptr ptr)
{
	auto req = ptr->getReqPtr();
	auto res = ptr->getReplyPtr();
	LOGFMTD("reciveget player info req = %s",req->reqContent.c_str());

	Json::Reader jsReader;
	Json::Value jsRoot;
	auto bRet = jsReader.parse(req->reqContent, jsRoot);
	if (!bRet)
	{
		LOGFMTE("parse agent get player info argument error");
		return false;
	}

	uint32_t nUID = 0;
	if (jsRoot["playerUID"].isNull() || jsRoot["playerUID"].isUInt() == false )
	{
		LOGFMTD("cant not finn uid argument");
		return false;
	}
	nUID = jsRoot["playerUID"].asUInt();

	// do async request 
	Json::Value jsReq;
	jsReq["targetUID"] = nUID;
	auto async = getSvrApp()->getAsynReqQueue();
	async->pushAsyncRequest(ID_MSG_PORT_DATA, eAsync_AgentGetPlayerInfo, jsReq, [this, ptr, nUID, async](uint16_t nReqType, const Json::Value& retContent, Json::Value& jsUserData)
	{
		// define help function , fetch not process mail card , and do respone
		auto lpfCheckDBMail = [](CAsyncRequestQuene* async, http::server::connection_ptr ptr, uint32_t nUID, Json::Value& jsAgentBack)
		{
			// take not process add card mail in to account 
			Json::Value jsSql;
			char pBuffer[512] = { 0 };
			sprintf(pBuffer, "SELECT * FROM mail WHERE userUID = '%u' and mailType = %u and state = '0' order by postTime desc limit 5 ;", nUID, eMailType::eMail_AddRoomCard);
			jsSql["sql"] = pBuffer;
			async->pushAsyncRequest(ID_MSG_PORT_DB, eAsync_DB_Select, jsSql, [ptr, nUID](uint16_t nReqType, const Json::Value& retContent, Json::Value& jsUserData)
			{
				uint32_t nTotalCnt = 0;
				uint8_t nRow = retContent["afctRow"].asUInt();
				Json::Value jsData = retContent["data"];
				for (uint8_t nIdx = 0; nIdx < jsData.size(); ++nIdx)
				{
					Json::Value jsRow = jsData[nIdx];

					Json::Reader jsReader;
					Json::Value jsC;
					auto bRt = jsReader.parse(jsRow["mailContent"].asString(), jsC);
					if ( !bRt || jsC["addCard"].isNull())
					{
						LOGFMTE("pasre add card mail error id = %u", nUID);
						continue;
					}
					nTotalCnt += jsC["addCard"].asInt();
				}
				LOGFMTD("uid = %u mail card cnt = %u",nUID,nTotalCnt);
				jsUserData["cardCnt"] = jsUserData["cardCnt"].asUInt() + nTotalCnt;

				// build msg to send ;
				auto res = ptr->getReplyPtr();
				Json::StyledWriter jswrite;
				auto str = jswrite.write(jsUserData);
				res->setContent(str, "text/json");
				ptr->doReply();
				LOGFMTD("do get player info cards uid = %u", nUID);
			}, jsAgentBack);
		};

		bool isOnline = retContent["isOnline"].asUInt() == 1;
		if (isOnline)
		{
			Json::Value jsAgentBack;
			jsAgentBack["ret"] = 1;
			jsAgentBack["name"] = retContent["name"];
			jsAgentBack["playerUID"] = nUID;
			jsAgentBack["cardCnt"] = retContent["leftCardCnt"];

			lpfCheckDBMail(async,ptr,nUID,jsAgentBack);

			return;
		}
		
		LOGFMTD("uid = %u not online get info from db ",nUID);
		// not online , must get name first ;
		Json::Value jsSql;
		char pBuffer[512] = { 0 };
		sprintf(pBuffer, "SELECT playerName, diamond FROM playerbasedata WHERE userUID = '%u' ;", nUID );
		jsSql["sql"] = pBuffer;
		async->pushAsyncRequest(ID_MSG_PORT_DB, eAsync_DB_Select, jsSql, [lpfCheckDBMail,async, ptr, nUID](uint16_t nReqType, const Json::Value& retContent, Json::Value& jsUserData)
		{
			uint8_t nRow = retContent["afctRow"].asUInt();

			Json::Value jsAgentBack;
			jsAgentBack["ret"] = 1;
			jsAgentBack["name"] = "";
			jsAgentBack["playerUID"] = nUID;
			jsAgentBack["cardCnt"] = 0;

			if (nRow == 0)
			{
				jsAgentBack["ret"] = 0;
				// build msg to send ;
				auto res = ptr->getReplyPtr();
				Json::StyledWriter jswrite;
				auto str = jswrite.write(jsAgentBack);
				res->setContent(str, "text/json");
				ptr->doReply();
				LOGFMTE("get can find uid = %u info from db",nUID);
				return;
			}
			else
			{
				Json::Value jsData = retContent["data"];
				Json::Value jsRow = jsData[0u];
				jsAgentBack["name"] = jsRow["playerName"];
				jsAgentBack["cardCnt"] = jsRow["diamond"];
				LOGFMTD("uid = %u base data card cnt = %u", nUID, jsRow["diamond"].asUInt());
				lpfCheckDBMail(async, ptr, nUID, jsAgentBack);

				return;
			}
		});
	});
	LOGFMTD("do async agent get player info uid = %u", nUID);
	return true;
}

bool CHttpModule::handleAddRoomCard(http::server::connection_ptr ptr)
{
	auto req = ptr->getReqPtr();
	auto res = ptr->getReplyPtr();
	LOGFMTD("reciveget add room card info req = %s", req->reqContent.c_str());

	Json::Reader jsReader;
	Json::Value jsRoot;
	auto bRet = jsReader.parse(req->reqContent, jsRoot);
	if (!bRet)
	{
		LOGFMTE("parse add room card argument error");
		return false;
	}

	uint32_t nUID = 0;
	int32_t nAddCard;
	uint32_t nAddCardNo;
	if (jsRoot["playerUID"].isNull() || jsRoot["playerUID"].isUInt() == false )
	{
		LOGFMTD("cant not finn uid argument");
		return false;
	}

	if (jsRoot["addCard"].isNull() || jsRoot["addCard"].isInt() == false)
	{
		LOGFMTD("cant not finn addCard argument");
		return false;
	}

	if (jsRoot["addCardNo"].isNull() || jsRoot["addCardNo"].isUInt() == false)
	{
		LOGFMTD("cant not finn addCardNo argument");
		return false;
	}

	nAddCard = jsRoot["addCard"].asInt();
	nUID = jsRoot["playerUID"].asUInt();
	nAddCardNo = jsRoot["addCardNo"].asUInt();

	// do async request 
	Json::Value jsReq;
	jsReq["targetUID"] = nUID;
	jsReq["addCard"] = nAddCard;
	jsReq["addCardNo"] = nAddCardNo;
	auto async =  getSvrApp()->getAsynReqQueue();
	async->pushAsyncRequest(ID_MSG_PORT_DATA, eAsync_AgentAddRoomCard, jsReq, [this, ptr, nUID, nAddCardNo](uint16_t nReqType, const Json::Value& retContent, Json::Value& jsUserData)
	{
		auto res = ptr->getReplyPtr();
		// do check 
		jsUserData["ret"] = 1;
		Json::StyledWriter jswrite;
		auto str = jswrite.write(jsUserData);
		res->setContent(str, "text/json");
		ptr->doReply();
		LOGFMTD("do agent add room cards uid = %u, addCardNo = %u ", nUID, nAddCardNo);
	}, jsRoot);
	LOGFMTD("do async agent add room cards uid = %u cnt = %u,addCardNo = %u", nUID, nAddCard, nAddCardNo);
	return true;
}

bool CHttpModule::handleAnySdkLogin(http::server::connection_ptr ptr)
{
	auto req = ptr->getReqPtr();
	auto res = ptr->getReplyPtr();
	LOGFMTD("reciveget any sdk req = %s", req->reqContent.c_str());
	auto pTaskPool = ((CVerifyApp*)getSvrApp())->getTaskPoolModule();
	auto pTask = pTaskPool->getReuseTask(CTaskPoolModule::eTask_AnyLogin);
	auto pLoginTask = (AnyLoginTask*)pTask.get();
	pLoginTask->setReqString(req->reqContent);
	pTask->setCallBack([ptr](ITask::ITaskPrt pTask)
	{
		auto pLoginTask = (AnyLoginTask*)pTask.get();
		auto res = ptr->getReplyPtr();
		if (pTask->getResultCode() == 1)
		{
			LOGFMTE("request error from any sdk");
			// do check 
			std::string str = "error";
			res->setContent(str, "text/json");
			ptr->doReply();
			return;
		}

		auto jsResult = pLoginTask->getResultJson();
		Json::StyledWriter jswrite;
		auto str = jswrite.write(jsResult);
		res->setContent(str, "text/json");
		ptr->doReply();
		LOGFMTE("any sdk resp: %s",str.c_str());
	});
	pTaskPool->postTask(pTask);
	return true;
}

// handle config file 
bool CHttpModule::onHandleConfigN(http::server::connection_ptr ptr)
{
	return responeConfigToConnect("../configFile/NJMJa.php",ptr);
}

bool CHttpModule::onHandleConfigNA(http::server::connection_ptr ptr)
{
	return responeConfigToConnect("../configFile/configNA.php", ptr);
}

bool CHttpModule::onHandleConfigGolden(http::server::connection_ptr ptr)
{
	return responeConfigToConnect("../configFile/Golden.php", ptr);
}

bool CHttpModule::onHandleConfigGoldenA(http::server::connection_ptr ptr)
{
	return responeConfigToConnect("../configFile/GoldenA.php", ptr);
}

bool CHttpModule::onHandleConfigRefresh(http::server::connection_ptr ptr)
{
	auto req = ptr->getReqPtr();
	auto res = ptr->getReplyPtr();
	vConfigFile.clear();
	std::string str = "success";
	res->setContent(str);
	ptr->doReply();
	return true;
}

bool CHttpModule::readFileToString(string file_name, string& fileData)
{
	ifstream file(file_name.c_str(), std::ifstream::binary);

	if (file)
	{
		// Calculate the file's size, and allocate a buffer of that size.
		file.seekg(0, file.end);
		const int file_size = file.tellg();
		char* file_buf = new char[file_size + 1];
		//make sure the end tag \0 of string.

		memset(file_buf, 0, file_size + 1);

		// Read the entire file into the buffer.
		file.seekg(0, ios::beg);
		file.read(file_buf, file_size);


		if (file)
		{
			fileData.append(file_buf);
		}
		else
		{
			std::cout << "error: only " << file.gcount() << " could be read";
			fileData.append(file_buf);
			return false;
		}
		file.close();
		delete[]file_buf;
	}
	else
	{
		return false;
	}


	return true;
}

bool CHttpModule::responeConfigToConnect(const char* pFileName, http::server::connection_ptr ptr)
{
	auto req = ptr->getReqPtr();
	auto res = ptr->getReplyPtr();

	auto iter = vConfigFile.find(pFileName);
	std::string strFile;
	if (iter == vConfigFile.end())
	{
		auto b = readFileToString(pFileName, strFile);
		if (b == false)
		{
			LOGFMTE("why can not find the file %s", pFileName);
			return false;
		}

		LOGFMTD("read file = %s : content : %s", pFileName, strFile.c_str());
		vConfigFile[pFileName] = strFile;
	}
	else
	{
		strFile = iter->second;
	}

	res->setContent(strFile);
	ptr->doReply();
	return true;
}
