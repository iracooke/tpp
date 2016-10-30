#ifndef DISCR_FUN_FACTORY_H
#define DISCR_FUN_FACTORY_H


/*


Copyright (C) 2008 ISB

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

Andrew Keller
Institute for Systems Biology
401 Terry Avenue North 
Seattle, WA  98109  USA
akeller@systemsbiology.org

*/

class DiscrimFunctionFactory
{
 public:
  DiscrimFunctionFactory(const char* name)
  {
      name_ = name;
  }

  const char* getName()
  { return name_.c_str(); }

 protected:
	 std::string name_;
};

#endif
