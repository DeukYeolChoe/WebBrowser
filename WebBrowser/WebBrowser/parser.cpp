#include "parser.h"

Parser::Parser()
{
}

string Parser::getIPaddrsFromURL(string url)
{
	string ipAddrs = url;

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

string Parser::getPortFromURL(string url)
{
	string port = url;

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

string Parser::getPathFromURL(string url)
{
	string path = url;

	int foundS = 0;
	int foundE = 0;

	//http:// 지우기
	path.erase(foundS, 7);

	foundE = path.find("/");
	if (foundE >= 0)
	{
		path = path.erase(0, foundE);
	}
	else
	{
		path = "/";
	}	

	return path;
}

wstring Parser::getHTMLTag(wstring content)
{
	wstring tag = content;
	int foundS = 0;
	int foundE = 0;
	int offset = 0;

	if ((foundS = tag.find(L"<")) < 0) return L"";
	if ((foundE = tag.find(L">")) < 0) return L"";
	tag = tag.substr(foundS + 1, foundE - (foundS + 1));

	if ((offset = tag.find(L" ")) >= 0)
	{
		tag = tag.substr(0, offset);
	}

	return tag;
}

wstring Parser::removeHTMLTag(wstring content)
{
	wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	char plainText[BUFSIZE];
	int n = 0;
	int i = 0;
	bool isSaved = false;
	char temp[2];

	memset(&plainText, '\0', sizeof(plainText));
	
	while (content.length() > n)
	{
		if (content[n] == L'<')
			isSaved = false;
			
		if (content[n] == L'>' && content.length() > (n+1))
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
	
	return converter.from_bytes(plainText);
}

ImgAttr Parser::getImgInfo(wstring content)
{
	ImgAttr img;
	content.erase(remove(content.begin(), content.end(), '\t'), content.end());
	wstring contentInfo = content;
	
	const wregex re(L"(<img[^>]*src=['|\"](.*?)['|\"].*?>)", regex::icase);

	wsmatch results;
	if (regex_match(contentInfo.cbegin(), contentInfo.cend(), results, re))
	{
		img.src = results[2];
		wcout << "img.src = " << img.src << endl;
	}

	const wregex rew(L"(<img[^>]*width=['|\"](.*?)['|\"].*?>)", regex::icase);
	if (regex_match(contentInfo.cbegin(), contentInfo.cend(), results, rew))
	{
		img.width = results[2];
		wcout << "img.width = " << img.width << endl;
	}

	const wregex reh(L"(<img[^>]*height=['|\"](.*?)['|\"].*?>)", regex::icase);
	if (regex_match(contentInfo.cbegin(), contentInfo.cend(), results, reh))
	{
		img.height = results[2];
		wcout << "img.height = " << img.height << endl;
	}
	return img;
}

wstring Parser::getFormAction(wstring content)
{
	wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	wstring action = L"";
	content.erase(remove(content.begin(), content.end(), '\t'), content.end());
	wstring contentInfo = content;

	const wregex re(L"(<form[^>]*action=['|\"](.*?)['|\"].*?>)", regex::icase);

	wsmatch results;
	if (regex_match(contentInfo.cbegin(), contentInfo.cend(), results, re))
	{
		action = results[1];
		wcout << "Action = " << action << endl;
	}
	return action;
}

wstring Parser::getFormMethod(wstring content)
{
	wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	wstring method = L"";
	content.erase(remove(content.begin(), content.end(), '\t'), content.end());
	wstring contentInfo = content;

	const wregex re(L"(<form[^>]*method=['|\"](.*?)['|\"].*?>)", regex::icase);

	wsmatch results;
	if (regex_match(contentInfo.cbegin(), contentInfo.cend(), results, re))
	{
		method = results[1];
		wcout << "Method = " << method << endl;
	}
	return method;
}

wstring Parser::getInputType(wstring content)
{
	wstring type = L"";
	content.erase(remove(content.begin(), content.end(), '\t'), content.end());
	wstring contentInfo = content;

	const wregex re(L"(<input[^>]*type=['|\"](.*?)['|\"].*?>)", regex::icase);

	wsmatch results;
	if (regex_match(contentInfo.cbegin(), contentInfo.cend(), results, re))
	{
		type = results[1];
		wcout << "Method = " << type << endl;
	}
	return type;
}

wstring Parser::getInputValue(wstring content)
{
	wstring value = L"";
	content.erase(remove(content.begin(), content.end(), '\t'), content.end());
	wstring contentInfo = content;

	const wregex re(L"(<input[^>]*value=['|\"](.*?)['|\"].*?>)", regex::icase);

	wsmatch results;
	if (regex_match(contentInfo.cbegin(), contentInfo.cend(), results, re))
	{
		value = results[1];
		wcout << "Value = " << value << endl;
	}
	return value;
}

wstring Parser::getInputName(wstring content)
{
	wstring name = L"";
	content.erase(remove(content.begin(), content.end(), '\t'), content.end());
	wstring contentInfo = content;

	const wregex re(L"(<input[^>]*name=['|\"](.*?)['|\"].*?>)", regex::icase);

	wsmatch results;
	if (regex_match(contentInfo.cbegin(), contentInfo.cend(), results, re))
	{
		name = results[1];
		wcout << "Name = " << name << endl;
	}
	return name;
}

wstring Parser::getSpanstyle(wstring content)
{
	wstring style = L"";
	content.erase(remove(content.begin(), content.end(), '\t'), content.end());
	wstring contentInfo = content;

	const wregex re(L"(<span[^>]*style=['|\"](.*?)['|\"].*?>)", regex::icase);

	wsmatch results;
	if (regex_match(contentInfo.cbegin(), contentInfo.cend(), results, re))
	{
		style = results[2];
		wcout << "Name = " << style << endl;
	}
	return style;
}

Attribute Parser::getAttributes(wstring tag, wstring content)
{
	Attribute attrs;
	wstring page = content;
	wstring onetag = L"";
	int foundS = 0;
	int foundE = 0;

	//초기화
	attrs.identity.isPre = false; attrs.identity.isP = false; attrs.identity.isCenter = false; attrs.identity.isAddress = false;
	attrs.font.isBold = false;  attrs.font.isCursive = false; attrs.font.underscore = false; 
	attrs.font.size = 20;

	if (tag == L"h1") attrs.font.size = 40;
	else if (tag == L"h2") attrs.font.size = 35;
	else if (tag == L"h3") attrs.font.size = 30;
	else if (tag == L"h4") attrs.font.size = 25;
	else if (tag == L"h5") attrs.font.size = 20;
	else if (tag == L"h6") attrs.font.size = 15;

	if (tag == L"pre") attrs.identity.isPre = true;
	if (tag == L"p") attrs.identity.isP = true;
	if (tag == L"center") attrs.identity.isCenter = true;
	if (tag == L"address") attrs.identity.isAddress = true;
	if (tag == L"address") attrs.font.isCursive = true;
	if (tag == L"b") attrs.font.isBold = true;
 
	//태그 전체를 읽어 parsing한다.
	foundS = page.find(L"<" + tag);
	foundE = page.find(L">");
	onetag = page.substr(foundS, foundE - foundS + 1);

	//form 속성
	if (tag == L"form")
	{
		attrs.form.action = getFormAction(onetag);
		attrs.form.method = getFormMethod(onetag);
	}

	//input 속성
	if (tag == L"input")
	{
		attrs.input.value = getInputValue(onetag);
		attrs.input.type = getInputType(onetag);
		attrs.input.name = getInputName(onetag);
	}

	//img 속성
	if (tag == L"img")
		attrs.img = getImgInfo(onetag);

	//span 속성
	if (tag == L"span")
		attrs.font.style = getSpanstyle(onetag);

	return attrs;
}