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



/** @file XMLNode.cxx
    @brief xml node implementation
*/

#include "PepXUtility.h"

#include "XMLNode.h"

using namespace std;



// define static member
XMLNodePtr XMLNode::nullPtr_ = XMLNodePtr(XMLNode::create());



void
XMLNode::reset(void) {
  isEmpty_ = true;
  name_ = "";
  attrs_.clear();
  startOffset_ = -1;
  endOffset_ = -1;
  parent_= nullPtr_;//0;//.reset();
  children_.clear();
  

}

void
XMLNode::extractExpatAttributes(const XML_Char ** attrs) {
  // extract char vec into map
  string attr_name;
  string attr_val;

  for (int i=0; attrs[i]; i+=2) {
    attr_name = attrs[i];
    attr_val = attrs[i+1];
    attrs_[attr_name] = attr_val;
  }
}






/**
   (private) constructors;
   wrapped with static "create()" methods with matching arg lists;
   this is done so that we can create a weak_ptr to this in constructor.
   
 */
XMLNode::XMLNode() {
  reset();
}


XMLNode::XMLNode(const XML_Char *elementName, const XML_Char **attrs) {
  reset();
  name_ = elementName;
  extractExpatAttributes(attrs);
  isEmpty_ = false;
}


XMLNode::XMLNode(const XML_Char *elementName, const XML_Char **attrs, XMLNodePtr parent) {
  reset();
  name_ = elementName;
  extractExpatAttributes(attrs);
  parent_ = parent;
  isEmpty_ = false;
}



XMLNode::XMLNode(const XMLNode& copy) : attrs_(copy.attrs_)  {
  isEmpty_ = copy.isEmpty_;
  name_ = copy.name_;
  startOffset_ = copy.startOffset_;
  endOffset_ = copy.endOffset_;
  parent_ = copy.parent_;

  for (unsigned int i=0; i< copy.children_.size(); i++) {
    children_.push_back(copy.children_[i]);
  }
  
}




/**
   (public) static constructor wrappers;
   
   these pointers use null-deleters, so we don't try to delete
   ourselves on ~XMLNode();

 */

XMLNodePtr
XMLNode::create(void) {
  XMLNodePtr newObjPtr(new XMLNode());
  newObjPtr->selfPtr_ = XMLNodeWeakPtr(newObjPtr);//, null_deleter());
  return newObjPtr;
}


XMLNodePtr
XMLNode::create(const XML_Char *elementName, const XML_Char **attrs) {
  XMLNodePtr newObjPtr(new XMLNode(elementName, attrs));
  newObjPtr->selfPtr_ = XMLNodeWeakPtr(newObjPtr);
  return newObjPtr;
}

XMLNodePtr
XMLNode::create(const XML_Char *elementName, const XML_Char **attrs, XMLNodePtr parent) {
  XMLNodePtr newObjPtr(new XMLNode(elementName, attrs, parent));
  newObjPtr->selfPtr_ = XMLNodeWeakPtr(newObjPtr);
  return newObjPtr;
}




XMLNode::~XMLNode() {

  //debugout << "destroying " << name_ << "(" << this << ")" << endl;
  /*  
  if (isEmpty_) {
    debugout << "  (empty)" << endl;
  }
  if (name_ == "spectrum_query") {
    debugout <<"  sq " << attrs_["index"] << endl;
    //print();
  }

  */
}


string
XMLNode::getAttrValue(const std::string& attrName) {
  static map<string,string>::iterator s;

  if (!isEmpty_) {
    s=attrs_.find(attrName);
    if (s != attrs_.end()) {
      return (*s).second;
    }
  }

  // either empty, or not found
  return string("");

}

void XMLNode::print(void) { // useful for gdb enthusiasts
	print(std::cout);
}

void
XMLNode::print(std::ostream &out) {
  string indent;
  for (int c=0; c<XMLNODE_INDENT; c++) {
    indent += " ";
  }
  
  if (!isEmpty_) {
    out << indent << "<" << name_;
    for (map<string,string>::iterator i = attrs_.begin(); i != attrs_.end(); i++) {
      out << " " << (*i).first << "=" << "\"" << (*i).second << "\"";
    }

    if (children_.empty()) {
      out << " />" << endl;
    }
    else {
      out << " >" << endl;

      XMLNODE_INDENT += 2;
      for (unsigned int j = 0; j<children_.size(); j++) {
	(children_[j])->print();
      }
      XMLNODE_INDENT -= 2;

      out << indent << "</" << name_ << ">" << endl;
    }
    
  }
  else {
    out << "[empty node]" << endl;
  }
}








void
XMLNode::addChild(const XMLNodePtr& child) {
  //assert child != this;
  // parent is a normal XMLNode*

  // FIX!  ref counting *won't* work this way
  child->parent_ = selfPtr_;
  children_.push_back(child);  
}









