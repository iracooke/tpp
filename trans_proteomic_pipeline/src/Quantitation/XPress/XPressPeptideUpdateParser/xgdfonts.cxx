/*
Credits and license terms

In order to resolve any possible confusion regarding the authorship of gd, the following copyright statement covers all of the authors who have requi
red such a statement. If you are aware of any oversights in this copyright notice, please contact Thomas Boutell who will be pleased to correct them.

COPYRIGHT STATEMENT FOLLOWS THIS LINE

    Portions copyright 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002 by Cold Spring Harbor Laboratory. Funded under Grant P41-RR02188 by the N
ational Institutes of Health.

    Portions copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002 by Boutell.Com, Inc.

    Portions relating to GD2 format copyright 1999, 2000, 2001, 2002 Philip Warner.

    Portions relating to PNG copyright 1999, 2000, 2001, 2002 Greg Roelofs.

    Portions relating to gdttf.c copyright 1999, 2000, 2001, 2002 John Ellson (ellson@graphviz.org).

    Portions relating to gdft.c copyright 2001, 2002 John Ellson (ellson@graphviz.org).

    Portions relating to JPEG and to color quantization copyright 2000, 2001, 2002, Doug Becker and copyright (C) 1994, 1995, 1996, 1997, 1998, 1999,
 2000, 2001, 2002, Thomas G. Lane. This software is based in part on the work of the Independent JPEG Group. See the file README-JPEG.TXT for more in
formation.

    Portions relating to WBMP copyright 2000, 2001, 2002 Maurice Szmurlo and Johan Van den Brande.

    Permission has been granted to copy, distribute and modify gd in any context without fee, including a commercial application, provided that this
notice is present in user-accessible supporting documentation.

    This does not affect your ownership of the derived work itself, and the intent is to assure proper credit for the authors of gd, not to interfere
 with your productive use of gd. If you have questions, ask. "Derived works" includes all programs that utilize the library. Credit must be given in
user-accessible documentation.

    This software is provided "AS IS." The copyright holders disclaim all warranties, either express or implied, including but not limited to implied
 warranties of merchantability and fitness for a particular purpose, with respect to this code and accompanying documentation.

    Although their code does not appear in gd 2.0.4, the authors wish to thank David Koblas, David Rowley, and Hutchison Avenue Software Corporation
for their prior contributions.

END OF COPYRIGHT STATEMENT
*/

#include "xgdfonts.h"

