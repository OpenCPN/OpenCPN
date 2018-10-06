/*
 *	davtool.cpp
 *	wxCURL
 *
 *	Created by Casey O'Donnell on Thu Jul 15 2004.
 *	Copyright (c) 2004 Casey O'Donnell. All rights reserved.
 *
 *
 *	Licence: wxWidgets Licence
 */

// davtool.cpp
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Headers
//////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#ifdef __WXMSW__
    #include <wx/msw/msvcrt.h>      // useful to catch memory leaks when compiling under MSVC 
#endif


#include <wx/mstream.h>
#include <wx/sstream.h>
#include <wx/xml/xml.h>

#include <wx/curl/davtool.h>

//////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////

// davtool.cpp: implementation of the wxCurlDAVFs class.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

wxCurlDAVFs::wxCurlDAVFs()
: m_iContentLength(0)
{
}

wxCurlDAVFs::wxCurlDAVFs(const wxString& szHREF			,
						 const wxString& szStatus		,
						 const wxString& szCreationDate	,
						 const wxString& szLastModified	,
						 const long& iContentLength		,
						 const wxString& szContentType	)
: m_szHREF(szHREF), m_szStatus(szStatus),
m_iContentLength(iContentLength), m_szContentType(szContentType)
{
	m_dtCreationDate = wxCurlBase::GetDateFromString(szCreationDate);
	m_dtLastModified = wxCurlBase::GetDateFromString(szLastModified);
}

wxCurlDAVFs::~wxCurlDAVFs()
{
}

// davtool.cpp: implementation of the wxCurlDAVTool class.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

wxCurlDAVTool::wxCurlDAVTool(const wxString& szURL /*= wxEmptyString*/, 
                             const wxString& szUserName /*= wxEmptyString*/, 
                             const wxString& szPassword /*= wxEmptyString*/, 
                             wxEvtHandler* pEvtHandler /*= NULL*/, 
                             long flags/*= wxCURL_DEFAULT_FLAGS*/)
: wxCurlDAV(szURL, szUserName, szPassword, pEvtHandler, flags)
{
}

wxCurlDAVTool::~wxCurlDAVTool()
{
}

//////////////////////////////////////////////////////////////////////
// Member Data Access Methods
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Action Methods
//////////////////////////////////////////////////////////////////////

bool wxCurlDAVTool::GetDAVFs(wxArrayDAVFs& fs, const wxString& szRemoteLoc /*= wxEmptyString*/)
{
	wxArrayString arrProps;

	arrProps.Add(wxS("creationdate"));
	arrProps.Add(wxS("getlastmodified"));
	arrProps.Add(wxS("getcontentlength"));
	arrProps.Add(wxS("getcontenttype"));

	if(Propfind(arrProps, szRemoteLoc))
	{
		// Construct Input Source...
		wxStringInputStream inStream(wxCURL_BUF2STRING(m_szResponseBody));

		if(inStream.IsOk())
		{
			// Construct XML Parser
			wxXmlDocument xmlPropfind(inStream);

			if(xmlPropfind.IsOk())
			{
				// Process XML!
				wxXmlNode* pNode = xmlPropfind.GetRoot();

				// Strip Past First "Multistatus" tag...
				while(pNode)
				{
					if(pNode->GetName().Find(wxS("multistatus")) != -1)
					{
						pNode = pNode->GetChildren();
						break;
					}
					else
						pNode = pNode->GetNext();
				}
				
				// Process "Response" tags...
				while(pNode)
				{
					wxCurlDAVFs fsItem;

					wxString szName = pNode->GetName();

					if(pNode->GetName().Find(wxS("response")) != -1)
					{
						if(ParseResponseXml(fsItem, pNode))
							fs.Add(fsItem);
					}

					pNode = pNode->GetNext();
				}

				return true;
			}
		}
	}

	return false;
}

bool wxCurlDAVTool::Exists(const wxString& szRemoteLoc /*= wxEmptyString*/)
{
	wxArrayDAVFs arrFs;

	if(GetDAVFs(arrFs, szRemoteLoc))
	{
		return true;
	}

	return false;
}

bool wxCurlDAVTool::IsDirectory(const wxString& szRemoteLoc /*= wxEmptyString*/)
{
	wxArrayDAVFs arrFs;

	if(GetDAVFs(arrFs, szRemoteLoc))
	{
		return arrFs.Last().IsDirectory();
	}

	return false;
}

bool wxCurlDAVTool::IsOk(const wxString& szRemoteLoc /*= wxEmptyString*/)
{
	wxArrayDAVFs arrFs;

	if(GetDAVFs(arrFs, szRemoteLoc))
	{
		return arrFs.Last().IsOK();
	}

	return false;
}

wxDateTime wxCurlDAVTool::GetLastModified(const wxString& szRemoteLoc /*= wxEmptyString*/)
{
	wxArrayDAVFs arrFs;

	if(GetDAVFs(arrFs, szRemoteLoc))
	{
		return arrFs.Last().GetLastModified();
	}

	return wxDateTime();
}

wxDateTime wxCurlDAVTool::GetCreationDate(const wxString& szRemoteLoc /*= wxEmptyString*/)
{
	wxArrayDAVFs arrFs;

	if(GetDAVFs(arrFs, szRemoteLoc))
	{
		return arrFs.Last().GetCreationDate();
	}

	return wxDateTime();
}