bool
XMLNode::removeChild(const XMLNodePtr& child) {
  if (! isEmpty_) {
    XMLNodeVec::iterator i = children_.begin();
    while (i != children_.end()) {
      //debugout << name_ << ": removing " << child->name_ << endl;

      if ( (*i) == child ) {
	/*
	debugout << name_ << ": removing " << child->name_ 
	     << ": sucess!"
	     << endl;
	*/
	// disconnect child's parent
	child->parent_ = nullPtr_;//0;//.reset();
	// disconnect parent's child
	children_.erase(i);      
	return true;
      }
      i++;
    }
    // not found
    /*
    debugout << name_ << ": removing " << child->name_ 
	 << ": FAIL"
	 << endl;
    */
    return false;

  } 
  else {
    // called on empty node
    return false;
  }
}






XMLNodePtr XMLNode::findChild(const string& name) {
  
  unsigned int i;

  if (!isEmpty_) {
    for (i=0;i<children_.size();i++) {
      if ( (children_[i])->name_ == name ) {
	return children_[i];
      }
    }
  }
  
  // if here,
  // either search attempted on empty node, or no sucess in search.

  // return an empty node
  return XMLNodePtr(new XMLNode());
  //return nullPtr_;
}




XMLNodeVecPtr XMLNode::findChildren(const string& name) {
  XMLNodeVecPtr nv( new XMLNodeVec() );

  unsigned int i;

  if (!isEmpty_) {
    for (i=0;i<children_.size();i++) {
      if ( (children_[i])->name_ == name ) {
	nv->push_back(children_[i]);
      }
    }
    return nv;
  } else {
    
    
    // if here,
    // search attempted on empty node 

    // return an empty node vector
    return XMLNodeVecPtr(new XMLNodeVec());
  }
  

}



XMLNodePtr XMLNode::findChild(const string& name, const string& attr_name, const string& attr_val) {
  
  static unsigned int i;
  static map<string,string>::iterator s;

  if (!isEmpty_) {
    for (i=0;i<children_.size();i++) {
      if (  (children_[i])->name_ == name)  {
	//debugout << "name match: " << name << endl;
	s = (children_[i])->attrs_.find(attr_name);
	if (s != (children_[i])->attrs_.end()) {
	  if ( (*s).second == attr_val ) {
	    return children_[i];
	  }
	}
      }
    }
  }
  
  // if here,
  // either search attempted on empty node, or no sucess in search.

  // return an empty node
  return XMLNodePtr(new XMLNode());
  //return nullPtr_;
}







// depth first
XMLNodePtr XMLNode::findDescendent(const string& name) {
  
  unsigned int i;
  map<string,string>::iterator s;
  
  //debugout << name << "," << attr_name << "," << attr_val << endl;
  //debugout << "testing " << name_ << endl;
  
  if (!isEmpty_) {

    // we might have gotten to a leaf node
    if (children_.size() == 0) {
      // the leaf has already been searched by the parent, so return fail
      //debugout << name_ << " is a leaf" << endl;
      
      return XMLNodePtr(new XMLNode());
      //return nullPtr_;
    }
    
    for (i=0;i<children_.size();i++) {
      //debugout << name_ << ": testing child " << (children_[i])->name_ << endl;
      if (  (children_[i])->name_ == name)  {
	return children_[i];
      }
      else {
	// if we're here, child [i] didn't match
	// recursively call

	//debugout << name_ << ": " << (children_[i])->name_ << " didn't match, descending. " << attr_name << endl;
	XMLNodePtr result = 
	  (children_[i])->findDescendent(name);
	if (!result->isEmpty_) {
	  // somewhere in the children, a match was found
	  // so return that result
	  //debugout << "(passing sucessful child search back up)" << endl;
	  return result;

	}
	// no child match found; keep going until all children searched
      }
    } // end 'all children search'
    //debugout << name_ << ": all children searched, nothing here" << endl;
    
    // pass 'fail' back up
    return XMLNodePtr(new XMLNode());
    //return nullPtr_;
    
  } 

  else {
    // nothing to do with the search;
    //debugout << name_ << " was an empty node" << endl;
    // return an empty node
    return XMLNodePtr(new XMLNode());
    //return nullPtr_;
  }
  
  // if here,
  // either search attempted on empty node, or no sucess in search.

}




