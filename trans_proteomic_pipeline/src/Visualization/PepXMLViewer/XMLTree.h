// -*- mode: c++ -*-


/*
    Program: PepXMLViewer.cgi
    Description: CGI program to display pepxml files in a web browser
    Date: July 31 2006

    Copyright (C) 2006  Natalie Tasman (original author), ISB Seattle

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    Natalie Tasman
    Institute for Systems Biology
    401 Terry Avenue North
    Seattle, WA  98109  USA
    
    email (remove underscores):
    n_tasman at systems_biology dot org
*/


#ifndef _INCLUDED_XMLTREE_H_
#define _INCLUDED_XMLTREE_H_

/** @file XMLTree.h
    @brief xml tree for an expat parser; holds XMLNodes
*/


#include <stack>
#include <iostream>

#include <boost/shared_ptr.hpp>

#include "XMLNode.h"

using std::stack;
using std::string;
using std::ostream;
using boost::shared_ptr;



/**
   XMLTree
   
   generic XMLNode tree 
 */

/*
class XMLTree;
typedef shared_ptr<XMLTree> XMLTreePtr;
*/



class XMLTree {
protected: // members
  XMLNodePtr root_;
  stack<XMLNodePtr> stack_;

public: // members    
public: // methods
  XMLTree();
  void reset(void);
  // XMLTree(const XMLTree& copy); // deep copy
  const XMLTree& operator=(const XMLTree& rhs);

  bool isEmpty(void);    
  XMLNodePtr getRootNode(void);
  void setRootNode(const XMLNodePtr& root);

  // the most recent addition
  XMLNodePtr getLastChild(void);

  void addNode(const XMLNodePtr & newNode);  

  // stack access/manipulation
  XMLNodePtr getStackTop(void);
  void popStack(void);
  void pushStack(const XMLNodePtr& );
  XMLNodePtr& top(void);
  void print(ostream &out);
  void print(void); // useful for gdb enthusiasts
};






#endif // header guard



