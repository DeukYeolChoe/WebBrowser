#include "tree.h"
#include "parser.h"

Tree::Tree()
{
}

Node Tree::createTree(wstring htmlTags)
{
	stack<Node> htmlStack;
	Parser parser;

	int foundS = 0;
	int foundE = 0;
	int offset = 0;
	bool opening = false;
	bool closing = false;	
	wstring page = htmlTags;

	wstring tag = L"";
	wstring content = L"";

	//request header는 필요없으므로 지우자
	page = removeHttpHeader(page);

	while (page.length() > 0)
	{
		//초기화
		Node node; 

		//<!-- --> 안에 있는 내용 다 지우자
		page = removeComments(page);
		//<![CDATA[ //]]> 지우자
		page = removeCDATA(page);

		if ((foundS = page.find(L"<")) >= 0) opening = true;
		if (opening && (foundE = page.find(L">")) >= 0) closing = true;
		if (foundS != 0)
		{
			content = L"";
			content = page.substr(0, foundS);
			int n;
			if ((n = content.find(L">")) < 0)
			{
				if (wcscmp(tag.c_str(), L"pre") != 0)
				{
					content.erase(remove(content.begin(), content.end(), '\t'), content.end());
					content.erase(remove(content.begin(), content.end(), '\r'), content.end());
					content.erase(remove(content.begin(), content.end(), '\n'), content.end());
				}
				if (htmlStack.size() > 0 && wcscmp(content.c_str(), L"") != 0 && tag != L"title")
				{
					node.tag = L"text";
					node.content = content;
					htmlStack.top().children.push_back(node);	
				}		
			}							
			page.erase(0, foundS);
		}
		else if (opening && closing)
		{
			tag = parser.getHTMLTag(page);
			transform(tag.begin(), tag.end(), tag.begin(), ::tolower);

			//이 태그안에서 속성들을 다시 파싱해줘야한다.
			//상황에 맞게 node에 속성들을 넣어주자
			node.attributes = parser.getAttributes(tag, page);
			
			if (htmlStack.size() == 0)
			{
				node.tag = tag;
				htmlStack.push(node);
			}
			else if (tag == L"br" || tag == L"br/" || tag == L"hr" || tag == L"hr/" || 
				tag == L"img" || tag == L"meta" || tag == L"link" || tag == L"area" || tag == L"input" ||
				tag == L"area" || tag == L"base" || tag == L"embed" || tag == L"keygen" || tag == L"menuitem" || tag == L"meta" || 
				tag == L"param" || tag == L"source" || tag == L"track" || tag == L"wbr")
			{
				//Void Elements
				node.tag = tag;
				htmlStack.top().children.push_back(node);
			}
			else
			{
				wstring isPop = L"/" + htmlStack.top().tag;
				if (tag == isPop)
				{
					node = htmlStack.top();
					if (htmlStack.size() > 1)
					{
						htmlStack.pop();
						htmlStack.top().children.push_back(node);

						//부모의 특성의 상속 받는다.
						node.attributes.font = htmlStack.top().attributes.font;
					}
				}
				else
				{
					node.tag = tag;
					htmlStack.push(node);
				}
			}
			//int len = tag.length() + 2;
			page.erase(foundS, foundE - foundS + 1);
		}
		opening = false;
		closing = false;
	}
	return htmlStack.top();
}

Attribute Tree::inheritAttrsFromParent(Attribute parent, Attribute child)
{
	//폰트 속성
	if (parent.font.isBold)				child.font.isBold = true;
	if (parent.font.isCursive)			child.font.isCursive = true;
	if (parent.font.underscore)			child.font.underscore = true;
	if (!parent.font.weight.empty())	child.font.weight = parent.font.weight;
	if (!parent.font.style.empty())		child.font.style = parent.font.style;
	if (parent.isPre)					child.isPre = parent.isPre;
	if (parent.isCenter)				child.isCenter = parent.isCenter;
	if (parent.isP)						child.isP = parent.isP;
	//if (parent.font.size >= 0)			child.font.size = parent.font.size;
	return child;
}


wstring Tree::removeHttpHeader(wstring contents)
{
	wstring removedResponse = contents;
	int foundS = 0;
	int foundE = 0;

	if ((foundE = removedResponse.find(L"HTTP/1.1 200 OK\n\n")) >= 0)
	{
		removedResponse = removedResponse.erase(0, foundE + strlen("HTTP:1.1 200 ok\n\n"));
	}

	if ((foundE = removedResponse.find(L"\r\n\r\n")) >= 0)
	{
		removedResponse = removedResponse.erase(0, foundE + strlen("\r\n\r\n"));
	}

	if ((foundE = removedResponse.find(L"<!doctype html>")) >= 0)
	{
		removedResponse = removedResponse.erase(0, foundE + strlen("<!doctype html>"));
	}

	return removedResponse;
}

wstring Tree::removeComments(wstring contents)
{
	wstring removedResponse = contents;
	int foundS = 0;
	int foundE = 0;

	if ((foundS = removedResponse.find(L"<!--")) >= 0)
	{
		foundE = removedResponse.find(L"-->");
		removedResponse = removedResponse.erase(foundS, foundE - foundS + 3);
	}

	return removedResponse;
}

wstring Tree::removeCDATA(wstring contents)
{
	wstring removedResponse = contents;
	int foundS = 0;
	int foundE = 0;

	if ((foundS = removedResponse.find(L"//<![CDATA[")) >= 0)
	{
		foundE = removedResponse.find(L"//]]>");
		removedResponse = removedResponse.erase(foundS, foundE - foundS + 5);
	}

	return removedResponse;
}