long wxCurlDAVTool::GetContentLength(const wxString& szRemoteLoc /*= wxEmptyString*/)
{
	wxArrayDAVFs arrFs;

	if(GetDAVFs(arrFs, szRemoteLoc))
	{
		return arrFs.Last().GetContentLength();
	}

	return -1;
}

wxString wxCurlDAVTool::GetContentType(const wxString& szRemoteLoc /*= wxEmptyString*/)
{
	wxArrayDAVFs arrFs;

	if(GetDAVFs(arrFs, szRemoteLoc))
	{
		return arrFs.Last().GetContentType();
	}

	return wxEmptyString;
}

//////////////////////////////////////////////////////////////////////
// Helper Methods
//////////////////////////////////////////////////////////////////////

bool wxCurlDAVTool::ParseResponseXml(wxCurlDAVFs& fsItem, wxXmlNode* pNode)
{
	if(pNode->GetName().Find(wxS("response")) != -1)
	{
		wxXmlNode* pChild = pNode->GetChildren();

		bool bFoundHref = false;
		bool bParsedPropstat = false;

		while(pChild)
		{
			if(pChild->GetName().Find(wxS("href")) != -1)
			{
				wxXmlNode* pText = pChild->GetChildren();

				while(pText)
				{
					if(pText->GetType() == wxXML_TEXT_NODE)
					{
						fsItem.m_szHREF = pText->GetContent();
						bFoundHref = true;
					}

					pText = pText->GetNext();
				}
			}

			if(!bParsedPropstat)
			{
				if(pChild->GetName().Find(wxS("propstat")) != -1)
				{
					bParsedPropstat = ParsePropstatXml(fsItem, pChild);
				}
			}

			pChild = pChild->GetNext();
		}

		return bFoundHref && bParsedPropstat;
	}

	return false;
}

bool wxCurlDAVTool::ParsePropstatXml(wxCurlDAVFs& fsItem, wxXmlNode* pNode)
{
	if(pNode->GetName().Find(wxS("propstat")) != -1)
	{
		wxXmlNode* pChild = pNode->GetChildren();

		bool bFoundStatus = false;
		bool bParsedProps = false;

		while(pChild)
		{
			if(pChild->GetName().Find(wxS("status")) != -1)
			{
				wxXmlNode* pText = pChild->GetChildren();

				while(pText)
				{
					if(pText->GetType() == wxXML_TEXT_NODE)
					{
						if(fsItem.m_szStatus.IsEmpty())
							fsItem.m_szStatus = pText->GetContent();

						bFoundStatus = true;
					}

					pText = pText->GetNext();
				}
			}

			if(pChild->GetName().Find(wxS("prop")) != -1)
			{
				bParsedProps = ParsePropsXml(fsItem, pChild);
			}

			pChild = pChild->GetNext();
		}

		return bFoundStatus && bParsedProps;
	}

	return false;
}

bool wxCurlDAVTool::ParsePropsXml(wxCurlDAVFs& fsItem, wxXmlNode* pNode)
{
	if(pNode->GetName().Find(wxS("prop")) != -1)
	{
		wxXmlNode* pChild = pNode->GetChildren();

		bool bFoundCreationDate = false;
		bool bFoundLastModified = false;
//		bool bFoundGetContentLength = false;
//		bool bFoundGetContentType = false;

		while(pChild)
		{
			if(pChild->GetName().Find(wxS("creationdate")) != -1)
			{
				wxXmlNode* pText = pChild->GetChildren();

				while(pText)
				{
					if(pText->GetType() == wxXML_TEXT_NODE)
					{
						wxString szVal = pText->GetContent();

						fsItem.m_dtCreationDate.ParseFormat(szVal, wxS("%Y-%m-%dT%H:%M:%S"));
						bFoundCreationDate = true;
					}

					pText = pText->GetNext();
				}
			}

			if(pChild->GetName().Find(wxS("getlastmodified")) != -1)
			{
				wxXmlNode* pText = pChild->GetChildren();

				while(pText)
				{
					if(pText->GetType() == wxXML_TEXT_NODE)
					{
						wxString szVal = pText->GetContent();

						fsItem.m_dtLastModified.ParseRfc822Date(szVal);
						bFoundLastModified = true;
					}

					pText = pText->GetNext();
				}
			}
#if 0
			if(pChild->GetName().Find(wxS("getcontentlength")) != -1)
			{
				wxXmlNode* pText = pChild->GetChildren();

				while(pText)
				{
					if(pText->GetType() == wxXML_TEXT_NODE)
					{
						wxString szVal = pText->GetContent();

						if(szVal.ToLong(&fsItem.m_iContentLength))
                                                    ;//	bFoundGetContentLength = true;
					}

					pText = pText->GetNext();
				}
			}
#endif
			if(pChild->GetName().Find(wxS("getcontenttype")) != -1)
			{
				wxXmlNode* pText = pChild->GetChildren();

				while(pText)
				{
					if(pText->GetType() == wxXML_TEXT_NODE)
					{
						fsItem.m_szContentType = pText->GetContent();
						//bFoundGetContentType = true;
					}

					pText = pText->GetNext();
				}
			}

			pChild = pChild->GetNext();
		}

		return bFoundCreationDate && bFoundLastModified;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
// wxArrayDAVFs Implementation
//////////////////////////////////////////////////////////////////////
#include <wx/arrimpl.cpp> // this is a magic incantation which must be done!
WX_DEFINE_USER_EXPORTED_OBJARRAY(wxArrayDAVFs);
