/*
 * gb2312.h
 *
 *  Created on: 2009-10-26
 *      Author: pc
 */

#ifndef GB2312_H_
#define GB2312_H_

typedef  unsigned short WCHAR; 

int unicode_to_gb2312(int unicode, char *r, char *c);
int gb2312_to_unicode(char r, char c);
void GB2312ToUTF_8(char *pOut,char *pText, int pLen);
void UTF_8ToGB2312(char *pOut, char *pText, int pLen);
//void UTF_8ToUnicode(WCHAR* pOut,char *pText);
#endif /* GB2312_H_ */
