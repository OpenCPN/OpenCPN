/*
 * RazdsParser.h
 *
 *  Created on: 22 apr 2012
 *      Author: Sysadm
 */

#ifndef RAZDSPARSER_H_
#define RAZDSPARSER_H_
class RazdsParser {
public:
	RazdsParser();
	virtual ~RazdsParser();
	int LoadFile ( s52plib* plib, const wxString& PLib );

private:
	int ReadS52Line( char *pBuffer, const char *delim, int nCount, FILE *fp );
	int ChopS52Line( char *pBuffer, char c );
	int ParsePos( position *pos, char *buf, bool patt );
    int ParseLBID(FILE *fp);
    int ParseCOLS(FILE *fp);
    int ParseLUPT(FILE *fp);
    int ParseLNST(FILE *fp);
    int ParsePATT(FILE *fp);
    int ParseSYMB(FILE *fp, RuleHash *pHash);
    int _CIE2RGB( S52color* toRGB, double x, double y, double L );

    // working buffer
    #define  MAX_BUF  1024
    char buffer[MAX_BUF];
    char *pBuf;
    s52plib* plib;
    wxArrayPtrVoid* ColorTableArray;
    int m_LUPSequenceNumber;
};

#endif /* RAZDSPARSER_H_ */
