/*
Program       : base64.h for Agilent2mzXML
Author        : David Shteynberg <dshteynb@systemsbiology.org> 
Date          : 01.30.06 
                                                                       
Copyright (C) 2006 David Shteynberg

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

David Shteynberg
Institute for Systems Biology
401 Terry Avenue North 
Seattle, WA  98109  USA
akeller@systemsbiology.org

Institute for Systems Biology, hereby disclaims all copyright interest 
in Agilent2mzXML written by David Shteynberg

*/

#ifndef _BASE64_H
#define _BASE64_H

int b64_encode (char *dest,
		const unsigned char *src,
		int len);
int b64_decode (char *dest,
		const char *src);

#endif /* BASE64_H */
