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




/** @file XMLTree.cxx
    @brief generic handler of XMLNode objects
*/

#include "PepXUtility.h"

#include "XMLTree.h"

using namespace std;



XMLTree::XMLTree() {
  reset();
  //stack_.clear();
}

void
XMLTree::reset(void) {
  root_.reset();
  root_ = XMLNode::create();
  root_->isEmpty_ = false;
  root_->name_="root";
  stack_.push(root_);
}

XMLNodePtr XMLTree::getRootNode(void) {
  return root_;
}


void XMLTree::setRootNode(const XMLNodePtr& root) {
  root_ = root;
  pushStack(root_);
}

void XMLTree::popStack(void) {
  stack_.pop();
}

XMLNodePtr& XMLTree::top(void) {
  return stack_.top();
}


void XMLTree::pushStack(const XMLNodePtr& node) {
  stack_.push(node);
}

void XMLTree::addNode(const XMLNodePtr& newNode) {
  if ( stack_.empty() ) {
    cout << "[adding root node]" << endl;
    // set root node
    root_ = newNode;
    stack_.push(root_);
  }
  else {
    //cout << "tree: adding " << newNode->name_ << " to " << (stack_.top())->name_ << endl;
    
    // if spectrum_query, check if it passes the filter; if not, forget it
    
    (stack_.top())->addChild(newNode);
    stack_.push(newNode);
  }
    
}

void XMLTree::print(void) { // useful for gdb enthusiasts
	print(std::cout);
}

void XMLTree::print(std::ostream &out) {
}