char gdFontSmallData[] = {
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,0,0,0,0,
  0,0,1,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,1,0,1,0,0,
  0,1,0,1,0,0,
  0,1,0,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,0,1,0,0,
  1,1,1,1,1,0,
  0,1,0,1,0,0,
  0,1,0,1,0,0,
  1,1,1,1,1,0,
  0,1,0,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,1,0,0,0,
  0,1,1,1,0,0,
  1,0,1,0,1,0,
  1,0,1,0,0,0,
  0,1,1,1,0,0,
  0,0,1,0,1,0,
  1,0,1,0,1,0,
  0,1,1,1,0,0,
  0,0,1,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,1,1,1,1,0,
  1,1,0,0,1,0,
  0,0,0,1,0,0,
  0,0,1,0,0,0,
  0,1,0,0,0,0,
  1,0,0,1,1,0,
  1,0,0,1,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,0,0,0,0,
  1,0,1,0,0,0,
  1,0,1,0,0,0,
  0,1,0,0,0,0,
  1,0,1,0,1,0,
  1,0,0,1,0,0,
  0,1,1,0,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,1,1,0,0,0,
  0,1,1,0,0,0,
  1,1,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,1,0,
  0,0,0,1,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,0,1,0,0,
  0,0,0,0,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,0,0,0,0,
  0,1,0,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,1,0,0,0,0,
  1,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,1,0,0,0,
  1,0,1,0,1,0,
  0,1,1,1,0,0,
  0,0,1,0,0,0,
  0,1,1,1,0,0,
  1,0,1,0,1,0,
  0,0,1,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  1,1,1,1,1,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,0,0,0,
  0,1,1,0,0,0,
  1,1,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,1,1,1,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,0,0,0,
  0,1,1,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,1,0,
  0,0,0,1,0,0,
  0,0,1,0,0,0,
  0,1,0,0,0,0,
  1,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,1,1,0,
  1,0,1,0,1,0,
  1,1,0,0,1,0,
  1,0,0,0,1,0,
  0,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,1,0,0,0,
  0,1,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,1,0,0,
  1,0,0,0,1,0,
  0,0,0,0,1,0,
  0,0,0,1,0,0,
  0,0,1,0,0,0,
  0,1,0,0,0,0,
  1,1,1,1,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,1,0,0,
  1,0,0,0,1,0,
  0,0,0,0,1,0,
  0,0,1,1,0,0,
  0,0,0,0,1,0,
  1,0,0,0,1,0,
  0,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,1,0,0,
  0,0,1,1,0,0,
  0,1,0,1,0,0,
  1,0,0,1,0,0,
  1,1,1,1,1,0,
  0,0,0,1,0,0,
  0,0,0,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,1,1,1,1,0,
  1,0,0,0,0,0,
  1,1,1,1,0,0,
  0,0,0,0,1,0,
  0,0,0,0,1,0,
  1,0,0,0,1,0,
  0,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,1,1,0,0,
  0,1,0,0,0,0,
  1,0,0,0,0,0,
  1,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,1,1,1,1,0,
  0,0,0,0,1,0,
  0,0,0,1,0,0,
  0,0,0,1,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,1,1,1,1,0,
  0,0,0,0,1,0,
  0,0,0,1,0,0,
  0,1,1,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,0,0,0,
  0,1,1,0,0,0,
  0,0,0,0,0,0,
  0,1,1,0,0,0,
  0,1,1,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,0,0,0,
  0,1,1,0,0,0,
  0,0,0,0,0,0,
  0,1,1,0,0,0,
  0,1,1,0,0,0,
  1,1,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,1,0,0,
  0,0,1,0,0,0,
  0,1,0,0,0,0,
  0,0,1,0,0,0,
  0,0,0,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,1,1,1,1,0,
  0,0,0,0,0,0,
  1,1,1,1,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,0,0,0,0,
  0,0,1,0,0,0,
  0,0,0,1,0,0,
  0,0,1,0,0,0,
  0,1,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,1,0,0,
  1,0,0,0,1,0,
  0,0,0,1,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,0,0,0,0,
  0,0,1,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,1,1,1,0,
  1,0,1,0,1,0,
  1,0,1,1,1,0,
  1,0,0,0,0,0,
  0,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,1,1,1,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  1,0,0,0,1,0,
  0,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,1,1,1,0,0,
  0,1,0,0,1,0,
  0,1,0,0,1,0,
  0,1,0,0,1,0,
  0,1,0,0,1,0,
  0,1,0,0,1,0,
  1,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,1,1,1,1,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  1,1,1,1,0,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  1,1,1,1,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,1,1,1,1,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  1,1,1,1,0,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  1,0,0,1,1,0,
  1,0,0,0,1,0,
  0,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,1,1,1,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,1,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,1,0,
  0,0,0,0,1,0,
  0,0,0,0,1,0,
  0,0,0,0,1,0,
  0,0,0,0,1,0,
  1,0,0,0,1,0,
  0,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,0,0,1,0,
  1,0,0,1,0,0,
  1,0,1,0,0,0,
  1,1,0,0,0,0,
  1,0,1,0,0,0,
  1,0,0,1,0,0,
  1,0,0,0,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  1,1,1,1,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,0,0,1,0,
  1,1,0,1,1,0,
  1,0,1,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,1,0,0,1,0,
  1,0,1,0,1,0,
  1,0,0,1,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,1,1,1,0,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,1,0,1,0,
  1,0,0,1,0,0,
  0,1,1,0,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,1,1,1,0,0,
  1,0,1,0,0,0,
  1,0,0,1,0,0,
  1,0,0,0,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,0,0,
  0,1,1,1,0,0,
  0,0,0,0,1,0,
  1,0,0,0,1,0,
  0,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,1,1,1,1,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,1,0,1,0,0,
  0,1,0,1,0,0,
  0,0,1,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,1,0,1,0,
  1,1,0,1,1,0,
  1,0,0,0,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,1,0,1,0,0,
  0,0,1,0,0,0,
  0,1,0,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,1,0,1,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,1,1,1,1,0,
  0,0,0,0,1,0,
  0,0,0,1,0,0,
  1,1,1,1,1,0,
  0,1,0,0,0,0,
  1,0,0,0,0,0,
  1,1,1,1,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,1,1,1,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,1,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,0,0,0,0,
  0,1,0,0,0,0,
  0,0,1,0,0,0,
  0,0,0,1,0,0,
  0,0,0,0,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  1,1,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  1,1,1,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,1,0,0,0,
  0,1,0,1,0,0,
  1,0,0,0,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,1,1,1,1,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,1,1,0,0,
  0,0,1,1,0,0,
  0,0,0,1,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,1,0,0,
  0,0,0,0,1,0,
  0,1,1,1,1,0,
  1,0,0,0,1,0,
  0,1,1,1,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  1,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  0,1,1,1,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,1,0,
  0,0,0,0,1,0,
  0,1,1,1,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,1,1,1,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,1,0,0,
  1,0,0,0,1,0,
  1,1,1,1,0,0,
  1,0,0,0,0,0,
  0,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,1,1,0,0,
  0,1,0,0,1,0,
  0,1,0,0,0,0,
  1,1,1,0,0,0,
  0,1,0,0,0,0,
  0,1,0,0,0,0,
  0,1,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,1,1,1,1,0,
  0,0,0,0,1,0,
  0,1,1,1,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  1,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,1,0,0,0,
  0,0,0,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,1,0,
  0,0,0,0,0,0,
  0,0,0,0,1,0,
  0,0,0,0,1,0,
  0,0,0,0,1,0,
  0,0,0,0,1,0,
  1,0,0,0,1,0,
  0,1,1,1,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  1,0,0,0,1,0,
  1,0,0,1,0,0,
  1,1,1,0,0,0,
  1,0,0,1,0,0,
  1,0,0,0,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,1,0,1,0,0,
  1,0,1,0,1,0,
  1,0,1,0,1,0,
  1,0,1,0,1,0,
  1,0,1,0,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,1,1,0,0,
  1,1,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,1,1,1,0,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,1,1,1,1,0,
  0,0,0,0,1,0,
  0,0,0,0,1,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,1,1,0,0,
  1,1,0,0,1,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,1,1,1,1,0,
  1,0,0,0,0,0,
  0,1,1,1,0,0,
  0,0,0,0,1,0,
  1,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  1,1,1,1,1,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,0,1,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,1,0,1,0,0,
  0,0,1,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,1,0,1,0,
  1,0,1,0,1,0,
  0,1,0,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,0,0,1,0,
  0,1,0,1,0,0,
  0,0,1,0,0,0,
  0,1,0,1,0,0,
  1,0,0,0,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  0,1,0,1,0,0,
  0,0,1,0,0,0,
  0,1,0,0,0,0,
  1,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,1,1,1,1,0,
  0,0,0,1,0,0,
  0,1,1,1,0,0,
  0,1,0,0,0,0,
  1,1,1,1,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,1,0,
  0,0,0,1,0,0,
  0,0,0,1,0,0,
  0,0,0,1,0,0,
  0,0,1,0,0,0,
  0,0,0,1,0,0,
  0,0,0,1,0,0,
  0,0,0,1,0,0,
  0,0,0,0,1,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,1,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  1,0,0,0,0,0,
  0,1,0,0,0,0,
  0,1,0,0,0,0,
  0,1,0,0,0,0,
  0,0,1,0,0,0,
  0,1,0,0,0,0,
  0,1,0,0,0,0,
  0,1,0,0,0,0,
  1,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,1,1,0,1,0,
  1,0,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,

  0,0,0,0,0,0,
  0,0,0,0,0,0,
  1,0,0,0,0,0,
  1,0,0,0,0,0,
  1,1,1,1,0,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,0,0,0,1,0,
  1,1,1,1,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0,
  0,0,0,0,0,0
};

gdFont gdFontSmallRep = {
	96,
	32,
	6,
	12,
	gdFontSmallData
};

gdFontPtr gdFontSmall = &gdFontSmallRep;