// depth first
XMLNodePtr XMLNode::findDescendent(const string& name, const string& attr_name, const string& attr_val) {
  
  unsigned int i;
  map<string,string>::iterator s;
  
  //debugout << name << "," << attr_name << "," << attr_val << endl;
  //debugout << "testing " << name_ << endl;
  
  if (!isEmpty_) {

    // we might have gotten to a leaf node
    if (children_.size() == 0) {
      // the leaf has already been searched by the parent, so return fail
      //debugout << name_ << " is a leaf" << endl;
      return XMLNodePtr(new XMLNode());
      //return nullPtr_;
    }
    
    for (i=0;i<children_.size();i++) {
      //debugout << name_ << ": testing child " << (children_[i])->name_ << endl;
      if (  (children_[i])->name_ == name)  {
	//debugout << name_ << ": " << (children_[i])->name_ << "  name *match*: " << name << endl;
	s = (children_[i])->attrs_.find(attr_name);
	if (s != (children_[i])->attrs_.end()) {
	  //debugout << name_ << ": child attr *match*: " << attr_name << endl;
	  if ( (*s).second == attr_val ) {
	    /*
	      debugout << name_ << ": child attr val *match*: " << attr_val 
		 << "==child:" << (*s).second
		 << endl;
	    */
	    return children_[i];
	  }
	  /*
	  debugout << name_ << ": child attr val fail: " << attr_val 
	       << "!=(child's)" << (*s).second
	       << endl;
	  */

	}
	else {
	  /*
	  debugout << name_ << ": child attr name fail: "
	       << attr_name << endl;
	  children_[i]->print();
	  */
	}
      }
      else {
	// if we're here, child [i] didn't match
	// recursively call

	//debugout << name_ << ": " << (children_[i])->name_ << " didn't match, descending. " << attr_name << endl;
	XMLNodePtr result = 
	  (children_[i])->findDescendent(name, attr_name, attr_val);
	if (!result->isEmpty_) {
	  // somewhere in the children, a match was found
	  // so return that result
	  //debugout << "(passing sucessful child search back up)" << endl;
	  return result;

	}
	// no child match found; keep going until all children searched
      }
    } // end 'all children search'
    //debugout << name_ << ": all children searched, nothing here" << endl;
    
    // pass 'fail' back up
    return XMLNodePtr(new XMLNode());
    //return nullPtr_;
    
  } 

  else {
    // nothing to do with the search;
    //debugout << name_ << " was an empty node" << endl;
    // return an empty node
    return XMLNodePtr(new XMLNode());
    //return nullPtr_;
  }
  
  // if here,
  // either search attempted on empty node, or no sucess in search.

}





// depth first
XMLNodePtr XMLNode::findDescendent(const string& name, 
				   const string& attr_name, const string& attr_val,
				   const string& attr_name2, const string& attr_val2) {
  
  unsigned int i;
  map<string,string>::iterator s;
  
  //debugout << name << "," << attr_name << "," << attr_val << endl;
  //debugout << "testing " << name_ << endl;
  
  if (!isEmpty_) {

    // we might have gotten to a leaf node
    if (children_.size() == 0) {
      // the leaf has already been searched by the parent, so return fail
      //debugout << name_ << " is a leaf" << endl;
      return XMLNodePtr(new XMLNode());
      //return nullPtr_;
    }
    
    for (i=0;i<children_.size();i++) {
      //debugout << name_ << ": testing child " << (children_[i])->name_ << endl;
      if (  (children_[i])->name_ == name)  {
	//debugout << name_ << ": " << (children_[i])->name_ << "  name *match*: " << name << endl;
	s = (children_[i])->attrs_.find(attr_name);
	if (s != (children_[i])->attrs_.end()) {
	  //debugout << name_ << ": child attr *match*: " << attr_name << endl;
	  if ( (*s).second == attr_val ) {
	    
	    // checking attr2 name for match
	    s = (children_[i])->attrs_.find(attr_name2);
	    if (s != (children_[i])->attrs_.end()) {
	      // checking attr2 value for match
	      if ( (*s).second == attr_val ) {
		// found it!
		return children_[i];
	      }
	    }
	    /*
	      debugout << name_ << ": child attr val *match*: " << attr_val 
		 << "==child:" << (*s).second
		 << endl;
	    */

	  }
	  /*
	  debugout << name_ << ": child attr val fail: " << attr_val 
	       << "!=(child's)" << (*s).second
	       << endl;
	  */

	}
	else {
	  /*
	  debugout << name_ << ": child attr name fail: "
	       << attr_name << endl;
	  children_[i]->print();
	  */
	}
      }
      else {
	// if we're here, child [i] didn't match
	// recursively call

	//debugout << name_ << ": " << (children_[i])->name_ << " didn't match, descending. " << attr_name << endl;
	XMLNodePtr result = 
	  (children_[i])->findDescendent(name, attr_name, attr_val);
	if (!result->isEmpty_) {
	  // somewhere in the children, a match was found
	  // so return that result
	  //debugout << "(passing sucessful child search back up)" << endl;
	  return result;

	}
	// no child match found; keep going until all children searched
      }
    } // end 'all children search'
    //debugout << name_ << ": all children searched, nothing here" << endl;
    
    // pass 'fail' back up
    return XMLNodePtr(new XMLNode());
    //return nullPtr_;
    
  } 

  else {
    // nothing to do with the search;
    //debugout << name_ << " was an empty node" << endl;
    // return an empty node
    return XMLNodePtr(new XMLNode());
    //return nullPtr_;
  }
  
  // if here,
  // either search attempted on empty node, or no sucess in search.

}

