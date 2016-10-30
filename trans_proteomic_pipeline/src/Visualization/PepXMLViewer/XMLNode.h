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

#ifndef _INCLUDED_XMLNODE_H_
#define _INCLUDED_XMLNODE_H_

/** @file XMLNode.h
    @brief xml node representation for an expat parser
*/

#include <vector>
#include <map>
#include <string>
#include <iostream>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <expat.h>

using std::vector;
using std::map;
using std::string;
using std::ostream;
using boost::shared_ptr;
using boost::weak_ptr;



class XMLNode;
typedef shared_ptr<XMLNode> XMLNodePtr;
typedef weak_ptr<XMLNode> XMLNodeWeakPtr;



typedef vector<XMLNodePtr> XMLNodeVec;
//typedef vector<XMLNode*> XMLNodeVec;
typedef shared_ptr< XMLNodeVec > XMLNodeVecPtr;


/** 
    weak_ptr vs shared_ptr
  
    a weak_ptr does _not_ contribute towards keeping the object alive;
    if a shared_ptr and weak_ptr both point to an object, the object
    is deleted when the shared_ptr is reset or deleted, regardless of
    the state of the weak_ptr.

    what is the use compared to a standard C++ pointer?  Derefing the
    weak_ptr returns NULL when the object has been deleted, whereas a
    standard pointer returns no info as to the state of the memory it
    points to; classType* A, *B=new A(); A=B; delete B; A==B is still
    true, i.e. B still points to the region in memory even though it's
    been deallocated.

    in a cyclic graph, shared_ptrs will fail because an object may
    *fail to be deleted*; weak_ptrs can solve this situation:

    From http://www.boost.org/libs/smart_ptr/shared_ptr.htm
    "Because the implementation uses reference counting, cycles of
    shared_ptr instances will not be reclaimed. For example, if main()
    holds a shared_ptr to A, which directly or indirectly holds a
    shared_ptr back to A, A's use count will be 2. Destruction of the
    original shared_ptr will leave A dangling with a use count of
    1. Use weak_ptr to "break cycles.""
  
    See http://www.codeproject.com/vcpp/stl/boostsmartptr.asp#Using%20weak_ptr%20to%20break%20cycles



    constructing weak_ptr in constructor:
    http://boost.org/libs/smart_ptr/weak_ptr.htm#FAQ
    
*/
    




class XMLNode {
public:
  // members

  bool isEmpty_;

  string name_;
  map<string, string> attrs_;

  long long startOffset_;
  long long endOffset_;



  
  XMLNodeWeakPtr parent_;
  XMLNodeVec children_;

  // maintain a weak_ptr to ourselves; this way, we don't try to
  // delete ourselves when destroying our member selfPtr_, and we can
  // pass this weak_ptr to our children when they need to refer to us
  // (via child->parent_), and they won't try to delete us (graph
  // cycle breaking.)
  XMLNodeWeakPtr selfPtr_;


private:
  static XMLNodePtr nullPtr_; 
  // shared by all objects; used as
  // special return value; never set
  // to anything, so is always == 0
  
  

private:


  
  inline void reset(void); // init to "zero"
  inline void extractExpatAttributes(const XML_Char ** attrs);


  XMLNode(); // nameless node   
  XMLNode(const XML_Char *elementName, const XML_Char **attrs); // no parent; null by default
  XMLNode(const XML_Char *elementName, const XML_Char **attrs, XMLNodePtr parent);


  const XMLNode& operator=(const XMLNode & DoNotUse_rhs);

  

public:



  //methods
  


  static XMLNodePtr create(void);
  static XMLNodePtr create(const XML_Char *elementName, const XML_Char **attrs);
  static XMLNodePtr create(const XML_Char *elementName, const XML_Char **attrs, XMLNodePtr parent);

  /**
     nameless nodes have special behavior: on any search that returns
     a child, return another nameless node; this allows for behavior
     such as:

     root.findChild("top").findChild("name","nonexistant attr", "attr val").findChild("nonexistant name");

     to return without throwing an exception
   */
  XMLNode(const XMLNode& copy);



  ~XMLNode();

  friend bool operator==(const XMLNode& lhs, const XMLNode& rhs);

  bool hasAttr(const string& attrName);
  string getAttrValue(const string& attrName);

  bool parseExpatAttrs(const XML_Char **attrs);

  XMLNodePtr getFirstChild(void);
  XMLNodeVecPtr getChildren(void);

  void addChild(const XMLNodePtr& child);
  bool removeChild(const XMLNodePtr& child); // false if child wasn't found

  // change to find first child?

  // searching children
  bool hasChild(const string& name);
  XMLNodePtr findChild(const string& name);
  XMLNodeVecPtr findChildren(const string& name);
  bool hasChild(const string& name, const string& attr_name, const string& attr_val);
  XMLNodePtr findChild(const string& name, const string& attr_name, const string& attr_val);

  // descendents: searching children and subchildren
  bool hasDescendent(const string& name);
  bool hasDescendent(const string& name, const string& attr_name, const string& attr_val);

  XMLNodePtr findDescendent(const string& name);
  XMLNodePtr findDescendent(const string& name, const string& attr_name, const string& attr_val);
  XMLNodePtr findDescendent(const string& name, 
			    const string& attr_name, const string& attr_val,
			    const string& attr_name2, const string& attr_val2);

  
  void print(ostream &out);
  void print(void); // useful for gdb enthusiasts

};

static int XMLNODE_INDENT = 0;

void XMLNode_Test(void);

#endif // header guard



