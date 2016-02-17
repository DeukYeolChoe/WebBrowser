#include "parser.h"

Parser::Parser()
{
}

string Parser::getIPaddrsFromURL(char* url)
{
	string ipAddrs(url);

	int foundS = 0;
	int foundE = 0;

	//http:// 지우기
	ipAddrs.erase(foundS, 7);

	foundE = ipAddrs.find(":");
	if (foundE >= 0)
	{
		ipAddrs = ipAddrs.substr(0, foundE);
	}

	foundE = ipAddrs.find("/");
	if (foundE >= 0)
	{
		ipAddrs = ipAddrs.substr(0, foundE);
	}

	return ipAddrs;
}

string Parser::getPortFromURL(char* url)
{
	string port(url);

	int foundS = 0;
	int foundE = 0;

	//http:// 지우기
	port.erase(foundS, 7);

	foundS = port.find(":");
	if (foundS >= 0)
	{
		foundE = port.find("/");
		port = port.substr(foundS + 1, foundE - (foundS + 1));
	}
	else
	{
		port = "80";
	}

	return port;
}

string Parser::getPathFromURL(char* url)
{
	string path(url);

	int foundS = 0;
	int foundE = 0;

	//http:// 지우기
	path.erase(foundS, 7);

	foundE = path.find("/");
	if (foundE >= 0)
	{
		path = path.erase(0, foundE + 1);
	}
	else
	{
		path = "";
	}	

	return path;
}

string Parser::getHTMLTag(string content)
{

	string tag = content;
	int foundS = 0;
	int foundE = 0;
	int offset = 0;

	if ((foundS = tag.find("<")) < 0) return "";
	if ((foundE = tag.find(">")) < 0) return "";
	tag = tag.substr(foundS + 1, foundE - (foundS + 1));

	if ((offset = tag.find(" ")) >= 0)
	{
		tag = tag.substr(0, offset);
	}

	return tag;
}

string Parser::removeHTMLTag(string content)
{
	char plainText[BUFSIZE];
	int n = 0;
	int i = 0;
	bool isSaved = false;
	char temp[2];

	//plainText = (char *)malloc(sizeof(char *) * strlen(content));
	memset(&plainText, '\0', sizeof(plainText));
	
	while (content.length() > n)
	{
		if (content[n] == '<')
			isSaved = false;
			
		if (content[n] == '>' && content.length() > (n+1))
		{
			isSaved = true;
			n++;
		}
		if (isSaved)
		{
			temp[0] = content[n];
			temp[1] = '\0';
			strcat(plainText, temp);
			i++;
		}
		n++;
	}
	
	return string(plainText);
}

ImgInfo Parser::getImgInfo(string content)
{
	ImgInfo img;
	content.erase(remove(content.begin(), content.end(), '\t'), content.end());
	string contentInfo = content;
	
	const regex re(R"(<img[^>]*src=['|\"](.*?)['|\"].*?>)", regex::icase);

	smatch results;
	if (regex_match(contentInfo, results, re))
	{
		img.src = results[1];
		cout << "img.src = " << img.src << endl;
	}

	const regex rew(R"(<img[^>]*width=['|\"](.*?)['|\"].*?>)", regex::icase);
	if (regex_match(contentInfo, results, rew))
	{
		img.width = results[1];
		cout << "img.width = " << img.width << endl;
	}

	const regex reh(R"(<img[^>]*height=['|\"](.*?)['|\"].*?>)", regex::icase);
	if (regex_match(contentInfo, results, reh))
	{
		img.height = results[1];
		cout << "img.height = " << img.height << endl;
	}

	return img;
}

HtmlInfo Parser::getHttpHeadBody(string content)
{
	int foundS = 0;
	int foundE = 0;
	string temp = content;

	if (content == "")
		return htmlInfo;
	
	if ((foundS = content.find("HTTP/1.")) >= 0)
	{
		foundE = content.find("\n");
		htmlInfo.status = content.substr(0, foundE);
	}

	std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
	if ((foundS = temp.find("<head>")) > 0)
	{
		foundE = temp.find("</head>");
		htmlInfo.head.append(content.substr(foundS + 6, foundE - (foundS + 6)));
	}

	if ((foundS = temp.find("<body>")) > 0)
	{
		foundE = temp.find("</body>");
		htmlInfo.body.append(content.substr(foundS + 6, foundE - (foundS + 6)));
	}

	return htmlInfo;
}

string Parser::getFormAction(string content)
{
	string action = "";
	content.erase(remove(content.begin(), content.end(), '\t'), content.end());
	string contentInfo = content;

	const regex re(R"(<form[^>]*action=['|\"](.*?)['|\"].*?>)", regex::icase);

	smatch results;
	if (regex_match(contentInfo, results, re))
	{
		action = results[1];
		cout << "Action = " << action << endl;
	}
	return action;
}

string Parser::getFormMethod(string content)
{
	string method = "";
	content.erase(remove(content.begin(), content.end(), '\t'), content.end());
	string contentInfo = content;

	const regex re(R"(<form[^>]*method=['|\"](.*?)['|\"].*?>)", regex::icase);

	smatch results;
	if (regex_match(contentInfo, results, re))
	{
		method = results[1];
		cout << "Method = " << method << endl;
	}
	return method;
}

string Parser::getInputType(string content)
{
	string type = "";
	content.erase(remove(content.begin(), content.end(), '\t'), content.end());
	string contentInfo = content;

	const regex re(R"(<input[^>]*type=['|\"](.*?)['|\"].*?>)", regex::icase);

	smatch results;
	if (regex_match(contentInfo, results, re))
	{
		type = results[1];
		cout << "Method = " << type << endl;
	}
	return type;
}

string Parser::getInputValue(string content)
{
	string value = "";
	content.erase(remove(content.begin(), content.end(), '\t'), content.end());
	string contentInfo = content;

	const regex re(R"(<input[^>]*value=['|\"](.*?)['|\"].*?>)", regex::icase);

	smatch results;
	if (regex_match(contentInfo, results, re))
	{
		value = results[1];
		cout << "Value = " << value << endl;
	}
	return value;
}

string Parser::getInputName(string content)
{
	string name = "";
	content.erase(remove(content.begin(), content.end(), '\t'), content.end());
	string contentInfo = content;

	const regex re(R"(<input[^>]*name=['|\"](.*?)['|\"].*?>)", regex::icase);

	smatch results;
	if (regex_match(contentInfo, results, re))
	{
		name = results[1];
		cout << "Name = " << name << endl;
	}
	return name;
}